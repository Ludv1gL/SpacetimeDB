#include <spacetimedb/spacetimedb.h>
#include <spacetimedb/spacetimedb_autogen.h>

using namespace SpacetimeDb;

// Example: Document management system with row-level security
struct Document {
    uint64_t id;
    std::string title;
    std::string content;
    SpacetimeDb::sdk::Identity owner_id;
    std::string visibility;  // "private", "team", "public"
    uint32_t team_id;
};

SPACETIMEDB_REGISTER_TYPE(Document,
    SPACETIMEDB_FIELD(Document, id),
    SPACETIMEDB_FIELD(Document, title),
    SPACETIMEDB_FIELD(Document, content),
    SPACETIMEDB_FIELD(Document, owner_id),
    SPACETIMEDB_FIELD(Document, visibility),
    SPACETIMEDB_FIELD(Document, team_id)
)

SPACETIMEDB_TABLE(Document, "documents", true)

// Team membership table
struct TeamMember {
    uint32_t team_id;
    SpacetimeDb::sdk::Identity user_id;
    std::string role;  // "member", "admin"
};

SPACETIMEDB_REGISTER_TYPE(TeamMember,
    SPACETIMEDB_FIELD(TeamMember, team_id),
    SPACETIMEDB_FIELD(TeamMember, user_id),
    SPACETIMEDB_FIELD(TeamMember, role)
)

SPACETIMEDB_TABLE(TeamMember, "team_members", true)

// RLS Policies for Document table

// SELECT: Users can see documents they own, public documents, or team documents they're members of
SPACETIMEDB_RLS_SELECT(documents, view_documents, 
    SpacetimeDb::rls::or_conditions({
        // User owns the document
        SpacetimeDb::rls::user_owns("owner_id"),
        // Document is public
        "visibility = 'public'",
        // User is member of the team
        SpacetimeDb::rls::and_conditions({
            "visibility = 'team'",
            "EXISTS (SELECT 1 FROM team_members WHERE team_members.team_id = documents.team_id AND team_members.user_id = current_user_identity())"
        })
    })
)

// INSERT: Users can only create documents they own
SPACETIMEDB_RLS_INSERT(documents, create_documents,
    SpacetimeDb::rls::user_owns("owner_id")
)

// UPDATE: Users can only update their own documents or team documents where they're admin
SPACETIMEDB_RLS_UPDATE(documents, update_documents,
    SpacetimeDb::rls::or_conditions({
        // User owns the document
        SpacetimeDb::rls::user_owns("owner_id"),
        // User is admin of the team
        SpacetimeDb::rls::and_conditions({
            "visibility = 'team'",
            "EXISTS (SELECT 1 FROM team_members WHERE team_members.team_id = documents.team_id " 
            "AND team_members.user_id = current_user_identity() AND team_members.role = 'admin')"
        })
    })
)

// DELETE: Only document owners can delete
SPACETIMEDB_RLS_DELETE(documents, delete_documents,
    SpacetimeDb::rls::user_owns("owner_id")
)

// RLS Policies for TeamMember table

// SELECT: Users can see team members of teams they belong to
SPACETIMEDB_RLS_SELECT(team_members, view_team_members,
    "EXISTS (SELECT 1 FROM team_members tm WHERE tm.team_id = team_members.team_id AND tm.user_id = current_user_identity())"
)

// INSERT: Only team admins can add members
SPACETIMEDB_RLS_INSERT(team_members, add_team_members,
    "EXISTS (SELECT 1 FROM team_members tm WHERE tm.team_id = team_members.team_id " 
    "AND tm.user_id = current_user_identity() AND tm.role = 'admin')"
)

// UPDATE: Only team admins can update roles
SPACETIMEDB_RLS_UPDATE(team_members, update_team_members,
    "EXISTS (SELECT 1 FROM team_members tm WHERE tm.team_id = team_members.team_id " 
    "AND tm.user_id = current_user_identity() AND tm.role = 'admin')"
)

// DELETE: Team admins can remove members, members can remove themselves
SPACETIMEDB_RLS_DELETE(team_members, remove_team_members,
    SpacetimeDb::rls::or_conditions({
        // User is admin of the team
        "EXISTS (SELECT 1 FROM team_members tm WHERE tm.team_id = team_members.team_id " 
        "AND tm.user_id = current_user_identity() AND tm.role = 'admin')",
        // User is removing themselves
        "team_members.user_id = current_user_identity()"
    })
)

// Example: Admin-only table with role-based RLS
struct SystemConfig {
    std::string key;
    std::string value;
};

SPACETIMEDB_REGISTER_TYPE(SystemConfig,
    SPACETIMEDB_FIELD(SystemConfig, key),
    SPACETIMEDB_FIELD(SystemConfig, value)
)

SPACETIMEDB_TABLE(SystemConfig, "system_config", false)  // Private table

// Only system admins can access this table
SPACETIMEDB_RLS_SELECT(system_config, admin_only_read,
    SpacetimeDb::rls::user_has_role("system_admin")
)

SPACETIMEDB_RLS_INSERT(system_config, admin_only_write,
    SpacetimeDb::rls::user_has_role("system_admin")
)

SPACETIMEDB_RLS_UPDATE(system_config, admin_only_update,
    SpacetimeDb::rls::user_has_role("system_admin")
)

SPACETIMEDB_RLS_DELETE(system_config, admin_only_delete,
    SpacetimeDb::rls::user_has_role("system_admin")
)

// Reducers
SPACETIMEDB_REDUCER(create_document, UserDefined, ctx,
    std::string title, std::string content, std::string visibility, uint32_t team_id)
{
    Document doc{
        .id = 0,  // Auto-generated
        .title = title,
        .content = content,
        .owner_id = ctx.sender,
        .visibility = visibility,
        .team_id = team_id
    };
    
    auto table = get_Document_table();
    auto created = table.insert(doc);
    
    LOG_INFO("Created document: " + title);
}

SPACETIMEDB_REDUCER(create_team, UserDefined, ctx, uint32_t team_id)
{
    // Creator becomes admin of the team
    TeamMember member{
        .team_id = team_id,
        .user_id = ctx.sender,
        .role = "admin"
    };
    
    auto table = get_TeamMember_table();
    table.insert(member);
    
    LOG_INFO("Created team with ID: " + std::to_string(team_id));
}

SPACETIMEDB_REDUCER(add_team_member, UserDefined, ctx,
    uint32_t team_id, SpacetimeDb::sdk::Identity user_id, std::string role)
{
    TeamMember member{
        .team_id = team_id,
        .user_id = user_id,
        .role = role
    };
    
    auto table = get_TeamMember_table();
    table.insert(member);
    
    LOG_INFO("Added team member");
}

// Init reducer
SPACETIMEDB_INIT(init_rls_example, ctx)
{
    LOG_INFO("RLS example module initialized");
    
    // Note: In a real system, you'd set up initial admin users
    // and system configuration here
}