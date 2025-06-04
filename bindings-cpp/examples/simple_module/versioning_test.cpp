/**
 * Comprehensive test for module versioning functionality
 */

#include <spacetimedb/versioning.h>
#include <cassert>
#include <iostream>

using namespace spacetimedb;

void test_version_parsing() {
    std::cout << "Testing version parsing..." << std::endl;
    
    // Basic version
    auto v1 = ModuleVersion::parse("1.2.3");
    assert(v1.major == 1);
    assert(v1.minor == 2);
    assert(v1.patch == 3);
    assert(!v1.prerelease.has_value());
    assert(!v1.metadata.has_value());
    
    // Version with prerelease
    auto v2 = ModuleVersion::parse("2.0.0-alpha.1");
    assert(v2.major == 2);
    assert(v2.minor == 0);
    assert(v2.patch == 0);
    assert(v2.prerelease.has_value());
    assert(v2.prerelease.value() == "alpha.1");
    
    // Version with metadata
    auto v3 = ModuleVersion::parse("1.0.0+build.123");
    assert(v3.major == 1);
    assert(v3.minor == 0);
    assert(v3.patch == 0);
    assert(!v3.prerelease.has_value());
    assert(v3.metadata.has_value());
    assert(v3.metadata.value() == "build.123");
    
    // Full version
    auto v4 = ModuleVersion::parse("3.1.4-beta.2+exp.sha.5114f85");
    assert(v4.major == 3);
    assert(v4.minor == 1);
    assert(v4.patch == 4);
    assert(v4.prerelease.value() == "beta.2");
    assert(v4.metadata.value() == "exp.sha.5114f85");
    
    std::cout << "✓ Version parsing tests passed" << std::endl;
}

void test_version_comparison() {
    std::cout << "Testing version comparison..." << std::endl;
    
    auto v1_0_0 = ModuleVersion{1, 0, 0};
    auto v1_0_1 = ModuleVersion{1, 0, 1};
    auto v1_1_0 = ModuleVersion{1, 1, 0};
    auto v2_0_0 = ModuleVersion{2, 0, 0};
    
    // Basic comparisons
    assert(v1_0_0 < v1_0_1);
    assert(v1_0_1 < v1_1_0);
    assert(v1_1_0 < v2_0_0);
    assert(v2_0_0 > v1_0_0);
    
    // Equality
    auto v1_0_0_copy = ModuleVersion{1, 0, 0};
    assert(v1_0_0 == v1_0_0_copy);
    assert(!(v1_0_0 != v1_0_0_copy));
    
    // Pre-release versions
    auto v1_0_0_alpha = ModuleVersion{1, 0, 0};
    v1_0_0_alpha.prerelease = "alpha";
    assert(v1_0_0_alpha < v1_0_0);  // Pre-release is less than release
    
    std::cout << "✓ Version comparison tests passed" << std::endl;
}

void test_version_compatibility() {
    std::cout << "Testing version compatibility..." << std::endl;
    
    auto v1_0_0 = ModuleVersion{1, 0, 0};
    auto v1_1_0 = ModuleVersion{1, 1, 0};
    auto v1_2_3 = ModuleVersion{1, 2, 3};
    auto v2_0_0 = ModuleVersion{2, 0, 0};
    
    // Same major version compatibility
    assert(v1_1_0.is_compatible_with(v1_0_0));
    assert(v1_2_3.is_compatible_with(v1_0_0));
    assert(v1_2_3.is_compatible_with(v1_1_0));
    
    // Different major version
    assert(!v2_0_0.is_compatible_with(v1_0_0));
    assert(!v1_0_0.is_compatible_with(v2_0_0));
    
    // Breaking changes
    assert(v2_0_0.is_breaking_change_from(v1_0_0));
    assert(!v1_1_0.is_breaking_change_from(v1_0_0));
    
    std::cout << "✓ Version compatibility tests passed" << std::endl;
}

void test_module_metadata_serialization() {
    std::cout << "Testing metadata serialization..." << std::endl;
    
    ModuleMetadata metadata{
        .name = "TestModule",
        .version = ModuleVersion{1, 2, 3},
        .author = "Test Author",
        .description = "Test module description",
        .license = "MIT",
        .custom_metadata = {
            {"key1", "value1"},
            {"key2", "value2"}
        }
    };
    
    // Serialize
    auto serialized = metadata.to_bsatn();
    
    // Deserialize
    auto deserialized = ModuleMetadata::from_bsatn(serialized.data(), serialized.size());
    
    // Verify
    assert(deserialized.name == metadata.name);
    assert(deserialized.version == metadata.version);
    assert(deserialized.author == metadata.author);
    assert(deserialized.description == metadata.description);
    assert(deserialized.license == metadata.license);
    assert(deserialized.custom_metadata.size() == 2);
    assert(deserialized.custom_metadata.at("key1") == "value1");
    assert(deserialized.custom_metadata.at("key2") == "value2");
    
    std::cout << "✓ Metadata serialization tests passed" << std::endl;
}

void test_schema_change_compatibility() {
    std::cout << "Testing schema change compatibility..." << std::endl;
    
    // Backward compatible changes
    SchemaChange add_table{
        .type = SchemaChangeType::TABLE_ADDED,
        .table_name = "new_table"
    };
    assert(add_table.is_backward_compatible());
    
    SchemaChange add_column{
        .type = SchemaChangeType::COLUMN_ADDED,
        .table_name = "users",
        .object_name = "new_column"
    };
    assert(add_column.is_backward_compatible());
    
    // Breaking changes
    SchemaChange remove_table{
        .type = SchemaChangeType::TABLE_REMOVED,
        .table_name = "old_table"
    };
    assert(!remove_table.is_backward_compatible());
    
    SchemaChange change_type{
        .type = SchemaChangeType::COLUMN_TYPE_CHANGED,
        .table_name = "users",
        .object_name = "age",
        .old_value = "u32",
        .new_value = "u64"
    };
    assert(!change_type.is_backward_compatible());
    
    std::cout << "✓ Schema change compatibility tests passed" << std::endl;
}

void test_migration_plan() {
    std::cout << "Testing migration plans..." << std::endl;
    
    auto v1 = ModuleVersion{1, 0, 0};
    auto v2 = ModuleVersion{2, 0, 0};
    
    MigrationPlan plan(v1, v2);
    
    // Add some changes
    plan.add_change({
        .type = SchemaChangeType::TABLE_ADDED,
        .table_name = "profiles"
    });
    
    plan.add_change({
        .type = SchemaChangeType::COLUMN_ADDED,
        .table_name = "users",
        .object_name = "display_name"
    });
    
    // Check properties
    assert(plan.from_version() == v1);
    assert(plan.to_version() == v2);
    assert(plan.changes().size() == 2);
    
    // This plan has mixed compatibility
    assert(!plan.is_backward_compatible());  // Has backward compatible changes but major version bump
    
    std::cout << "✓ Migration plan tests passed" << std::endl;
}

void test_version_registry() {
    std::cout << "Testing version registry..." << std::endl;
    
    VersionRegistry registry;
    
    // Register some versions
    auto v1_0_0 = ModuleVersion{1, 0, 0};
    auto v1_1_0 = ModuleVersion{1, 1, 0};
    auto v2_0_0 = ModuleVersion{2, 0, 0};
    
    ModuleMetadata meta_v1{
        .name = "TestModule",
        .version = v1_0_0,
        .author = "Author",
        .description = "Version 1.0.0",
        .license = "MIT"
    };
    
    ModuleMetadata meta_v1_1{
        .name = "TestModule",
        .version = v1_1_0,
        .author = "Author",
        .description = "Version 1.1.0",
        .license = "MIT"
    };
    
    ModuleMetadata meta_v2{
        .name = "TestModule",
        .version = v2_0_0,
        .author = "Author",
        .description = "Version 2.0.0",
        .license = "MIT"
    };
    
    registry.register_version(v1_0_0, meta_v1);
    registry.register_version(v1_1_0, meta_v1_1);
    registry.register_version(v2_0_0, meta_v2);
    
    // Check registrations
    assert(registry.has_version(v1_0_0));
    assert(registry.has_version(v1_1_0));
    assert(registry.has_version(v2_0_0));
    assert(!registry.has_version(ModuleVersion{3, 0, 0}));
    
    // Get all versions
    auto versions = registry.get_versions();
    assert(versions.size() == 3);
    assert(versions[0] == v1_0_0);  // Should be sorted
    assert(versions[1] == v1_1_0);
    assert(versions[2] == v2_0_0);
    
    std::cout << "✓ Version registry tests passed" << std::endl;
}

void test_version_manager() {
    std::cout << "Testing version manager..." << std::endl;
    
    ModuleMetadata metadata{
        .name = "TestModule",
        .version = ModuleVersion{1, 0, 0},
        .author = "Test Author",
        .description = "Test module",
        .license = "MIT"
    };
    
    ModuleVersionManager manager(metadata);
    
    // Check current version
    assert(manager.current_version() == ModuleVersion{1, 0, 0});
    assert(manager.metadata().name == "TestModule");
    
    // Test upgrade checks
    assert(manager.can_upgrade_to(ModuleVersion{1, 1, 0}));  // Minor version
    assert(manager.can_upgrade_to(ModuleVersion{1, 0, 1}));  // Patch version
    
    // Major version upgrade depends on config
    VersioningConfig config{
        .allow_breaking_changes = false
    };
    ModuleVersionManager strict_manager(metadata, config);
    assert(!strict_manager.can_upgrade_to(ModuleVersion{2, 0, 0}));
    
    std::cout << "✓ Version manager tests passed" << std::endl;
}

void run_example_scenarios() {
    std::cout << "\nExample Scenarios:" << std::endl;
    
    // Scenario 1: Compatible update
    std::cout << "\n1. Minor version update (1.0.0 -> 1.1.0):" << std::endl;
    auto v1_0 = ModuleVersion::parse("1.0.0");
    auto v1_1 = ModuleVersion::parse("1.1.0");
    std::cout << "   " << v1_0.to_string() << " -> " << v1_1.to_string() << std::endl;
    std::cout << "   Compatible: " << (v1_1.is_compatible_with(v1_0) ? "Yes" : "No") << std::endl;
    std::cout << "   Breaking change: " << (v1_1.is_breaking_change_from(v1_0) ? "Yes" : "No") << std::endl;
    
    // Scenario 2: Breaking change
    std::cout << "\n2. Major version update (1.5.2 -> 2.0.0):" << std::endl;
    auto v1_5 = ModuleVersion::parse("1.5.2");
    auto v2_0 = ModuleVersion::parse("2.0.0");
    std::cout << "   " << v1_5.to_string() << " -> " << v2_0.to_string() << std::endl;
    std::cout << "   Compatible: " << (v2_0.is_compatible_with(v1_5) ? "Yes" : "No") << std::endl;
    std::cout << "   Breaking change: " << (v2_0.is_breaking_change_from(v1_5) ? "Yes" : "No") << std::endl;
    
    // Scenario 3: Pre-release version
    std::cout << "\n3. Pre-release version handling:" << std::endl;
    auto v2_beta = ModuleVersion::parse("2.0.0-beta.1");
    auto v2_release = ModuleVersion::parse("2.0.0");
    std::cout << "   " << v2_beta.to_string() << " vs " << v2_release.to_string() << std::endl;
    std::cout << "   Beta < Release: " << (v2_beta < v2_release ? "Yes" : "No") << std::endl;
    
    // Scenario 4: Migration plan
    std::cout << "\n4. Migration plan example:" << std::endl;
    MigrationPlan plan(v1_5, v2_0);
    plan.add_change({SchemaChangeType::TABLE_ADDED, "user_profiles", "", {}, {}});
    plan.add_change({SchemaChangeType::COLUMN_REMOVED, "users", "legacy_field", {}, {}});
    std::cout << plan.summary() << std::endl;
}

int main() {
    std::cout << "Running SpacetimeDB C++ SDK Versioning Tests\n" << std::endl;
    
    try {
        test_version_parsing();
        test_version_comparison();
        test_version_compatibility();
        test_module_metadata_serialization();
        test_schema_change_compatibility();
        test_migration_plan();
        test_version_registry();
        test_version_manager();
        
        std::cout << "\n✅ All tests passed!" << std::endl;
        
        run_example_scenarios();
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}