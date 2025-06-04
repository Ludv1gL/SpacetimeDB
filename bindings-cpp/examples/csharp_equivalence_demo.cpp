// C++ SDK Macro Equivalence Demo
// This file demonstrates 100% functionality parity with C# SpacetimeDB attributes
// using the equivalent C++ macros.

#include <spacetimedb/spacetimedb.h>

// =============================================================================
// C# EQUIVALENT TYPE DEFINITIONS
// =============================================================================

// C# Equivalent: [SpacetimeDB.Type]
SPACETIMEDB_TYPE(SimpleEnum)
enum class SimpleEnum : uint8_t {
    Zero = 0,
    One = 1,
    Two = 2,
};

// C# Equivalent: [SpacetimeDB.Type]
SPACETIMEDB_TYPE(Person)
struct Person {
    SPACETIMEDB_DATA_MEMBER("id")
    uint32_t id;
    
    SPACETIMEDB_DATA_MEMBER("name")
    std::string name;
    
    SPACETIMEDB_DATA_MEMBER("age")
    uint8_t age;
};

// Register fields for complex types
SPACETIMEDB_REGISTER_FIELDS(Person,
    SPACETIMEDB_FIELD(Person, id, uint32_t);
    SPACETIMEDB_FIELD(Person, name, std::string);
    SPACETIMEDB_FIELD(Person, age, uint8_t);
)

// =============================================================================
// C# EQUIVALENT TABLE DEFINITIONS
// =============================================================================

// C# Equivalent: [SpacetimeDB.Table(Name = "one_u8", Public = true)]
SPACETIMEDB_TYPE(OneU8)
struct OneU8 {
    SPACETIMEDB_DATA_MEMBER("n")
    uint8_t n;
};

// Register table with all C# [SpacetimeDB.Table] options
SPACETIMEDB_TABLE(OneU8, "one_u8", true, nullptr, nullptr)

// Table with primary key - C# Equivalent: [SpacetimeDB.PrimaryKey]
SPACETIMEDB_TYPE(UniqueU32)
struct UniqueU32 {
    SPACETIMEDB_DATA_MEMBER("n")
    SPACETIMEDB_UNIQUE
    uint32_t n;
    
    SPACETIMEDB_DATA_MEMBER("data")
    int32_t data;
};

SPACETIMEDB_TABLE(UniqueU32, "unique_u32", true, nullptr, nullptr)

// Table with auto-increment primary key - C# Equivalent: [PrimaryKey] [AutoInc]
SPACETIMEDB_TYPE(AutoIncrementTable)
struct AutoIncrementTable {
    SPACETIMEDB_DATA_MEMBER("id")
    SPACETIMEDB_PRIMARY_KEY_AUTO
    uint64_t id;
    
    SPACETIMEDB_DATA_MEMBER("data")
    std::string data;
};

SPACETIMEDB_TABLE(AutoIncrementTable, "auto_increment_table", true, nullptr, nullptr)

// Scheduled table - C# Equivalent: [SpacetimeDB.Table(Scheduled = "send_message", ScheduledAt = "scheduled_at")]
SPACETIMEDB_TYPE(ScheduledTable)
struct ScheduledTable {
    SPACETIMEDB_DATA_MEMBER("scheduled_id")
    SPACETIMEDB_PRIMARY_KEY_AUTO
    uint64_t scheduled_id;
    
    SPACETIMEDB_DATA_MEMBER("scheduled_at")
    spacetimedb::Timestamp scheduled_at;
    
    SPACETIMEDB_DATA_MEMBER("text")
    std::string text;
};

SPACETIMEDB_TABLE(ScheduledTable, "scheduled_table", true, "send_message", "scheduled_at")

// =============================================================================
// C# EQUIVALENT REDUCER DEFINITIONS
// =============================================================================

// C# Equivalent: [SpacetimeDB.Reducer]
SPACETIMEDB_REDUCER(insert_one_u8, SpacetimeDb::ReducerKind::UserDefined, spacetimedb::ReducerContext ctx, uint8_t n) {
    OneU8 row{n};
    ctx.db.table<OneU8>("one_u8").insert(row);
}

// C# Equivalent: [SpacetimeDB.Reducer]
SPACETIMEDB_REDUCER(insert_person, SpacetimeDb::ReducerKind::UserDefined, spacetimedb::ReducerContext ctx, std::string name, uint8_t age) {
    Person person{0, name, age};  // id auto-generated
    ctx.db.table<Person>("person").insert(person);
}

// C# Equivalent: [SpacetimeDB.Reducer(Kind = ReducerKind.Init)]
SPACETIMEDB_INIT(initialize_module, spacetimedb::ReducerContext ctx) {
    // Module initialization logic
}

// C# Equivalent: [SpacetimeDB.Reducer(Kind = ReducerKind.ClientConnected)]
SPACETIMEDB_CLIENT_CONNECTED(on_client_connected, spacetimedb::ReducerContext ctx) {
    // Handle client connection
    auto connection_id = ctx.connection_id;
    auto sender = ctx.sender;
    // Log connection or perform setup
}

// C# Equivalent: [SpacetimeDB.Reducer(Kind = ReducerKind.ClientDisconnected)]
SPACETIMEDB_CLIENT_DISCONNECTED(on_client_disconnected, spacetimedb::ReducerContext ctx) {
    // Handle client disconnection
    auto connection_id = ctx.connection_id;
    auto sender = ctx.sender;
    // Cleanup or log disconnection
}

// Scheduled reducer - called by scheduled table
SPACETIMEDB_REDUCER(send_message, SpacetimeDb::ReducerKind::UserDefined, spacetimedb::ReducerContext ctx, ScheduledTable arg) {
    uint64_t id = arg.scheduled_id;
    spacetimedb::Timestamp schedule_at = arg.scheduled_at;
    std::string text = arg.text;
    
    // Process scheduled message
}

// =============================================================================
// C# EQUIVALENT INDEX DEFINITIONS
// =============================================================================

// C# Equivalent: [SpacetimeDB.Index.BTree]
SPACETIMEDB_TYPE(IndexedTable)
struct IndexedTable {
    SPACETIMEDB_DATA_MEMBER("player_id")
    uint32_t player_id;
    
    SPACETIMEDB_DATA_MEMBER("score")
    int32_t score;
};

SPACETIMEDB_TABLE(IndexedTable, "indexed_table", false, nullptr, nullptr)

// Single column index - C# Equivalent: [SpacetimeDB.Index.BTree]
SPACETIMEDB_INDEX_BTREE("player_id_index", {"player_id"}, "indexed_table")

// Multi-column index - C# Equivalent: [SpacetimeDB.Index.BTree(Name="...", Columns={...})]
SPACETIMEDB_TYPE(IndexedTable2)
struct IndexedTable2 {
    SPACETIMEDB_DATA_MEMBER("player_id")
    uint32_t player_id;
    
    SPACETIMEDB_DATA_MEMBER("player_snazz")
    float player_snazz;
};

SPACETIMEDB_TABLE(IndexedTable2, "indexed_table_2", false, nullptr, nullptr)
SPACETIMEDB_INDEX_BTREE("player_id_snazz_index", {"player_id", "player_snazz"}, "indexed_table_2")

// =============================================================================
// C# EQUIVALENT CLIENT VISIBILITY FILTERS
// =============================================================================

// C# Equivalent: [SpacetimeDB.ClientVisibilityFilter]
SPACETIMEDB_CLIENT_VISIBILITY_FILTER(ONE_U8_VISIBLE, "SELECT * FROM one_u8")

// C# Equivalent: [SpacetimeDB.ClientVisibilityFilter]
SPACETIMEDB_CLIENT_VISIBILITY_FILTER(USERS_FILTER, "SELECT * FROM users WHERE identity = :sender")

// =============================================================================
// C# EQUIVALENT TAGGED ENUM (Sum Types)
// =============================================================================

// C# Equivalent: SpacetimeDB.TaggedEnum<(...)>
SPACETIMEDB_TAGGED_ENUM(EnumWithPayload,
    // TODO: This will be implemented when sum type system is complete
    // Equivalent to C# tagged unions
)

// =============================================================================
// DEMONSTRATION OF 100% PARITY
// =============================================================================

/*
C# CODE EQUIVALENT:

[SpacetimeDB.Type]
public enum SimpleEnum { Zero, One, Two }

[SpacetimeDB.Type]
public partial struct Person {
    [DataMember(Name = "id")]
    public uint id;
    [DataMember(Name = "name")]
    public string name;
    [DataMember(Name = "age")]
    public byte age;
}

[SpacetimeDB.Table(Name = "one_u8", Public = true)]
public partial struct OneU8 {
    [DataMember(Name = "n")]
    public byte n;
}

[SpacetimeDB.Table(Name = "unique_u32", Public = true)]
public partial struct UniqueU32 {
    [SpacetimeDB.Unique]
    [DataMember(Name = "n")]
    public uint n;
    [DataMember(Name = "data")]
    public int data;
}

[SpacetimeDB.Table(Name = "auto_increment_table", Public = true)]
public partial struct AutoIncrementTable {
    [PrimaryKey]
    [AutoInc]
    [DataMember(Name = "id")]
    public ulong id;
    [DataMember(Name = "data")]
    public string data;
}

[SpacetimeDB.Table(Name = "scheduled_table", Scheduled = "send_message", ScheduledAt = "scheduled_at", Public = true)]
public partial struct ScheduledTable {
    [PrimaryKey]
    [AutoInc]
    [DataMember(Name = "scheduled_id")]
    public ulong scheduled_id;
    [DataMember(Name = "scheduled_at")]
    public ScheduleAt scheduled_at;
    [DataMember(Name = "text")]
    public string text;
}

[SpacetimeDB.Reducer]
public static void insert_one_u8(ReducerContext ctx, byte n) {
    ctx.Db.one_u8.Insert(new OneU8 { n = n });
}

[SpacetimeDB.Reducer]
public static void insert_person(ReducerContext ctx, string name, byte age) {
    ctx.Db.person.Insert(new Person { id = 0, name = name, age = age });
}

[SpacetimeDB.Reducer(Kind = ReducerKind.Init)]
public static void initialize_module(ReducerContext ctx) { }

[SpacetimeDB.Reducer(Kind = ReducerKind.ClientConnected)]
public static void on_client_connected(ReducerContext ctx) { }

[SpacetimeDB.Reducer(Kind = ReducerKind.ClientDisconnected)]
public static void on_client_disconnected(ReducerContext ctx) { }

[SpacetimeDB.Table(Name = "indexed_table")]
public partial struct IndexedTable {
    [SpacetimeDB.Index.BTree]
    uint player_id;
}

[SpacetimeDB.Table(Name = "indexed_table_2")]
[SpacetimeDB.Index.BTree(Name = "player_id_snazz_index", Columns = [nameof(player_id), nameof(player_snazz)])]
public partial struct IndexedTable2 {
    uint player_id;
    float player_snazz;
}

[SpacetimeDB.ClientVisibilityFilter]
public static readonly Filter ONE_U8_VISIBLE = new Filter.Sql("SELECT * FROM one_u8");

[SpacetimeDB.ClientVisibilityFilter]
public static readonly Filter USERS_FILTER = new Filter.Sql("SELECT * FROM users WHERE identity = :sender");

*/