#include <spacetimedb/spacetimedb.h>
#include <spacetimedb/transaction.h>
#include <string>
#include <vector>

using namespace SpacetimeDb;

// Example tables for demonstrating transactions

// Account table for banking example
struct Account {
    uint32_t id;
    std::string owner;
    double balance;
};

SPACETIMEDB_REGISTER_FIELDS(Account,
    SPACETIMEDB_FIELD(Account, id, uint32_t);
    SPACETIMEDB_FIELD(Account, owner, std::string);
    SPACETIMEDB_FIELD(Account, balance, double);
)

SPACETIMEDB_TABLE(Account, account, true)

// Transaction log table
struct TransactionLog {
    uint32_t id;
    uint32_t from_account;
    uint32_t to_account;
    double amount;
    uint64_t timestamp;
    std::string status;
};

SPACETIMEDB_REGISTER_FIELDS(TransactionLog,
    SPACETIMEDB_FIELD(TransactionLog, id, uint32_t);
    SPACETIMEDB_FIELD(TransactionLog, from_account, uint32_t);
    SPACETIMEDB_FIELD(TransactionLog, to_account, uint32_t);
    SPACETIMEDB_FIELD(TransactionLog, amount, double);
    SPACETIMEDB_FIELD(TransactionLog, timestamp, uint64_t);
    SPACETIMEDB_FIELD(TransactionLog, status, std::string);
)

SPACETIMEDB_TABLE(TransactionLog, transaction_log, true)

// Inventory table for demonstrating optimistic locking
struct InventoryItem {
    uint32_t id;
    std::string name;
    uint32_t quantity;
    uint64_t version;  // For optimistic locking
};

SPACETIMEDB_REGISTER_FIELDS(InventoryItem,
    SPACETIMEDB_FIELD(InventoryItem, id, uint32_t);
    SPACETIMEDB_FIELD(InventoryItem, name, std::string);
    SPACETIMEDB_FIELD(InventoryItem, quantity, uint32_t);
    SPACETIMEDB_FIELD(InventoryItem, version, uint64_t);
)

SPACETIMEDB_TABLE(InventoryItem, inventory, true)

// Example 1: Basic transaction with commit/rollback
SPACETIMEDB_REDUCER(basic_transaction_example, ReducerContext ctx, uint32_t account_id, double amount) {
    try {
        // Begin a transaction
        auto tx = TransactionManager::begin(ctx);
        
        // Find the account
        auto account_table = tx.database().table<Account>("account");
        auto accounts = account_table.iter();
        
        Account* target_account = nullptr;
        for (auto& acc : accounts) {
            if (acc.id == account_id) {
                target_account = &acc;
                break;
            }
        }
        
        if (!target_account) {
            SpacetimeDb::log("Account not found", LogLevel::Error);
            tx.rollback();  // Explicit rollback
            return;
        }
        
        // Check if sufficient balance
        if (target_account->balance < amount) {
            SpacetimeDb::log("Insufficient balance", LogLevel::Error);
            tx.rollback();
            return;
        }
        
        // Update balance
        target_account->balance -= amount;
        account_table.update(*target_account);
        
        // Log the transaction
        TransactionLog log_entry{
            0,  // auto-generated
            account_id,
            0,  // withdrawal, no destination
            amount,
            static_cast<uint64_t>(std::time(nullptr)),
            "withdrawal"
        };
        tx.database().table<TransactionLog>("transaction_log").insert(log_entry);
        
        // Commit the transaction
        tx.commit();
        SpacetimeDb::log("Transaction committed successfully", LogLevel::Info);
        
    } catch (const TransactionError& e) {
        SpacetimeDb::log(std::string("Transaction error: ") + e.what(), LogLevel::Error);
    }
}

// Example 2: Using TransactionGuard for RAII
SPACETIMEDB_REDUCER(transfer_with_guard, ReducerContext ctx, uint32_t from_id, uint32_t to_id, double amount) {
    try {
        // TransactionGuard automatically rolls back on exception
        TransactionGuard guard(TransactionManager::begin(ctx));
        
        auto account_table = guard->database().table<Account>("account");
        
        // Find both accounts
        Account* from_account = nullptr;
        Account* to_account = nullptr;
        
        for (auto& acc : account_table.iter()) {
            if (acc.id == from_id) from_account = &acc;
            if (acc.id == to_id) to_account = &acc;
        }
        
        if (!from_account || !to_account) {
            throw TransactionError("One or both accounts not found");
        }
        
        if (from_account->balance < amount) {
            throw TransactionError("Insufficient balance");
        }
        
        // Perform transfer
        from_account->balance -= amount;
        to_account->balance += amount;
        
        account_table.update(*from_account);
        account_table.update(*to_account);
        
        // Log the transfer
        TransactionLog log_entry{
            0,
            from_id,
            to_id,
            amount,
            static_cast<uint64_t>(std::time(nullptr)),
            "transfer"
        };
        guard->database().table<TransactionLog>("transaction_log").insert(log_entry);
        
        // Explicitly commit (otherwise auto-rollback happens)
        guard.commit();
        
    } catch (const std::exception& e) {
        SpacetimeDb::log(std::string("Transfer failed: ") + e.what(), LogLevel::Error);
    }
}

// Example 3: Using savepoints for nested operations
SPACETIMEDB_REDUCER(complex_operation_with_savepoints, ReducerContext ctx) {
    try {
        TransactionGuard guard(TransactionManager::begin(ctx));
        
        // First operation
        Account new_account{1001, "Alice", 1000.0};
        guard->database().table<Account>("account").insert(new_account);
        
        // Create savepoint
        guard->savepoint("after_account_creation");
        
        try {
            // Risky operation that might fail
            Account risky_account{1002, "Bob", -100.0};  // Invalid balance
            if (risky_account.balance < 0) {
                throw std::runtime_error("Negative balance not allowed");
            }
            guard->database().table<Account>("account").insert(risky_account);
            
        } catch (const std::exception& e) {
            // Rollback to savepoint, keeping the first account
            guard->rollback_to_savepoint("after_account_creation");
            SpacetimeDb::log("Rolled back risky operation", LogLevel::Info);
        }
        
        // Continue with other operations
        Account safe_account{1003, "Charlie", 500.0};
        guard->database().table<Account>("account").insert(safe_account);
        
        guard.commit();
        
    } catch (const TransactionError& e) {
        SpacetimeDb::log(std::string("Transaction failed: ") + e.what(), LogLevel::Error);
    }
}

// Example 4: Using with_transaction helper for automatic retry
SPACETIMEDB_REDUCER(transfer_with_retry, ReducerContext ctx, uint32_t from_id, uint32_t to_id, double amount) {
    try {
        auto result = TransactionManager::with_transaction(ctx, [&](Transaction& tx) {
            auto account_table = tx.database().table<Account>("account");
            
            // Simulate potential serialization conflict
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            
            // Perform transfer logic
            Account* from_account = nullptr;
            Account* to_account = nullptr;
            
            for (auto& acc : account_table.iter()) {
                if (acc.id == from_id) from_account = &acc;
                if (acc.id == to_id) to_account = &acc;
            }
            
            if (!from_account || !to_account) {
                throw TransactionError("Account not found");
            }
            
            if (from_account->balance < amount) {
                throw TransactionError("Insufficient balance");
            }
            
            from_account->balance -= amount;
            to_account->balance += amount;
            
            account_table.update(*from_account);
            account_table.update(*to_account);
            
            return true;  // Success
        });
        
        if (result) {
            SpacetimeDb::log("Transfer completed successfully", LogLevel::Info);
        }
        
    } catch (const std::exception& e) {
        SpacetimeDb::log(std::string("Transfer failed after retries: ") + e.what(), LogLevel::Error);
    }
}

// Example 5: Read-only transaction for reporting
SPACETIMEDB_REDUCER(generate_balance_report, ReducerContext ctx) {
    TransactionManager::with_readonly_transaction(ctx, [&](Transaction& tx) {
        auto account_table = tx.database().table<Account>("account");
        
        double total_balance = 0.0;
        int account_count = 0;
        
        for (const auto& account : account_table.iter()) {
            total_balance += account.balance;
            account_count++;
            SpacetimeDb::log("Account " + std::to_string(account.id) + 
                           " (" + account.owner + "): $" + 
                           std::to_string(account.balance), LogLevel::Info);
        }
        
        SpacetimeDb::log("Total accounts: " + std::to_string(account_count), LogLevel::Info);
        SpacetimeDb::log("Total balance: $" + std::to_string(total_balance), LogLevel::Info);
        
        // Get transaction metrics
        auto metrics = tx.metrics();
        SpacetimeDb::log("Rows read: " + std::to_string(metrics.rows_read), LogLevel::Debug);
        SpacetimeDb::log("Index seeks: " + std::to_string(metrics.index_seeks), LogLevel::Debug);
        
        return 0;
    });
}

// Example 6: Optimistic locking pattern
SPACETIMEDB_REDUCER(update_inventory_optimistic, ReducerContext ctx, uint32_t item_id, uint32_t quantity_change) {
    const int max_retries = 5;
    
    for (int retry = 0; retry < max_retries; ++retry) {
        try {
            TransactionGuard guard(TransactionManager::begin(ctx));
            
            auto inventory_table = guard->database().table<InventoryItem>("inventory");
            
            // Read the current item
            InventoryItem* item = nullptr;
            uint64_t original_version = 0;
            
            for (auto& inv : inventory_table.iter()) {
                if (inv.id == item_id) {
                    item = &inv;
                    original_version = inv.version;
                    break;
                }
            }
            
            if (!item) {
                throw TransactionError("Item not found");
            }
            
            // Simulate some processing time where conflicts might occur
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            
            // Check if version changed (optimistic lock check)
            if (item->version != original_version) {
                throw SerializationError();  // Retry
            }
            
            // Update the item
            item->quantity += quantity_change;
            item->version++;  // Increment version
            
            inventory_table.update(*item);
            
            guard.commit();
            SpacetimeDb::log("Inventory updated successfully", LogLevel::Info);
            break;  // Success, exit retry loop
            
        } catch (const SerializationError&) {
            if (retry == max_retries - 1) {
                SpacetimeDb::log("Failed to update inventory after max retries", LogLevel::Error);
                throw;
            }
            SpacetimeDb::log("Version conflict detected, retrying...", LogLevel::Debug);
            std::this_thread::sleep_for(std::chrono::milliseconds(10 * (1 << retry)));
        }
    }
}

// Example 7: Transaction with custom isolation level
SPACETIMEDB_REDUCER(snapshot_isolation_example, ReducerContext ctx) {
    TransactionOptions options;
    options.isolation_level = IsolationLevel::Snapshot;
    options.timeout = std::chrono::seconds(5);
    options.name = "snapshot_read";
    
    try {
        auto tx = TransactionManager::begin(ctx, options);
        
        // Snapshot isolation ensures consistent view even if other transactions commit
        auto account_table = tx.database().table<Account>("account");
        
        std::vector<Account> snapshot;
        for (const auto& account : account_table.iter()) {
            snapshot.push_back(account);
        }
        
        // Simulate long-running analysis
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Data is still consistent with the snapshot
        double total = 0.0;
        for (const auto& acc : snapshot) {
            total += acc.balance;
        }
        
        SpacetimeDb::log("Snapshot total: $" + std::to_string(total), LogLevel::Info);
        
        tx.commit();
        
    } catch (const TransactionError& e) {
        SpacetimeDb::log(std::string("Snapshot read failed: ") + e.what(), LogLevel::Error);
    }
}

// Initialize some test data
SPACETIMEDB_REDUCER(init_transaction_demo, ReducerContext ctx) {
    try {
        TransactionGuard guard(TransactionManager::begin(ctx));
        
        // Create some accounts
        std::vector<Account> accounts = {
            {1, "Alice", 1000.0},
            {2, "Bob", 500.0},
            {3, "Charlie", 750.0}
        };
        
        auto account_table = guard->database().table<Account>("account");
        for (const auto& acc : accounts) {
            account_table.insert(acc);
        }
        
        // Create some inventory items
        std::vector<InventoryItem> items = {
            {1, "Widget", 100, 1},
            {2, "Gadget", 50, 1},
            {3, "Doohickey", 75, 1}
        };
        
        auto inventory_table = guard->database().table<InventoryItem>("inventory");
        for (const auto& item : items) {
            inventory_table.insert(item);
        }
        
        guard.commit();
        SpacetimeDb::log("Demo data initialized", LogLevel::Info);
        
    } catch (const std::exception& e) {
        SpacetimeDb::log(std::string("Initialization failed: ") + e.what(), LogLevel::Error);
    }
}