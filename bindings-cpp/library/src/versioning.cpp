#include <spacetimedb/versioning.h>
#include <sstream>
#include <algorithm>
#include <regex>
#include <queue>
#include <set>

namespace SpacetimeDb {

// ModuleVersion implementation
ModuleVersion ModuleVersion::parse(const std::string& version_string) {
    static const std::regex version_regex(
        R"(^(\d+)\.(\d+)\.(\d+)(?:-([0-9A-Za-z-]+(?:\.[0-9A-Za-z-]+)*))?(?:\+([0-9A-Za-z-]+(?:\.[0-9A-Za-z-]+)*))?$)"
    );
    
    std::smatch match;
    if (!std::regex_match(version_string, match, version_regex)) {
        throw std::runtime_error("Invalid version string: " + version_string);
    }
    
    ModuleVersion version;
    version.major = std::stoi(match[1]);
    version.minor = std::stoi(match[2]);
    version.patch = std::stoi(match[3]);
    
    if (match[4].matched) {
        version.prerelease = match[4].str();
    }
    
    if (match[5].matched) {
        version.metadata = match[5].str();
    }
    
    return version;
}

std::string ModuleVersion::to_string() const {
    std::stringstream ss;
    ss << major << "." << minor << "." << patch;
    
    if (prerelease.has_value()) {
        ss << "-" << prerelease.value();
    }
    
    if (metadata.has_value()) {
        ss << "+" << metadata.value();
    }
    
    return ss.str();
}

bool ModuleVersion::operator==(const ModuleVersion& other) const {
    return major == other.major && 
           minor == other.minor && 
           patch == other.patch &&
           prerelease == other.prerelease;
    // Note: metadata is ignored for version comparison per semver spec
}

bool ModuleVersion::operator!=(const ModuleVersion& other) const {
    return !(*this == other);
}

bool ModuleVersion::operator<(const ModuleVersion& other) const {
    if (major != other.major) return major < other.major;
    if (minor != other.minor) return minor < other.minor;
    if (patch != other.patch) return patch < other.patch;
    
    // Handle pre-release versions
    if (!prerelease.has_value() && other.prerelease.has_value()) {
        return false;  // Release version is greater than pre-release
    }
    if (prerelease.has_value() && !other.prerelease.has_value()) {
        return true;   // Pre-release is less than release
    }
    if (prerelease.has_value() && other.prerelease.has_value()) {
        return prerelease.value() < other.prerelease.value();
    }
    
    return false;
}

bool ModuleVersion::operator>(const ModuleVersion& other) const {
    return other < *this;
}

bool ModuleVersion::operator<=(const ModuleVersion& other) const {
    return !(*this > other);
}

bool ModuleVersion::operator>=(const ModuleVersion& other) const {
    return !(*this < other);
}

bool ModuleVersion::is_compatible_with(const ModuleVersion& other) const {
    // Following semver: compatible if same major version and this >= other
    return major == other.major && *this >= other;
}

bool ModuleVersion::is_breaking_change_from(const ModuleVersion& other) const {
    return major != other.major;
}

// ModuleMetadata implementation
std::vector<uint8_t> ModuleMetadata::to_bsatn() const {
    std::vector<uint8_t> buffer;
    
    // Helper to write string
    auto write_string = [&buffer](const std::string& s) {
        uint32_t len = s.length();
        buffer.push_back(len & 0xFF);
        buffer.push_back((len >> 8) & 0xFF);
        buffer.push_back((len >> 16) & 0xFF);
        buffer.push_back((len >> 24) & 0xFF);
        buffer.insert(buffer.end(), s.begin(), s.end());
    };
    
    // Write fields
    write_string(name);
    
    // Write version
    buffer.push_back(version.major & 0xFF);
    buffer.push_back((version.major >> 8) & 0xFF);
    buffer.push_back(version.minor & 0xFF);
    buffer.push_back((version.minor >> 8) & 0xFF);
    buffer.push_back(version.patch & 0xFF);
    buffer.push_back((version.patch >> 8) & 0xFF);
    
    // Write optional prerelease
    if (version.prerelease.has_value()) {
        buffer.push_back(0);  // Some
        write_string(version.prerelease.value());
    } else {
        buffer.push_back(1);  // None
    }
    
    // Write optional metadata
    if (version.metadata.has_value()) {
        buffer.push_back(0);  // Some
        write_string(version.metadata.value());
    } else {
        buffer.push_back(1);  // None
    }
    
    write_string(author);
    write_string(description);
    write_string(license);
    
    // Write custom metadata map
    uint32_t map_size = custom_metadata.size();
    buffer.push_back(map_size & 0xFF);
    buffer.push_back((map_size >> 8) & 0xFF);
    buffer.push_back((map_size >> 16) & 0xFF);
    buffer.push_back((map_size >> 24) & 0xFF);
    
    for (const auto& [key, value] : custom_metadata) {
        write_string(key);
        write_string(value);
    }
    
    return buffer;
}

ModuleMetadata ModuleMetadata::from_bsatn(const uint8_t* data, size_t len) {
    ModuleMetadata metadata;
    size_t offset = 0;
    
    // Helper to read string
    auto read_string = [&data, &offset, len]() -> std::string {
        if (offset + 4 > len) throw std::runtime_error("Buffer underflow");
        
        uint32_t str_len = data[offset] | 
                          (data[offset + 1] << 8) |
                          (data[offset + 2] << 16) |
                          (data[offset + 3] << 24);
        offset += 4;
        
        if (offset + str_len > len) throw std::runtime_error("Buffer underflow");
        
        std::string result(reinterpret_cast<const char*>(data + offset), str_len);
        offset += str_len;
        return result;
    };
    
    // Read fields
    metadata.name = read_string();
    
    // Read version
    if (offset + 6 > len) throw std::runtime_error("Buffer underflow");
    metadata.version.major = data[offset] | (data[offset + 1] << 8);
    offset += 2;
    metadata.version.minor = data[offset] | (data[offset + 1] << 8);
    offset += 2;
    metadata.version.patch = data[offset] | (data[offset + 1] << 8);
    offset += 2;
    
    // Read optional prerelease
    if (offset >= len) throw std::runtime_error("Buffer underflow");
    if (data[offset++] == 0) {  // Some
        metadata.version.prerelease = read_string();
    }
    
    // Read optional metadata
    if (offset >= len) throw std::runtime_error("Buffer underflow");
    if (data[offset++] == 0) {  // Some
        metadata.version.metadata = read_string();
    }
    
    metadata.author = read_string();
    metadata.description = read_string();
    metadata.license = read_string();
    
    // Read custom metadata map
    if (offset + 4 > len) throw std::runtime_error("Buffer underflow");
    uint32_t map_size = data[offset] | 
                        (data[offset + 1] << 8) |
                        (data[offset + 2] << 16) |
                        (data[offset + 3] << 24);
    offset += 4;
    
    for (uint32_t i = 0; i < map_size; i++) {
        std::string key = read_string();
        std::string value = read_string();
        metadata.custom_metadata[key] = value;
    }
    
    return metadata;
}

// SchemaChange implementation
bool SchemaChange::is_backward_compatible() const {
    switch (type) {
        case SchemaChangeType::TABLE_ADDED:
        case SchemaChangeType::COLUMN_ADDED:
        case SchemaChangeType::INDEX_ADDED:
        case SchemaChangeType::REDUCER_ADDED:
            return true;
            
        case SchemaChangeType::TABLE_REMOVED:
        case SchemaChangeType::COLUMN_REMOVED:
        case SchemaChangeType::COLUMN_TYPE_CHANGED:
        case SchemaChangeType::REDUCER_REMOVED:
        case SchemaChangeType::REDUCER_SIGNATURE_CHANGED:
            return false;
            
        case SchemaChangeType::TABLE_RENAMED:
        case SchemaChangeType::COLUMN_RENAMED:
        case SchemaChangeType::INDEX_REMOVED:
        case SchemaChangeType::INDEX_MODIFIED:
        case SchemaChangeType::CONSTRAINT_ADDED:
        case SchemaChangeType::CONSTRAINT_REMOVED:
        case SchemaChangeType::SEQUENCE_ADDED:
        case SchemaChangeType::SEQUENCE_REMOVED:
            return true;  // These can be handled with care
            
        default:
            return false;
    }
}

// MigrationPlan implementation
void MigrationPlan::add_step(std::unique_ptr<MigrationStep> step) {
    steps_.push_back(std::move(step));
}

void MigrationPlan::add_change(const SchemaChange& change) {
    changes_.push_back(change);
}

bool MigrationPlan::is_automatic() const {
    return std::all_of(steps_.begin(), steps_.end(),
                      [](const auto& step) { return step->is_automatic(); });
}

bool MigrationPlan::is_backward_compatible() const {
    return std::all_of(changes_.begin(), changes_.end(),
                      [](const SchemaChange& change) { return change.is_backward_compatible(); });
}

bool MigrationPlan::execute() {
    for (auto& step : steps_) {
        if (!step->execute()) {
            // Rollback on failure
            for (auto it = steps_.rbegin(); it != steps_.rend(); ++it) {
                if (it->get() == step.get()) break;
                (*it)->rollback();
            }
            return false;
        }
    }
    return true;
}

std::string MigrationPlan::summary() const {
    std::stringstream ss;
    ss << "Migration from " << from_version_.to_string() 
       << " to " << to_version_.to_string() << "\n";
    ss << "Changes:\n";
    
    for (const auto& change : changes_) {
        ss << "  - ";
        switch (change.type) {
            case SchemaChangeType::TABLE_ADDED:
                ss << "Add table: " << change.table_name;
                break;
            case SchemaChangeType::TABLE_REMOVED:
                ss << "Remove table: " << change.table_name;
                break;
            case SchemaChangeType::COLUMN_ADDED:
                ss << "Add column: " << change.table_name << "." << change.object_name;
                break;
            case SchemaChangeType::COLUMN_REMOVED:
                ss << "Remove column: " << change.table_name << "." << change.object_name;
                break;
            case SchemaChangeType::COLUMN_TYPE_CHANGED:
                ss << "Change column type: " << change.table_name << "." << change.object_name
                   << " from " << change.old_value.value_or("?") 
                   << " to " << change.new_value.value_or("?");
                break;
            default:
                ss << "Other change";
        }
        ss << "\n";
    }
    
    return ss.str();
}

// VersionRegistry implementation
void VersionRegistry::register_version(const ModuleVersion& version, 
                                     const ModuleMetadata& metadata) {
    versions_[version] = metadata;
}

void VersionRegistry::register_migration(const ModuleVersion& from, 
                                       const ModuleVersion& to,
                                       MigrationPlan plan) {
    migrations_.emplace(std::make_pair(from, to), std::move(plan));
}

std::optional<std::vector<const MigrationPlan*>> VersionRegistry::find_migration_path(
    const ModuleVersion& from, const ModuleVersion& to) const {
    
    if (from == to) {
        return std::vector<const MigrationPlan*>{};
    }
    
    // BFS to find shortest migration path
    std::queue<std::pair<ModuleVersion, std::vector<const MigrationPlan*>>> queue;
    std::set<ModuleVersion> visited;
    
    queue.push({from, {}});
    visited.insert(from);
    
    while (!queue.empty()) {
        auto [current_version, path] = queue.front();
        queue.pop();
        
        // Check all possible migrations from current version
        for (const auto& [key, plan] : migrations_) {
            if (key.first == current_version && visited.find(key.second) == visited.end()) {
                auto new_path = path;
                new_path.push_back(&plan);
                
                if (key.second == to) {
                    return new_path;
                }
                
                queue.push({key.second, new_path});
                visited.insert(key.second);
            }
        }
    }
    
    return std::nullopt;
}

std::vector<ModuleVersion> VersionRegistry::get_versions() const {
    std::vector<ModuleVersion> result;
    for (const auto& [version, _] : versions_) {
        result.push_back(version);
    }
    std::sort(result.begin(), result.end());
    return result;
}

bool VersionRegistry::has_version(const ModuleVersion& version) const {
    return versions_.find(version) != versions_.end();
}

// ModuleVersionManager implementation
void ModuleVersionManager::register_version(const ModuleVersion& version,
                                          const ModuleMetadata& metadata) {
    registry_.register_version(version, metadata);
}

std::optional<MigrationPlan> ModuleVersionManager::plan_migration(
    const ModuleVersion& to_version) {
    
    auto path = registry_.find_migration_path(current_version(), to_version);
    if (!path.has_value() || path->empty()) {
        return std::nullopt;
    }
    
    // Combine all migration steps into a single plan
    MigrationPlan combined_plan(current_version(), to_version);
    
    for (auto* plan : path.value()) {
        for (const auto& change : plan->changes()) {
            combined_plan.add_change(change);
        }
    }
    
    return combined_plan;
}

bool ModuleVersionManager::can_upgrade_to(const ModuleVersion& version) const {
    if (!config_.allow_breaking_changes && version.is_breaking_change_from(current_version())) {
        return false;
    }
    
    if (config_.require_migration_for_major && version.major != current_version().major) {
        return registry_.find_migration_path(current_version(), version).has_value();
    }
    
    return true;
}

std::vector<std::pair<ModuleVersion, ModuleVersion>> 
ModuleVersionManager::get_migration_history() const {
    // This would typically query the migration history from the database
    // For now, return empty
    return {};
}

// Helper function implementation
void add_version_to_module_def(void* module_def_builder, const ModuleMetadata& metadata) {
    // This would add version metadata to the module definition
    // Implementation depends on the module definition structure
    // For now, this is a placeholder
}

} // namespace SpacetimeDb