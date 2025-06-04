#include <spacetimedb/spacetimedb.h>
#include <spacetimedb/sdk/reducer_context_enhanced.h>
#include <vector>
#include <algorithm>

// Game-related tables
struct Player {
    uint32_t id;
    std::string name;
    spacetimedb::Identity identity;
    uint32_t score;
    uint64_t joined_at;  // timestamp in microseconds
};

struct GameSession {
    uint32_t id;
    std::string name;
    uint32_t host_player_id;
    uint64_t started_at;
    bool is_active;
};

struct ChatMessage {
    uint32_t id;
    uint32_t player_id;
    std::string message;
    uint64_t sent_at;
};

// Field registration for complex types
SPACETIMEDB_REGISTER_FIELDS(Player,
    SPACETIMEDB_FIELD(Player, id, uint32_t);
    SPACETIMEDB_FIELD(Player, name, std::string);
    SPACETIMEDB_FIELD(Player, identity, spacetimedb::Identity);
    SPACETIMEDB_FIELD(Player, score, uint32_t);
    SPACETIMEDB_FIELD(Player, joined_at, uint64_t);
)

SPACETIMEDB_REGISTER_FIELDS(GameSession,
    SPACETIMEDB_FIELD(GameSession, id, uint32_t);
    SPACETIMEDB_FIELD(GameSession, name, std::string);
    SPACETIMEDB_FIELD(GameSession, host_player_id, uint32_t);
    SPACETIMEDB_FIELD(GameSession, started_at, uint64_t);
    SPACETIMEDB_FIELD(GameSession, is_active, bool);
)

SPACETIMEDB_REGISTER_FIELDS(ChatMessage,
    SPACETIMEDB_FIELD(ChatMessage, id, uint32_t);
    SPACETIMEDB_FIELD(ChatMessage, player_id, uint32_t);
    SPACETIMEDB_FIELD(ChatMessage, message, std::string);
    SPACETIMEDB_FIELD(ChatMessage, sent_at, uint64_t);
)

// Register tables
SPACETIMEDB_TABLE(Player, player, true)
SPACETIMEDB_TABLE(GameSession, game_session, true)
SPACETIMEDB_TABLE(ChatMessage, chat_message, true)

// Initialize the module
SPACETIMEDB_REDUCER(init, spacetimedb::ReducerContext ctx) {
    LOG_INFO("Game module initialized");
    auto module_id = ctx.identity();
    LOG_INFO("Module identity: 0x" + spacetimedb::bytes_to_hex(module_id.bytes.data(), module_id.bytes.size()));
    LOG_INFO("Initialization time: " + std::to_string(ctx.timestamp_millis()) + " ms");
}

// Handle client connections
SPACETIMEDB_REDUCER(client_connected, spacetimedb::ReducerContext ctx) {
    LOG_INFO("Client connected!");
    LOG_INFO("Client identity: 0x" + spacetimedb::bytes_to_hex(ctx.sender.bytes.data(), ctx.sender.bytes.size()));
    
    if (ctx.connection_id) {
        LOG_INFO("Connection ID: 0x" + spacetimedb::bytes_to_hex(ctx.connection_id->bytes.data(), ctx.connection_id->bytes.size()));
    }
    
    // Create a new player for this connection
    Player new_player{
        0,  // auto-generated ID
        "Player_" + std::to_string(ctx.random_range<uint32_t>(1000, 9999)),
        ctx.sender,
        0,  // initial score
        ctx.timestamp_micros()
    };
    
    ctx.db.table<Player>("player").insert(new_player);
    LOG_INFO("Created player: " + new_player.name);
}

// Handle client disconnections
SPACETIMEDB_REDUCER(client_disconnected, spacetimedb::ReducerContext ctx) {
    LOG_INFO("Client disconnected: 0x" + spacetimedb::bytes_to_hex(ctx.sender.bytes.data(), ctx.sender.bytes.size()));
    
    // Find and log the disconnecting player
    auto players = ctx.db.table<Player>("player");
    for (const auto& player : players.iter()) {
        if (player.identity == ctx.sender) {
            LOG_INFO("Player " + player.name + " has left the game");
            // Note: We keep the player record for history
            break;
        }
    }
}

// Create a new game session
SPACETIMEDB_REDUCER(create_game, spacetimedb::ReducerContext ctx, std::string session_name) {
    LOG_INFO("Creating game session: " + session_name);
    
    // Find the player creating the game
    auto players = ctx.db.table<Player>("player");
    uint32_t host_id = 0;
    
    for (const auto& player : players.iter()) {
        if (player.identity == ctx.sender) {
            host_id = player.id;
            LOG_INFO("Host player: " + player.name);
            break;
        }
    }
    
    if (host_id == 0) {
        LOG_ERROR("Player not found for sender: 0x" + spacetimedb::bytes_to_hex(ctx.sender.bytes.data(), ctx.sender.bytes.size()));
        return;
    }
    
    // Create the game session
    GameSession session{
        0,  // auto-generated ID
        session_name,
        host_id,
        ctx.timestamp_micros(),
        true
    };
    
    ctx.db.table<GameSession>("game_session").insert(session);
    LOG_INFO("Game session created at " + std::to_string(ctx.timestamp_millis()) + " ms");
}

// Send a chat message
SPACETIMEDB_REDUCER(send_message, spacetimedb::ReducerContext ctx, std::string message) {
    // Find the player sending the message
    auto players = ctx.db.table<Player>("player");
    uint32_t player_id = 0;
    std::string player_name;
    
    for (const auto& player : players.iter()) {
        if (player.identity == ctx.sender) {
            player_id = player.id;
            player_name = player.name;
            break;
        }
    }
    
    if (player_id == 0) {
        LOG_WARN("Unknown player tried to send message");
        return;
    }
    
    // Store the chat message
    ChatMessage chat{
        0,  // auto-generated ID
        player_id,
        message,
        ctx.timestamp_micros()
    };
    
    ctx.db.table<ChatMessage>("chat_message").insert(chat);
    LOG_INFO("[" + player_name + "]: " + message);
}

// Update player score with random bonus
SPACETIMEDB_REDUCER(update_score, spacetimedb::ReducerContext ctx, uint32_t base_points) {
    auto players = ctx.db.table<Player>("player");
    
    // Find the player
    for (auto& player : players.iter()) {
        if (player.identity == ctx.sender) {
            // Add random bonus (10-50% of base points)
            uint32_t bonus = ctx.random_range<uint32_t>(
                base_points / 10, 
                base_points / 2
            );
            
            player.score += base_points + bonus;
            
            LOG_INFO("Player " + player.name + " scored " + 
                    std::to_string(base_points) + " points + " +
                    std::to_string(bonus) + " bonus = " +
                    std::to_string(player.score) + " total");
            
            // Update the player record
            players.update(player);
            break;
        }
    }
}

// Generate random event
SPACETIMEDB_REDUCER(random_event, spacetimedb::ReducerContext ctx) {
    LOG_INFO("Random event triggered at " + std::to_string(ctx.timestamp_millis()) + " ms");
    
    // Generate different types of events randomly
    uint32_t event_type = ctx.random_range<uint32_t>(0, 3);
    
    switch (event_type) {
        case 0: {
            // Random score bonus for all players
            uint32_t bonus = ctx.random_range<uint32_t>(10, 100);
            LOG_INFO("BONUS EVENT: All players receive " + std::to_string(bonus) + " points!");
            
            auto players = ctx.db.table<Player>("player");
            for (auto& player : players.iter()) {
                player.score += bonus;
                players.update(player);
            }
            break;
        }
        case 1: {
            // Random system message
            std::vector<std::string> messages = {
                "The winds of fortune blow strongly!",
                "A mysterious force energizes the arena!",
                "The game master smiles upon you!",
                "Chaos reigns supreme!"
            };
            
            uint32_t msg_idx = ctx.random_range<uint32_t>(0, messages.size() - 1);
            
            ChatMessage system_msg{
                0,
                0,  // System message (no player)
                "[SYSTEM] " + messages[msg_idx],
                ctx.timestamp_micros()
            };
            
            ctx.db.table<ChatMessage>("chat_message").insert(system_msg);
            LOG_INFO("System message: " + messages[msg_idx]);
            break;
        }
        case 2: {
            // Log module statistics
            auto player_count = ctx.db.table<Player>("player").count();
            auto session_count = ctx.db.table<GameSession>("game_session").count();
            auto message_count = ctx.db.table<ChatMessage>("chat_message").count();
            
            LOG_INFO("=== Module Statistics ===");
            LOG_INFO("Total players: " + std::to_string(player_count));
            LOG_INFO("Total sessions: " + std::to_string(session_count));
            LOG_INFO("Total messages: " + std::to_string(message_count));
            LOG_INFO("Module uptime: " + 
                    std::to_string((ctx.timestamp_micros() - ctx.identity().get_bytes()[0]) / 1000000) + 
                    " seconds");
            break;
        }
        default:
            LOG_INFO("Unknown event type: " + std::to_string(event_type));
    }
}

// Debug: Show context information
SPACETIMEDB_REDUCER(debug_context, spacetimedb::ReducerContext ctx) {
    LOG_INFO("=== Reducer Context Debug Info ===");
    LOG_INFO(ctx.to_string());
    
    // Show module identity
    auto module_id = ctx.identity();
    LOG_INFO("Module Identity: 0x" + spacetimedb::bytes_to_hex(module_id.bytes.data(), module_id.bytes.size()));
    
    // Test random number generation
    LOG_INFO("Random numbers:");
    LOG_INFO("  Random u32: " + std::to_string(ctx.random<uint32_t>()));
    LOG_INFO("  Random float: " + std::to_string(ctx.random<float>()));
    LOG_INFO("  Random range [1-100]: " + std::to_string(ctx.random_range(1, 100)));
    
    // Show timing information
    auto now_micros = ctx.timestamp_micros();
    auto now_millis = ctx.timestamp_millis();
    LOG_INFO("Current time: " + std::to_string(now_micros) + " μs (" + 
             std::to_string(now_millis) + " ms)");
    
    // Connection status
    if (ctx.is_connected_client()) {
        LOG_INFO("Called by connected client");
    } else if (ctx.is_module()) {
        LOG_INFO("Called by module itself (scheduled/init)");
    }
}