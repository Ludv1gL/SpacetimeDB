#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <chrono>
#include <exception>

namespace spacetimedb {

// Forward declarations
class ModuleDatabase;
class ReducerContext;

// Transaction isolation levels matching SpacetimeDB's internal implementation
enum class IsolationLevel {
    ReadUncommitted,
    ReadCommitted,
    RepeatableRead,
    Snapshot,
    Serializable
};

// Transaction state
enum class TransactionState {
    Active,
    Committed,
    RolledBack,
    Failed
};

// Transaction error types
class TransactionError : public std::exception {
public:
    explicit TransactionError(const std::string& message) : message_(message) {}
    const char* what() const noexcept override { return message_.c_str(); }
private:
    std::string message_;
};

class DeadlockError : public TransactionError {
public:
    DeadlockError() : TransactionError("Transaction deadlock detected") {}
};

class IsolationViolationError : public TransactionError {
public:
    IsolationViolationError() : TransactionError("Transaction isolation violation") {}
};

class SerializationError : public TransactionError {
public:
    SerializationError() : TransactionError("Transaction serialization error") {}
};

// Transaction options
struct TransactionOptions {
    IsolationLevel isolation_level = IsolationLevel::Serializable;
    std::chrono::milliseconds timeout = std::chrono::milliseconds(30000); // 30 seconds default
    bool readonly = false;
    std::string name; // Optional transaction name for debugging
};

// Transaction metrics
struct TransactionMetrics {
    size_t rows_read = 0;
    size_t rows_written = 0;
    size_t bytes_read = 0;
    size_t bytes_written = 0;
    size_t index_seeks = 0;
    std::chrono::milliseconds duration;
    std::chrono::milliseconds lock_wait_time;
};

// Forward declaration of internal transaction implementation
class TransactionImpl;

// Transaction handle
class Transaction {
public:
    // Cannot copy transactions
    Transaction(const Transaction&) = delete;
    Transaction& operator=(const Transaction&) = delete;
    
    // Can move transactions
    Transaction(Transaction&& other) noexcept;
    Transaction& operator=(Transaction&& other) noexcept;
    
    ~Transaction();
    
    // Transaction control
    void commit();
    void rollback();
    
    // Transaction state
    TransactionState state() const;
    bool is_active() const { return state() == TransactionState::Active; }
    bool is_readonly() const;
    IsolationLevel isolation_level() const;
    
    // Savepoints (nested transactions)
    void savepoint(const std::string& name);
    void release_savepoint(const std::string& name);
    void rollback_to_savepoint(const std::string& name);
    
    // Metrics
    TransactionMetrics metrics() const;
    
    // Get the database handle for this transaction
    ModuleDatabase& database();
    const ModuleDatabase& database() const;
    
private:
    friend class TransactionManager;
    explicit Transaction(std::unique_ptr<TransactionImpl> impl);
    std::unique_ptr<TransactionImpl> impl_;
};

// RAII Transaction guard
class TransactionGuard {
public:
    explicit TransactionGuard(Transaction&& tx) : tx_(std::move(tx)) {}
    
    ~TransactionGuard() {
        if (tx_.is_active()) {
            // Automatic rollback if not explicitly committed
            try {
                tx_.rollback();
            } catch (...) {
                // Suppress exceptions in destructor
            }
        }
    }
    
    // Explicitly commit
    void commit() { tx_.commit(); committed_ = true; }
    
    // Access the underlying transaction
    Transaction& operator*() { return tx_; }
    const Transaction& operator*() const { return tx_; }
    Transaction* operator->() { return &tx_; }
    const Transaction* operator->() const { return &tx_; }
    
private:
    Transaction tx_;
    bool committed_ = false;
};

// Transaction manager
class TransactionManager {
public:
    // Begin a new transaction
    static Transaction begin(ReducerContext& ctx, const TransactionOptions& options = {});
    
    // Execute a function within a transaction with automatic retry on serialization errors
    template<typename F>
    static auto with_transaction(ReducerContext& ctx, F&& func, const TransactionOptions& options = {}) 
        -> decltype(func(std::declval<Transaction&>())) {
        
        const int max_retries = 3;
        for (int retry = 0; retry < max_retries; ++retry) {
            try {
                TransactionGuard guard(begin(ctx, options));
                auto result = func(*guard);
                guard.commit();
                return result;
            } catch (const SerializationError&) {
                if (retry == max_retries - 1) throw;
                // Exponential backoff
                std::this_thread::sleep_for(std::chrono::milliseconds(10 * (1 << retry)));
            }
        }
        throw TransactionError("Transaction failed after maximum retries");
    }
    
    // Execute a read-only query
    template<typename F>
    static auto with_readonly_transaction(ReducerContext& ctx, F&& func) 
        -> decltype(func(std::declval<Transaction&>())) {
        
        TransactionOptions options;
        options.readonly = true;
        options.isolation_level = IsolationLevel::Snapshot;
        
        TransactionGuard guard(begin(ctx, options));
        return func(*guard);
    }
    
    // Check if we're currently in a transaction
    static bool in_transaction(const ReducerContext& ctx);
    
    // Get the current transaction (if any)
    static std::optional<Transaction*> current_transaction(ReducerContext& ctx);
};

// Optimistic concurrency control helpers
class OptimisticLock {
public:
    // Version-based optimistic locking
    template<typename T>
    static bool check_version(const T& entity, uint64_t expected_version);
    
    // Timestamp-based optimistic locking  
    template<typename T>
    static bool check_timestamp(const T& entity, const std::chrono::system_clock::time_point& expected_timestamp);
};

// Distributed transaction support (future extension)
class DistributedTransaction {
public:
    // Two-phase commit protocol
    void prepare();
    void commit();
    void abort();
    
    // Add participants
    void add_participant(const std::string& participant_id);
    
private:
    std::vector<std::string> participants_;
    TransactionState state_ = TransactionState::Active;
};

} // namespace spacetimedb