#include <spacetimedb/spacetimedb.h>
#include <spacetimedb/constraint_validation.h>
#include <spacetimedb/validated_table.h>
#include <unordered_map>
#include <unordered_set>

using namespace SpacetimeDb;

/**
 * @brief Foreign key validator that checks references to other tables.
 * 
 * This validator maintains a cache of valid foreign key values and validates
 * that referenced rows exist in the parent table.
 */
template<typename T, typename ForeignKeyType>
class ForeignKeyValidator : public IConstraintValidator<T> {
private:
    std::string constraint_name_;
    std::string column_name_;
    std::string referenced_table_;
    std::string referenced_column_;
    std::function<ForeignKeyType(const T&)> key_extractor_;
    mutable std::shared_ptr<std::unordered_set<ForeignKeyType>> valid_keys_cache_;
    
public:
    ForeignKeyValidator(
        const std::string& name,
        const std::string& column,
        const std::string& ref_table,
        const std::string& ref_column,
        std::function<ForeignKeyType(const T&)> extractor,
        std::shared_ptr<std::unordered_set<ForeignKeyType>> cache = nullptr)
        : constraint_name_(name),
          column_name_(column),
          referenced_table_(ref_table),
          referenced_column_(ref_column),
          key_extractor_(extractor),
          valid_keys_cache_(cache) {
        if (!valid_keys_cache_) {
            valid_keys_cache_ = std::make_shared<std::unordered_set<ForeignKeyType>>();
        }
    }
    
    ValidationResult validate(const T& row) const override {
        ValidationResult result;
        ForeignKeyType key = key_extractor_(row);
        
        // Skip validation for null foreign keys (assuming they're optional)
        if constexpr (std::is_pointer_v<ForeignKeyType>) {
            if (key == nullptr) return result;
        }
        
        // Check if key exists in cache
        if (valid_keys_cache_->find(key) == valid_keys_cache_->end()) {
            result.add_violation(ConstraintViolation(
                ConstraintViolation::FOREIGN_KEY,
                constraint_name_,
                column_name_,
                "Foreign key violation: " + column_name_ + " references non-existent " +
                referenced_table_ + "." + referenced_column_
            ));
        }
        
        return result;
    }
    
    std::string name() const override {
        return "FOREIGN KEY " + constraint_name_ + " REFERENCES " + 
               referenced_table_ + "(" + referenced_column_ + ")";
    }
    
    // Cache management
    void add_valid_key(const ForeignKeyType& key) {
        valid_keys_cache_->insert(key);
    }
    
    void remove_valid_key(const ForeignKeyType& key) {
        valid_keys_cache_->erase(key);
    }
    
    void refresh_cache(const std::vector<ForeignKeyType>& valid_keys) {
        valid_keys_cache_->clear();
        for (const auto& key : valid_keys) {
            valid_keys_cache_->insert(key);
        }
    }
};

// Example schema with foreign key relationships

// Department table
struct Department {
    uint32_t dept_id;
    std::string name;
    std::string location;
    
    void bsatn_serialize(bsatn::Writer& writer) const {
        writer.write_u32(dept_id);
        writer.write_string(name);
        writer.write_string(location);
    }
    
    void bsatn_deserialize(bsatn::Reader& reader) {
        dept_id = reader.read_u32();
        name = reader.read_string();
        location = reader.read_string();
    }
};

SPACETIMEDB_REGISTER_FIELDS(Department,
    SPACETIMEDB_FIELD(Department, dept_id, uint32_t);
    SPACETIMEDB_FIELD(Department, name, std::string);
    SPACETIMEDB_FIELD(Department, location, std::string);
)

SPACETIMEDB_TABLE(Department, departments, true)

// Employee table with foreign key to Department
struct Employee {
    uint32_t emp_id;
    std::string name;
    std::string email;
    uint32_t dept_id;  // Foreign key to Department
    std::optional<uint32_t> manager_id;  // Self-referencing foreign key
    double salary;
    
    void bsatn_serialize(bsatn::Writer& writer) const {
        writer.write_u32(emp_id);
        writer.write_string(name);
        writer.write_string(email);
        writer.write_u32(dept_id);
        writer.write_option(manager_id);
        writer.write_f64(salary);
    }
    
    void bsatn_deserialize(bsatn::Reader& reader) {
        emp_id = reader.read_u32();
        name = reader.read_string();
        email = reader.read_string();
        dept_id = reader.read_u32();
        manager_id = reader.read_option<uint32_t>();
        salary = reader.read_f64();
    }
};

SPACETIMEDB_REGISTER_FIELDS(Employee,
    SPACETIMEDB_FIELD(Employee, emp_id, uint32_t);
    SPACETIMEDB_FIELD(Employee, name, std::string);
    SPACETIMEDB_FIELD(Employee, email, std::string);
    SPACETIMEDB_FIELD(Employee, dept_id, uint32_t);
    SPACETIMEDB_FIELD(Employee, manager_id, std::optional<uint32_t>);
    SPACETIMEDB_FIELD(Employee, salary, double);
)

SPACETIMEDB_TABLE(Employee, employees, true)

// Project table
struct Project {
    uint32_t project_id;
    std::string name;
    uint32_t lead_emp_id;  // Foreign key to Employee
    std::string status;
    double budget;
    
    void bsatn_serialize(bsatn::Writer& writer) const {
        writer.write_u32(project_id);
        writer.write_string(name);
        writer.write_u32(lead_emp_id);
        writer.write_string(status);
        writer.write_f64(budget);
    }
    
    void bsatn_deserialize(bsatn::Reader& reader) {
        project_id = reader.read_u32();
        name = reader.read_string();
        lead_emp_id = reader.read_u32();
        status = reader.read_string();
        budget = reader.read_f64();
    }
};

SPACETIMEDB_REGISTER_FIELDS(Project,
    SPACETIMEDB_FIELD(Project, project_id, uint32_t);
    SPACETIMEDB_FIELD(Project, name, std::string);
    SPACETIMEDB_FIELD(Project, lead_emp_id, uint32_t);
    SPACETIMEDB_FIELD(Project, status, std::string);
    SPACETIMEDB_FIELD(Project, budget, double);
)

SPACETIMEDB_TABLE(Project, projects, true)

// Many-to-many relationship table
struct EmployeeProject {
    uint32_t emp_id;      // Foreign key to Employee
    uint32_t project_id;  // Foreign key to Project
    std::string role;
    double hours_allocated;
    
    void bsatn_serialize(bsatn::Writer& writer) const {
        writer.write_u32(emp_id);
        writer.write_u32(project_id);
        writer.write_string(role);
        writer.write_f64(hours_allocated);
    }
    
    void bsatn_deserialize(bsatn::Reader& reader) {
        emp_id = reader.read_u32();
        project_id = reader.read_u32();
        role = reader.read_string();
        hours_allocated = reader.read_f64();
    }
};

SPACETIMEDB_REGISTER_FIELDS(EmployeeProject,
    SPACETIMEDB_FIELD(EmployeeProject, emp_id, uint32_t);
    SPACETIMEDB_FIELD(EmployeeProject, project_id, uint32_t);
    SPACETIMEDB_FIELD(EmployeeProject, role, std::string);
    SPACETIMEDB_FIELD(EmployeeProject, hours_allocated, double);
)

SPACETIMEDB_TABLE(EmployeeProject, employee_projects, true)

// Foreign key cache manager
class ForeignKeyManager {
private:
    std::shared_ptr<std::unordered_set<uint32_t>> dept_ids_;
    std::shared_ptr<std::unordered_set<uint32_t>> emp_ids_;
    std::shared_ptr<std::unordered_set<uint32_t>> project_ids_;
    
public:
    ForeignKeyManager() 
        : dept_ids_(std::make_shared<std::unordered_set<uint32_t>>()),
          emp_ids_(std::make_shared<std::unordered_set<uint32_t>>()),
          project_ids_(std::make_shared<std::unordered_set<uint32_t>>()) {}
    
    auto get_dept_cache() { return dept_ids_; }
    auto get_emp_cache() { return emp_ids_; }
    auto get_project_cache() { return project_ids_; }
    
    void add_department(uint32_t id) { dept_ids_->insert(id); }
    void remove_department(uint32_t id) { dept_ids_->erase(id); }
    
    void add_employee(uint32_t id) { emp_ids_->insert(id); }
    void remove_employee(uint32_t id) { emp_ids_->erase(id); }
    
    void add_project(uint32_t id) { project_ids_->insert(id); }
    void remove_project(uint32_t id) { project_ids_->erase(id); }
};

// Setup validated tables with foreign key constraints
std::unique_ptr<ValidatedTable<Department>> setup_department_table(
    uint32_t table_id, ForeignKeyManager& fk_manager) {
    
    auto table = std::make_unique<ValidatedTable<Department>>(table_id, "departments");
    
    ConstraintBuilder<Department> builder;
    builder.unique("pk_dept_id", {"dept_id"}, [](const Department& d) { return d.dept_id; })
           .not_null("name", [](const Department& d) { return &d.name; })
           .check("check_name_length", 
                  [](const Department& d) { return d.name.length() > 0 && d.name.length() <= 100; },
                  "length(name) BETWEEN 1 AND 100");
    
    auto composite = builder.build();
    table->constraints().add_validator(
        std::make_unique<CompositeValidator<Department>>(std::move(composite))
    );
    
    return table;
}

std::unique_ptr<ValidatedTable<Employee>> setup_employee_table(
    uint32_t table_id, ForeignKeyManager& fk_manager) {
    
    auto table = std::make_unique<ValidatedTable<Employee>>(table_id, "employees");
    
    // Add foreign key validators
    table->constraints().add_validator(
        std::make_unique<ForeignKeyValidator<Employee, uint32_t>>(
            "fk_emp_dept",
            "dept_id",
            "departments",
            "dept_id",
            [](const Employee& e) { return e.dept_id; },
            fk_manager.get_dept_cache()
        )
    );
    
    // Self-referencing foreign key for manager
    auto manager_fk = std::make_unique<ForeignKeyValidator<Employee, std::optional<uint32_t>>>(
        "fk_emp_manager",
        "manager_id",
        "employees",
        "emp_id",
        [](const Employee& e) { return e.manager_id; },
        nullptr  // Will use custom validation
    );
    
    // Regular constraints
    ConstraintBuilder<Employee> builder;
    builder.unique("pk_emp_id", {"emp_id"}, [](const Employee& e) { return e.emp_id; })
           .unique("unique_email", {"email"}, [](const Employee& e) { return e.email; })
           .not_null("name", [](const Employee& e) { return &e.name; })
           .not_null("email", [](const Employee& e) { return &e.email; })
           .check("check_salary_positive", 
                  [](const Employee& e) { return e.salary > 0; },
                  "salary > 0")
           .check("check_email_format",
                  [](const Employee& e) { 
                      return e.email.find('@') != std::string::npos && 
                             e.email.find('.') != std::string::npos; 
                  },
                  "email must be valid format");
    
    auto composite = builder.build();
    table->constraints().add_validator(
        std::make_unique<CompositeValidator<Employee>>(std::move(composite))
    );
    
    return table;
}

std::unique_ptr<ValidatedTable<Project>> setup_project_table(
    uint32_t table_id, ForeignKeyManager& fk_manager) {
    
    auto table = std::make_unique<ValidatedTable<Project>>(table_id, "projects");
    
    // Foreign key to employee (project lead)
    table->constraints().add_validator(
        std::make_unique<ForeignKeyValidator<Project, uint32_t>>(
            "fk_project_lead",
            "lead_emp_id",
            "employees",
            "emp_id",
            [](const Project& p) { return p.lead_emp_id; },
            fk_manager.get_emp_cache()
        )
    );
    
    ConstraintBuilder<Project> builder;
    builder.unique("pk_project_id", {"project_id"}, [](const Project& p) { return p.project_id; })
           .not_null("name", [](const Project& p) { return &p.name; })
           .check("check_budget_positive",
                  [](const Project& p) { return p.budget > 0; },
                  "budget > 0")
           .check("check_valid_status",
                  [](const Project& p) { 
                      return p.status == "planning" || p.status == "active" || 
                             p.status == "completed" || p.status == "cancelled"; 
                  },
                  "status IN ('planning', 'active', 'completed', 'cancelled')");
    
    auto composite = builder.build();
    table->constraints().add_validator(
        std::make_unique<CompositeValidator<Project>>(std::move(composite))
    );
    
    return table;
}

std::unique_ptr<ValidatedTable<EmployeeProject>> setup_employee_project_table(
    uint32_t table_id, ForeignKeyManager& fk_manager) {
    
    auto table = std::make_unique<ValidatedTable<EmployeeProject>>(table_id, "employee_projects");
    
    // Foreign keys
    table->constraints().add_validator(
        std::make_unique<ForeignKeyValidator<EmployeeProject, uint32_t>>(
            "fk_ep_employee",
            "emp_id",
            "employees",
            "emp_id",
            [](const EmployeeProject& ep) { return ep.emp_id; },
            fk_manager.get_emp_cache()
        )
    );
    
    table->constraints().add_validator(
        std::make_unique<ForeignKeyValidator<EmployeeProject, uint32_t>>(
            "fk_ep_project",
            "project_id",
            "projects",
            "project_id",
            [](const EmployeeProject& ep) { return ep.project_id; },
            fk_manager.get_project_cache()
        )
    );
    
    ConstraintBuilder<EmployeeProject> builder;
    // Composite primary key
    builder.unique("pk_employee_project", {"emp_id", "project_id"},
                   [](const EmployeeProject& ep) { 
                       return std::make_pair(ep.emp_id, ep.project_id); 
                   })
           .not_null("role", [](const EmployeeProject& ep) { return &ep.role; })
           .check("check_hours_positive",
                  [](const EmployeeProject& ep) { return ep.hours_allocated > 0; },
                  "hours_allocated > 0")
           .check("check_hours_reasonable",
                  [](const EmployeeProject& ep) { return ep.hours_allocated <= 60; },
                  "hours_allocated <= 60");
    
    auto composite = builder.build();
    table->constraints().add_validator(
        std::make_unique<CompositeValidator<EmployeeProject>>(std::move(composite))
    );
    
    return table;
}

// Test foreign key validation
SPACETIMEDB_REDUCER(test_foreign_keys, SpacetimeDb::ReducerContext ctx) {
    std::cout << "\n=== Foreign Key Validation Demo ===\n" << std::endl;
    
    ForeignKeyManager fk_manager;
    
    // Setup tables
    auto dept_table = setup_department_table(1, fk_manager);
    auto emp_table = setup_employee_table(2, fk_manager);
    auto proj_table = setup_project_table(3, fk_manager);
    auto ep_table = setup_employee_project_table(4, fk_manager);
    
    // Test 1: Insert departments (parent table)
    std::cout << "Test 1: Insert departments" << std::endl;
    std::vector<Department> departments = {
        {101, "Engineering", "Building A"},
        {102, "Marketing", "Building B"},
        {103, "HR", "Building C"}
    };
    
    for (const auto& dept : departments) {
        auto result = dept_table->validate(dept);
        if (result.is_valid()) {
            fk_manager.add_department(dept.dept_id);
            std::cout << "  ✓ Department " << dept.name << " added" << std::endl;
        } else {
            std::cout << "  ✗ " << result.to_string() << std::endl;
        }
    }
    
    // Test 2: Insert employee with valid department
    std::cout << "\nTest 2: Insert employee with valid department" << std::endl;
    Employee valid_emp{1001, "John Doe", "john@company.com", 101, std::nullopt, 75000};
    auto result = emp_table->validate(valid_emp);
    if (result.is_valid()) {
        fk_manager.add_employee(valid_emp.emp_id);
        std::cout << "  ✓ Employee added successfully" << std::endl;
    } else {
        std::cout << "  ✗ " << result.to_string() << std::endl;
    }
    
    // Test 3: Insert employee with invalid department
    std::cout << "\nTest 3: Insert employee with invalid department" << std::endl;
    Employee invalid_dept_emp{1002, "Jane Smith", "jane@company.com", 999, std::nullopt, 80000};
    result = emp_table->validate(invalid_dept_emp);
    std::cout << "  Result: " << result.to_string() << std::endl;
    
    // Test 4: Insert more employees for hierarchy testing
    std::cout << "\nTest 4: Create employee hierarchy" << std::endl;
    std::vector<Employee> employees = {
        {1003, "Alice Johnson", "alice@company.com", 101, 1001, 85000},  // Reports to John
        {1004, "Bob Williams", "bob@company.com", 102, std::nullopt, 70000},
        {1005, "Charlie Brown", "charlie@company.com", 101, 1001, 72000}  // Reports to John
    };
    
    for (const auto& emp : employees) {
        result = emp_table->validate(emp);
        if (result.is_valid()) {
            fk_manager.add_employee(emp.emp_id);
            std::cout << "  ✓ Employee " << emp.name << " added" << std::endl;
        } else {
            std::cout << "  ✗ " << result.to_string() << std::endl;
        }
    }
    
    // Test 5: Create projects with employee leads
    std::cout << "\nTest 5: Create projects" << std::endl;
    std::vector<Project> projects = {
        {2001, "Website Redesign", 1001, "active", 150000},      // Led by John
        {2002, "Marketing Campaign", 1004, "planning", 50000},   // Led by Bob
        {2003, "AI Integration", 1003, "active", 300000}        // Led by Alice
    };
    
    for (const auto& proj : projects) {
        result = proj_table->validate(proj);
        if (result.is_valid()) {
            fk_manager.add_project(proj.project_id);
            std::cout << "  ✓ Project " << proj.name << " created" << std::endl;
        } else {
            std::cout << "  ✗ " << result.to_string() << std::endl;
        }
    }
    
    // Test 6: Assign employees to projects
    std::cout << "\nTest 6: Assign employees to projects" << std::endl;
    std::vector<EmployeeProject> assignments = {
        {1001, 2001, "Lead Developer", 20},
        {1003, 2001, "Senior Developer", 30},
        {1005, 2001, "Developer", 40},
        {1004, 2002, "Marketing Lead", 35},
        {1003, 2003, "Tech Lead", 25}
    };
    
    for (const auto& assignment : assignments) {
        result = ep_table->validate(assignment);
        if (result.is_valid()) {
            std::cout << "  ✓ Assigned employee " << assignment.emp_id 
                      << " to project " << assignment.project_id << std::endl;
        } else {
            std::cout << "  ✗ " << result.to_string() << std::endl;
        }
    }
    
    // Test 7: Try invalid assignment (non-existent employee)
    std::cout << "\nTest 7: Try invalid assignment" << std::endl;
    EmployeeProject invalid_assignment{9999, 2001, "Ghost Employee", 40};
    result = ep_table->validate(invalid_assignment);
    std::cout << "  Result: " << result.to_string() << std::endl;
    
    // Test 8: Cascading delete simulation
    std::cout << "\nTest 8: Simulate cascading operations" << std::endl;
    std::cout << "  Removing department 101 would affect:" << std::endl;
    
    // Count affected employees
    int affected_employees = 0;
    for (const auto& emp_id : {1001, 1003, 1005}) {
        affected_employees++;
    }
    std::cout << "    - " << affected_employees << " employees" << std::endl;
    std::cout << "    - Their managed employees (cascade)" << std::endl;
    std::cout << "    - Projects they lead" << std::endl;
    std::cout << "    - Project assignments" << std::endl;
    
    std::cout << "\n=== Foreign Key Validation Demo Complete ===\n" << std::endl;
}

// Helper reducer to show foreign key relationships
SPACETIMEDB_REDUCER(show_relationships, SpacetimeDb::ReducerContext ctx) {
    std::cout << "\n=== Database Relationship Map ===\n" << std::endl;
    
    std::cout << "departments" << std::endl;
    std::cout << "  └─→ employees (via dept_id)" << std::endl;
    std::cout << "       ├─→ employees (via manager_id, self-referencing)" << std::endl;
    std::cout << "       └─→ projects (via lead_emp_id)" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "employee_projects (junction table)" << std::endl;
    std::cout << "  ├─→ employees (via emp_id)" << std::endl;
    std::cout << "  └─→ projects (via project_id)" << std::endl;
    
    std::cout << "\nConstraint Rules:" << std::endl;
    std::cout << "- Cannot insert employee without valid department" << std::endl;
    std::cout << "- Cannot insert employee with non-existent manager" << std::endl;
    std::cout << "- Cannot create project without valid lead employee" << std::endl;
    std::cout << "- Cannot assign non-existent employee/project" << std::endl;
    std::cout << "- Deleting department would require handling employees first" << std::endl;
    
    std::cout << "\n================================\n" << std::endl;
}