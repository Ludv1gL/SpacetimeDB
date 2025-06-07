# C++ Module Library Feature Parity Tracker

This document tracks the **actual** feature parity status between Rust and C++ SpacetimeDB module libraries.

## Current Status: ~65% Feature Parity

The C++ Module Library has solid core functionality but significant gaps remain for full parity with Rust.

## Feature Comparison Matrix

| Feature Category | Rust Status | C++ Status | Priority | Implementation Needed |
|------------------|-------------|------------|----------|----------------------|
| **Core Module System** |
| Module exports/imports | ✅ Complete | ✅ Complete | - | None |
| Table definitions | ✅ Complete | ✅ Complete | - | None (X-Macro working) |
| Reducer registration | ✅ Complete | ✅ Complete | - | None (recently fixed) |
| BSATN serialization | ✅ Complete | ✅ Complete | - | None (primitives working) |
| **Type System** |
| Primitive types | ✅ Complete | ✅ Complete | - | None |
| Structs (Products) | ✅ Complete | ✅ Complete | - | None (X-Macros working) |
| Enums (Sum types) | ✅ Complete | ❌ Missing | **HIGH** | BSATN sum type serialization |
| Arrays/Vectors | ✅ Complete | ⚠️ Partial | **HIGH** | Array type registration |
| Optional types | ✅ Complete | ⚠️ Partial | **HIGH** | Option<T> BSATN support |
| Nested types | ✅ Complete | ⚠️ Partial | **MEDIUM** | Complex struct nesting |
| **Index System** |
| Primary keys | ✅ Complete | ❌ Missing | **HIGH** | Unique constraint enforcement |
| Unique constraints | ✅ Complete | ❌ Missing | **HIGH** | Validation in insert/update |
| BTree indexes | ✅ Complete | ⚠️ Header Only | **HIGH** | FFI implementation + range queries |
| Hash indexes | ✅ Complete | ❌ Missing | **MEDIUM** | Index type implementation |
| Multi-column indexes | ✅ Complete | ❌ Missing | **MEDIUM** | Composite key support |
| **Query Operations** |
| Basic table ops | ✅ Complete | ✅ Complete | - | None (CRUD working) |
| Filter operations | ✅ Complete | ✅ Complete | - | None (basic filtering) |
| Range queries | ✅ Complete | ❌ Missing | **HIGH** | BTree index integration |
| Join operations | ✅ Complete | ❌ Missing | **MEDIUM** | Table relationship queries |
| Aggregations | ✅ Complete | ❌ Missing | **MEDIUM** | COUNT, SUM, AVG, etc. |
| **Advanced Features** |
| Transactions | ✅ Complete | ❌ Missing | **HIGH** | Transaction context & rollback |
| Row-level security | ✅ Complete | ❌ Missing | **HIGH** | Access control filters |
| Scheduled reducers | ✅ Complete | ⚠️ Header Only | **HIGH** | Duration scheduling integration |
| Built-in reducers | ✅ Complete | ✅ Complete | - | None (working) |
| Error handling | ✅ Complete | ⚠️ Basic | **MEDIUM** | Enhanced error propagation |
| **Client Integration** |
| Authentication | ✅ Complete | ⚠️ Header Only | **HIGH** | JWT token parsing |
| Identity management | ✅ Complete | ✅ Complete | - | None (working) |
| Connection lifecycle | ✅ Complete | ✅ Complete | - | None (working) |

## Critical Implementation Gaps

### 1. Enum Support (Sum Types) ⚠️
**Current**: Basic infrastructure exists, macro implementation in progress  
**Needed**: Complete SPACETIMEDB_ENUM_UNIT macro and test integration  
**Impact**: Cannot model discriminated unions, critical for many data models  
**Progress**: SumType classes implemented, macro syntax defined, compilation framework established  

### 2. Primary Key Enforcement ❌  
**Current**: Tables accept duplicate primary keys  
**Needed**: Unique constraint validation on insert/update  
**Impact**: Data integrity violations possible  

### 3. BTree Index Operations ⚠️
**Current**: Headers exist but no implementation  
**Needed**: FFI calls for `btree_scan`, range queries  
**Impact**: Poor query performance, no range filtering  

### 4. Transaction Support ❌
**Current**: No transaction context  
**Needed**: Begin/commit/rollback operations  
**Impact**: No ACID guarantees, data consistency issues  

### 5. Scheduled Reducers ⚠️
**Current**: Headers only, no registration  
**Needed**: Integration with module description system  
**Impact**: Cannot implement time-based automation  

## Working Core Features ✅

### Table Operations
- ✅ Insert, update, delete operations
- ✅ Basic filtering with predicates  
- ✅ Table iteration and scanning
- ✅ Row counting

### Type System  
- ✅ All primitive types (i8, u8, i16, u16, i32, u32, i64, u64, f32, f64, bool, string)
- ✅ Product types (structs) via X-Macros
- ✅ BSATN serialization for primitives and products

### Reducer System
- ✅ Parameter type registration (recently fixed)
- ✅ Built-in lifecycle reducers (init, client_connected, client_disconnected)
- ✅ Context passing with database access

### Development Tools
- ✅ X-Macro table registration system
- ✅ CMake build integration
- ✅ LogStopwatch performance monitoring
- ✅ Multi-level logging (LOG_INFO, LOG_DEBUG, LOG_TRACE)

## Implementation Priority Roadmap

### Phase 1: Critical Features (HIGH Priority)
**Target: 2-3 weeks**

1. **Enum Support**
   - Implement `AlgebraicType::Sum` serialization
   - Add enum registration macros
   - Test with simple enum types

2. **Primary Key Enforcement**  
   - Add unique constraint checking
   - Integrate with table insert/update operations
   - Error handling for constraint violations

3. **BTree Index Implementation**
   - Implement `btree_scan` FFI integration
   - Add range query support
   - Test index filtering operations

### Phase 2: Advanced Features (HIGH Priority)  
**Target: 2-3 weeks**

4. **Transaction Support**
   - Add transaction context to reducers
   - Implement begin/commit/rollback
   - Test rollback scenarios

5. **Scheduled Reducers**
   - Integrate with module description system
   - Add duration-based scheduling
   - Test time-based execution

### Phase 3: Remaining Gaps (MEDIUM Priority)
**Target: 2-3 weeks**

6. **Enhanced Type System**
   - Array/Vector type registration
   - Optional type support
   - Nested type handling

7. **Query Operations**
   - Join operations between tables
   - Aggregation functions
   - Advanced filtering

## Current Known Issues

### Fixed ✅
1. **Reducer Parameter Types**: Fixed macro to use correct type registration
2. **Table Registration**: X-Macro system working correctly  

### Active 🔧
1. **SpacetimeDB CLI Bug**: Parameter values truncated to u8 (not C++ library bug)
2. **Macro Conflicts**: Multiple macro definition systems need consolidation
3. **Array Type Registration**: Arrays not properly registered in type system

### Backlog 📋
1. Memory management in WASI shims
2. Error context propagation  
3. Performance optimization opportunities

## Success Metrics

**Phase 1 Complete (75% parity)**: Enum support, primary keys, basic indexes  
**Phase 2 Complete (85% parity)**: Transactions, scheduled reducers  
**Phase 3 Complete (95% parity)**: Enhanced types, query operations  

## Testing Strategy

### Current Test Coverage
- ✅ Module compilation and publishing
- ✅ Basic table operations  
- ✅ Primitive type handling
- ✅ Reducer execution

### Needed Test Coverage  
- [ ] Enum serialization/deserialization
- [ ] Constraint violation handling
- [ ] Index query performance
- [ ] Transaction rollback scenarios
- [ ] Scheduled reducer execution
- [ ] Error handling edge cases

## Implementation Notes

- **X-Macro Pattern**: Successfully used for table registration, consider for other features
- **BSATN Integration**: Core serialization working, need to extend for sum types
- **FFI Layer**: Solid foundation, need to add more index and transaction operations
- **Error Handling**: Basic infrastructure exists, need enhanced error context

---

**Reality Check**: The C++ library has a solid foundation but significant work remains for true feature parity. Focus on HIGH priority items first to achieve production readiness.

## Recent Progress (December 2024)

### ✅ **Completed**
- **Comprehensive Parity Assessment**: Realistic evaluation showing ~65% feature parity
- **Enum Infrastructure**: SPACETIMEDB_ENUM_UNIT macro, SumType classes, BSATN integration
- **Core Validation**: Verified X-Macro table system and basic module compilation
- **Documentation**: Accurate feature comparison matrix and implementation roadmap

### 🔄 **In Progress**
- **Primary Key Enforcement**: Next critical feature for data integrity
- **Enum Integration**: Complete integration with X-Macro table registration
- **BTree Index Operations**: FFI implementation for range queries

### 🎯 **Immediate Next Steps**
1. Implement unique constraint validation on table inserts
2. Complete enum type registration with module description system
3. Add BTree index FFI calls for range queries

*Last Updated: December 7, 2024 - Active development towards feature parity*