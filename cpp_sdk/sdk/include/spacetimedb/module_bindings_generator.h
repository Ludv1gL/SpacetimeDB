#ifndef MODULE_BINDINGS_GENERATOR_H
#define MODULE_BINDINGS_GENERATOR_H

// Forward declaration of user-defined reducer
void insert_one_u8(spacetimedb::ReducerContext ctx, spacetimedb::byte n);

// This macro will define the initialization function after OneU8 is defined
#define SPACETIMEDB_MODULE_INIT() \
    inline void initialize_module() { \
        static bool initialized = false; \
        if (initialized) return; \
        initialized = true; \
        \
        auto& module = ModuleDef::instance(); \
        \
        /* Register OneU8 table */ \
        ModuleDef::Table table; \
        table.name = "one_u8"; \
        table.is_public = true; \
        table.type = &typeid(OneU8); \
        \
        /* Add field */ \
        spacetimedb::FieldInfo field; \
        field.name = "n"; \
        field.type_id = spacetimedb::type_id<uint8_t>::value; \
        field.offset = offsetof(OneU8, n); \
        field.size = sizeof(uint8_t); \
        field.serialize = [](std::vector<uint8_t>& buf, const void* obj) { \
            const OneU8* typed_obj = static_cast<const OneU8*>(obj); \
            spacetimedb::write_value(buf, typed_obj->n); \
        }; \
        table.fields.push_back(field); \
        \
        table.write_schema = [](std::vector<uint8_t>& buf) { \
            buf.push_back(2); /* Product type */ \
            spacetimedb::write_u32(buf, 1); /* One field */ \
            buf.push_back(0); /* Some */ \
            spacetimedb::write_string(buf, "n"); \
            buf.push_back(spacetimedb::type_id<uint8_t>::value); \
        }; \
        \
        table.serialize = [](std::vector<uint8_t>& buf, const void* obj) { \
            const OneU8* typed_obj = static_cast<const OneU8*>(obj); \
            spacetimedb::write_value(buf, typed_obj->n); \
        }; \
        \
        module.add_table(std::move(table)); \
        \
        /* Register insert_one_u8 reducer */ \
        ModuleDef::Reducer reducer; \
        reducer.name = "insert_one_u8"; \
        reducer.handler = [](spacetimedb::ReducerContext& ctx, uint32_t args) { \
            spacetimedb_reducer_wrapper(insert_one_u8, ctx, args); \
        }; \
        reducer.write_params = [](std::vector<uint8_t>& buf) { \
            spacetimedb::write_u32(buf, 1); \
            buf.push_back(0); /* Some */ \
            spacetimedb::write_string(buf, "n"); \
            buf.push_back(spacetimedb::type_id<spacetimedb::byte>::value); \
        }; \
        module.reducers.push_back(std::move(reducer)); \
    }

#endif // MODULE_BINDINGS_GENERATOR_H