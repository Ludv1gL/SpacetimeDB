# SpacetimeDB C++ SDK - Critical Refactoring Plan

## Executive Summary

Testing on June 4, 2025 revealed that the SpacetimeDB C++ SDK, while having extensive infrastructure, is fundamentally broken due to header conflicts and missing implementations. This document outlines a plan to fix these critical issues.

## Current State Assessment

### What Actually Works (15%)
- Basic table registration and insert via minimal_sdk.h
- BSATN serialization for simple types
- Module exports (__describe_module__, __call_reducer__)

### What Doesn't Work (85%)
- Main spacetimedb.h header causes compilation failures
- All advanced features (queries, indexes, transactions, etc.)
- Built-in reducers (header conflicts)
- Enhanced context features
- Table operations beyond insert

## Root Cause Analysis

### 1. Header Architecture Problems
```
spacetimedb.h
├── Includes spacetimedb_sdk.h → defines ReducerContext
├── Includes reducer_context_enhanced.h → redefines ReducerContext
├── Includes builtin_reducers.h → expects ReducerContext
└── Result: Multiple definition errors
```

### 2. Missing FFI Implementation
- Functions declared but not implemented:
  - `_table_iter_start`, `_table_iter_next`
  - `_delete_by_col_eq` 
  - `_update_by_col_eq`
  - Index-related FFI functions
  - Transaction FFI functions

### 3. Integration Layer Missing
- Headers created in isolation
- No proper integration testing
- Features built on top of non-working foundation

## Refactoring Plan

### Phase 1: Fix Header Architecture (1-2 days)

1. **Create Clean Header Hierarchy**
   ```
   spacetimedb_core.h     // Core types only, no implementations
   ├── spacetimedb_ffi.h  // FFI declarations
   ├── spacetimedb_types.h // Basic types (Identity, etc.)
   └── spacetimedb_bsatn.h // BSATN serialization
   
   spacetimedb.h          // Main user-facing header
   ├── includes core
   ├── includes table operations
   ├── includes reducers
   └── includes context (single definition)
   ```

2. **Eliminate Circular Dependencies**
   - Use forward declarations
   - Move implementations to .cpp files
   - Create clear dependency order

3. **Single Definition Rule**
   - One ReducerContext definition
   - One Identity definition
   - Proper namespacing

### Phase 2: Implement Missing FFI (2-3 days)

1. **Priority FFI Functions**
   ```cpp
   // Table operations
   extern "C" {
       Iterator _table_iter_start(TableId table_id);
       bool _table_iter_next(Iterator iter, uint8_t* buffer, size_t* len);
       void _table_iter_drop(Iterator iter);
       
       uint16_t _delete_by_col_eq(TableId table, ColId col, 
                                  const uint8_t* val, size_t len);
       uint16_t _update_by_col_eq(TableId table, ColId col,
                                  const uint8_t* val, size_t val_len,
                                  const uint8_t* row, size_t row_len);
   }
   ```

2. **Create FFI Test Suite**
   - Test each FFI function individually
   - Verify WASM imports/exports
   - Document expected behavior

### Phase 3: Rebuild Features Incrementally (3-5 days)

1. **Core Features First**
   - Table operations (iter, delete, update)
   - Basic context (no enhancements initially)
   - Simple reducers

2. **Test Each Feature**
   - Create minimal test for each feature
   - Ensure compilation and runtime success
   - Document working examples

3. **Advanced Features Second**
   - Built-in reducers
   - Enhanced context
   - Logging system
   - Error handling

4. **Complex Features Last**
   - Indexes and queries
   - Transactions
   - Constraints
   - Scheduling

### Phase 4: Integration Testing (1-2 days)

1. **Create Integration Test Suite**
   - Test feature combinations
   - Stress test with complex modules
   - Performance benchmarking

2. **Update Documentation**
   - Mark what actually works
   - Provide migration guide
   - Update examples

## Implementation Strategy

### Week 1: Foundation
- Day 1-2: Fix header architecture
- Day 3-4: Implement core FFI functions
- Day 5: Test basic functionality

### Week 2: Features
- Day 1-2: Rebuild table operations
- Day 3-4: Rebuild reducer system
- Day 5: Advanced features

### Week 3: Polish
- Day 1-2: Integration testing
- Day 3-4: Documentation
- Day 5: Release preparation

## Success Criteria

1. **All examples compile** with main spacetimedb.h
2. **Core features work** end-to-end
3. **No header conflicts** or circular dependencies
4. **FFI functions implemented** and tested
5. **Clear documentation** of what works/doesn't work

## Risk Mitigation

1. **Backward Compatibility**
   - Keep minimal_sdk.h as fallback
   - Provide migration guide
   - Version the SDK properly

2. **Testing Strategy**
   - Test after each change
   - Maintain working baseline
   - Use CI/CD for regression testing

3. **Communication**
   - Update users about breaking changes
   - Provide timeline for fixes
   - Be honest about current limitations

## Conclusion

The C++ SDK requires fundamental restructuring before adding new features. This plan prioritizes fixing the foundation over adding capabilities. Once complete, the SDK will provide a solid base for future development.

**Estimated Timeline**: 2-3 weeks for full refactoring
**Priority**: Critical - SDK is unusable without these fixes
**Impact**: High - Enables C++ module development