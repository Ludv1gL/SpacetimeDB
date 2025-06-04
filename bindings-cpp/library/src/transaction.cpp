#include "spacetimedb/transaction.h"
#include "spacetimedb/spacetimedb.h"
#include <mutex>
#include <thread>
#include <unordered_map>

namespace spacetimedb {

// Internal transaction implementation
class TransactionImpl {
public:
    TransactionImpl(ReducerContext& ctx, const TransactionOptions& options)
        : ctx_(ctx)
        , options_(options)
        , state_(TransactionState::Active)
        , start_time_(std::chrono::steady_clock::now()) {
        
        // Note: In SpacetimeDB, transactions are implicit within reducers
        // This implementation provides a logical transaction API on top
        
        // Store reference to this transaction in the context
        auto thread_id = std::this_thread::get_id();
        {
            std::lock_guard<std::mutex> lock(active_transactions_mutex_);
            active_transactions_[thread_id] = this;
        }
        
        // Initialize metrics
        metrics_.lock_wait_time = std::chrono::milliseconds(0);
    }
    
    ~TransactionImpl() {
        if (state_ == TransactionState::Active) {
            // Implicit rollback
            try {
                rollback();
            } catch (...) {
                // Suppress exceptions in destructor
            }
        }
        
        // Remove from active transactions
        auto thread_id = std::this_thread::get_id();
        {
            std::lock_guard<std::mutex> lock(active_transactions_mutex_);
            active_transactions_.erase(thread_id);
        }
    }
    
    void commit() {
        if (state_ != TransactionState::Active) {
            throw TransactionError("Transaction is not active");
        }
        
        // In SpacetimeDB, commits happen automatically at reducer completion
        // This is a logical commit that validates our transaction state
        
        if (!savepoints_.empty()) {
            throw TransactionError("Cannot commit with active savepoints");
        }
        
        // Calculate final metrics
        auto end_time = std::chrono::steady_clock::now();
        metrics_.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time_);
        
        state_ = TransactionState::Committed;
        
        // Log transaction completion
        spacetimedb::log("Transaction committed", spacetimedb::LogLevel::Debug);
    }
    
    void rollback() {
        if (state_ != TransactionState::Active) {
            throw TransactionError("Transaction is not active");
        }
        
        // In SpacetimeDB, rollback happens automatically on reducer failure
        // We would need to throw an exception to trigger actual rollback
        state_ = TransactionState::RolledBack;
        
        // Clear all savepoints
        savepoints_.clear();
        
        spacetimedb::log("Transaction rolled back", spacetimedb::LogLevel::Debug);
        
        // To actually rollback in SpacetimeDB, we need to make the reducer fail
        throw TransactionError("Transaction explicitly rolled back");
    }
    
    void savepoint(const std::string& name) {
        if (state_ != TransactionState::Active) {
            throw TransactionError("Transaction is not active");
        }
        
        if (savepoints_.find(name) != savepoints_.end()) {
            throw TransactionError("Savepoint already exists: " + name);
        }
        
        // Capture current state
        SavepointState sp;
        sp.metrics = metrics_;
        sp.timestamp = std::chrono::steady_clock::now();
        
        savepoints_[name] = sp;
        savepoint_order_.push_back(name);
    }
    
    void release_savepoint(const std::string& name) {
        if (state_ != TransactionState::Active) {
            throw TransactionError("Transaction is not active");
        }
        
        auto it = savepoints_.find(name);
        if (it == savepoints_.end()) {
            throw TransactionError("Savepoint not found: " + name);
        }
        
        // Remove this savepoint and all savepoints created after it
        auto order_it = std::find(savepoint_order_.begin(), savepoint_order_.end(), name);
        if (order_it != savepoint_order_.end()) {
            // Remove all savepoints from this one onwards
            for (auto it2 = order_it; it2 != savepoint_order_.end(); ++it2) {
                savepoints_.erase(*it2);
            }
            savepoint_order_.erase(order_it, savepoint_order_.end());
        }
    }
    
    void rollback_to_savepoint(const std::string& name) {
        if (state_ != TransactionState::Active) {
            throw TransactionError("Transaction is not active");
        }
        
        auto it = savepoints_.find(name);
        if (it == savepoints_.end()) {
            throw TransactionError("Savepoint not found: " + name);
        }
        
        // Restore metrics to savepoint state
        metrics_ = it->second.metrics;
        
        // Remove all savepoints created after this one
        auto order_it = std::find(savepoint_order_.begin(), savepoint_order_.end(), name);
        if (order_it != savepoint_order_.end()) {
            ++order_it; // Keep the target savepoint
            for (auto it2 = order_it; it2 != savepoint_order_.end(); ++it2) {
                savepoints_.erase(*it2);
            }
            savepoint_order_.erase(order_it, savepoint_order_.end());
        }
        
        // Note: Actual data rollback would require SpacetimeDB support
        spacetimedb::log("Rolled back to savepoint: " + name, spacetimedb::LogLevel::Debug);
    }
    
    ModuleDatabase& database() {
        return ctx_.db;
    }
    
    const ModuleDatabase& database() const {
        return ctx_.db;
    }
    
    // Static method to get current transaction for a thread
    static TransactionImpl* current_for_thread() {
        auto thread_id = std::this_thread::get_id();
        std::lock_guard<std::mutex> lock(active_transactions_mutex_);
        auto it = active_transactions_.find(thread_id);
        return (it != active_transactions_.end()) ? it->second : nullptr;
    }
    
public:
    ReducerContext& ctx_;
    TransactionOptions options_;
    TransactionState state_;
    TransactionMetrics metrics_;
    
private:
    struct SavepointState {
        TransactionMetrics metrics;
        std::chrono::steady_clock::time_point timestamp;
    };
    
    std::unordered_map<std::string, SavepointState> savepoints_;
    std::vector<std::string> savepoint_order_;
    std::chrono::steady_clock::time_point start_time_;
    
    // Thread-local storage for active transactions
    static std::mutex active_transactions_mutex_;
    static std::unordered_map<std::thread::id, TransactionImpl*> active_transactions_;
};

// Static member definitions
std::mutex TransactionImpl::active_transactions_mutex_;
std::unordered_map<std::thread::id, TransactionImpl*> TransactionImpl::active_transactions_;

// Transaction implementation
Transaction::Transaction(std::unique_ptr<TransactionImpl> impl)
    : impl_(std::move(impl)) {}

Transaction::Transaction(Transaction&& other) noexcept = default;
Transaction& Transaction::operator=(Transaction&& other) noexcept = default;
Transaction::~Transaction() = default;

void Transaction::commit() {
    impl_->commit();
}

void Transaction::rollback() {
    impl_->rollback();
}

TransactionState Transaction::state() const {
    return impl_->state_;
}

bool Transaction::is_readonly() const {
    return impl_->options_.readonly;
}

IsolationLevel Transaction::isolation_level() const {
    return impl_->options_.isolation_level;
}

void Transaction::savepoint(const std::string& name) {
    impl_->savepoint(name);
}

void Transaction::release_savepoint(const std::string& name) {
    impl_->release_savepoint(name);
}

void Transaction::rollback_to_savepoint(const std::string& name) {
    impl_->rollback_to_savepoint(name);
}

TransactionMetrics Transaction::metrics() const {
    return impl_->metrics_;
}

ModuleDatabase& Transaction::database() {
    return impl_->database();
}

const ModuleDatabase& Transaction::database() const {
    return impl_->database();
}

// TransactionManager implementation
Transaction TransactionManager::begin(ReducerContext& ctx, const TransactionOptions& options) {
    // Check if we're already in a transaction
    if (TransactionImpl::current_for_thread() != nullptr) {
        throw TransactionError("Nested transactions are not supported (use savepoints instead)");
    }
    
    // Validate options
    if (options.readonly && options.isolation_level > IsolationLevel::Snapshot) {
        throw TransactionError("Read-only transactions cannot use Serializable isolation");
    }
    
    // Create new transaction
    auto impl = std::make_unique<TransactionImpl>(ctx, options);
    
    // Log transaction start
    std::string isolation_str;
    switch (options.isolation_level) {
        case IsolationLevel::ReadUncommitted: isolation_str = "ReadUncommitted"; break;
        case IsolationLevel::ReadCommitted: isolation_str = "ReadCommitted"; break;
        case IsolationLevel::RepeatableRead: isolation_str = "RepeatableRead"; break;
        case IsolationLevel::Snapshot: isolation_str = "Snapshot"; break;
        case IsolationLevel::Serializable: isolation_str = "Serializable"; break;
    }
    
    spacetimedb::log("Beginning transaction with isolation level: " + isolation_str, 
                     spacetimedb::LogLevel::Debug);
    
    return Transaction(std::move(impl));
}

bool TransactionManager::in_transaction(const ReducerContext& ctx) {
    // In SpacetimeDB, we're always in a transaction within a reducer
    // This checks for our explicit transaction API usage
    return TransactionImpl::current_for_thread() != nullptr;
}

std::optional<Transaction*> TransactionManager::current_transaction(ReducerContext& ctx) {
    auto* impl = TransactionImpl::current_for_thread();
    if (impl && impl->state_ == TransactionState::Active) {
        // This is a bit of a hack - we'd need to store the Transaction object
        // For now, return empty optional
        return std::nullopt;
    }
    return std::nullopt;
}

// Optimistic locking helpers
template<typename T>
bool OptimisticLock::check_version(const T& entity, uint64_t expected_version) {
    // This would require version fields in tables
    // For now, this is a placeholder
    return true;
}

template<typename T>
bool OptimisticLock::check_timestamp(const T& entity, const std::chrono::system_clock::time_point& expected_timestamp) {
    // This would require timestamp fields in tables
    // For now, this is a placeholder
    return true;
}

// Distributed transaction implementation (placeholder)
void DistributedTransaction::prepare() {
    if (state_ != TransactionState::Active) {
        throw TransactionError("Transaction is not active");
    }
    // Would implement 2PC prepare phase here
}

void DistributedTransaction::commit() {
    if (state_ != TransactionState::Active) {
        throw TransactionError("Transaction is not active");
    }
    // Would implement 2PC commit phase here
    state_ = TransactionState::Committed;
}

void DistributedTransaction::abort() {
    if (state_ != TransactionState::Active) {
        throw TransactionError("Transaction is not active");
    }
    // Would implement 2PC abort phase here
    state_ = TransactionState::RolledBack;
}

void DistributedTransaction::add_participant(const std::string& participant_id) {
    if (state_ != TransactionState::Active) {
        throw TransactionError("Cannot add participant to inactive transaction");
    }
    participants_.push_back(participant_id);
}

} // namespace spacetimedb