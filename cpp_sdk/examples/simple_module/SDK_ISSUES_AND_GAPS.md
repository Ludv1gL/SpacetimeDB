# SpacetimeDB C++ SDK: Issues and Missing Pieces

## 🔴 Critical Issues (Blocking All Development)

### 1. Header File Architecture Breakdown
**Impact**: No modules can use the full SDK
- Conflicting type definitions between headers
- Circular dependencies
- Namespace confusion between global and `spacetimedb::` namespace
- Include order dependencies

**Specific Conflicts**:
```cpp
// In algebraic_type.h
struct AlgebraicType { ... };

// In bsatn/algebraic_type.h  
using AlgebraicType = SpacetimeDb::bsatn::AlgebraicType;
// Conflict!
```

### 2. ReducerContext Definition Chaos
**Impact**: Cannot use reducer context features
- Multiple incompatible definitions
- `IReducer.h` references non-existent `spacetimedb::ReducerContext`
- Type alias vs struct definition conflicts

### 3. Macro Signature Inconsistencies
**Impact**: Built-in reducers unusable
- `SPACETIMEDB_INIT()` vs `SPACETIMEDB_INIT(function_name)`
- Parameter mismatches between declarations

## 🟡 Major Missing Features

### 1. Query Operations (Feature 4)
- ❌ Filter operations  
- ❌ Map/transform operations
- ❌ Aggregations (sum, average, min, max)
- ❌ Join operations
- ❌ Complex predicates
- ❌ Sorting

### 2. Index Management (Feature 5)
- ❌ Index creation API
- ❌ Index usage in queries
- ❌ Composite indexes
- ❌ Index hints

### 3. Schema Management (Feature 6)
- ❌ Dynamic schema inspection
- ❌ Schema evolution
- ❌ Migration callbacks

### 4. Transaction Support (Feature 9)
- ❌ Transaction begin/commit/rollback
- ❌ Isolation levels
- ❌ Savepoints
- ❌ Deadlock handling

### 5. Scheduled Reducers (Feature 10)
- ❌ `SPACETIMEDB_SCHEDULED_REDUCER` macro
- ❌ Cron expressions
- ❌ Interval scheduling
- ❌ Schedule management API

### 6. Versioning System (Feature 11)
- ❌ `SPACETIMEDB_VERSIONED_TABLE` macro
- ❌ Version migration callbacks
- ❌ Schema evolution handling
- ❌ Backward compatibility

### 7. Credentials & Permissions (Feature 12)
- ❌ Permission checks in reducers
- ❌ Role-based access control
- ❌ Row-level security implementation
- ❌ Credential storage and retrieval

## 🟢 Working Features (via minimal_sdk.h workaround)

### What Works:
- ✅ Basic table registration (manual)
- ✅ Simple reducer definitions
- ✅ Direct BSATN encoding
- ✅ FFI imports/exports
- ✅ Basic logging
- ✅ Simple insert operations

### Limitations of Working Features:
- No type safety
- Manual BSATN encoding required
- No error handling
- No advanced operations
- No SDK conveniences

## 📊 SDK Completion Status by Component

| Component | Status | Completion |
|-----------|--------|------------|
| Core Headers | 🔴 Broken | 0% |
| Table Operations | 🟡 Partial | 30% |
| Reducer System | 🟡 Partial | 25% |
| Built-in Reducers | 🔴 Broken | 0% |
| Query Operations | ❌ Missing | 0% |
| Index Management | ❌ Missing | 0% |
| Transactions | ❌ Missing | 0% |
| Scheduled Reducers | ❌ Missing | 0% |
| Versioning | ❌ Missing | 0% |
| Credentials | ❌ Missing | 0% |
| Error Handling | 🟡 Partial | 20% |
| BSATN Support | 🟡 Partial | 40% |

**Overall SDK Completion: ~15%**

## 🛠️ Required Fixes Priority Order

### Phase 1: Make SDK Compilable (P0)
1. Resolve header conflicts
2. Fix ReducerContext definitions  
3. Standardize macro signatures
4. Fix include paths
5. Create proper include guards

### Phase 2: Core Functionality (P1)
1. Complete table operations
2. Implement error handling
3. Fix built-in reducers
4. Add basic query support

### Phase 3: Advanced Features (P2)
1. Transaction support
2. Index management
3. Scheduled reducers
4. Query optimization

### Phase 4: Enterprise Features (P3)
1. Versioning system
2. Credentials/permissions
3. Schema management
4. Performance tools

## 💡 Recommendations for SDK Team

### Immediate Actions:
1. **Header Cleanup Sprint**
   - Audit all headers for conflicts
   - Establish clear ownership of types
   - Create dependency graph
   - Fix include order issues

2. **Create Test Suite**
   - Add compilation tests for all examples
   - Prevent regression of fixes
   - Test each feature in isolation

3. **Documentation**
   - Document current workarounds
   - Create migration plan
   - Update examples to working state

### Architecture Recommendations:
1. **Clear Namespace Strategy**
   ```cpp
   namespace spacetimedb {
       namespace core { ... }
       namespace sdk { ... }
       namespace internal { ... }
   }
   ```

2. **Single Source of Truth**
   - One definition per type
   - Clear public vs internal API
   - Consistent naming conventions

3. **Modular Headers**
   - Feature-specific headers
   - Minimal dependencies
   - Optional advanced features

## 📝 Notes for Developers

### Current Workaround:
Use `minimal_sdk.h` for basic functionality:
```cpp
#include "minimal_sdk.h"
// Instead of:
// #include <spacetimedb/spacetimedb.h>
```

### What to Avoid:
- Don't use full SDK headers until fixed
- Don't rely on advanced features
- Don't expect type safety from current SDK

### Migration Path:
1. Start with minimal_sdk.h
2. Implement core logic with manual BSATN
3. Plan to migrate when SDK is fixed
4. Keep business logic separate from SDK calls