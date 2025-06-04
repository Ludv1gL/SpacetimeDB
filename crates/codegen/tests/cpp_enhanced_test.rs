//! Test the enhanced C++ code generation with C# attribute equivalent macros

use spacetimedb_codegen::{cpp_enhanced, generate};
use spacetimedb_lib::{RawModuleDef, RawModuleDefV8};
use spacetimedb_schema::def::ModuleDef;

#[test]
fn test_cpp_enhanced_codegen() -> anyhow::Result<()> {
    // Create a test module using RawModuleDef like the original example
    let module = RawModuleDefV8::with_builder(|module| {
        module.add_type::<RawModuleDef>();
    });
    
    let module: ModuleDef = module.try_into()?;
    
    // Generate C++ code
    let files = generate(
        &module,
        &cpp_enhanced::CppEnhanced {
            namespace: "SpacetimeDb::Internal",
        },
    );
    
    // Verify that files were generated
    assert!(!files.is_empty(), "Should generate at least one file");
    
    // Check that all generated files contain the expected C# equivalent macros
    for (filename, content) in &files {
        println!("Checking file: {}", filename);
        
        // All generated files should include macro header
        assert!(
            content.contains("#include \"spacetimedb/macros.h\""),
            "File {} should include macros.h",
            filename
        );
        
        // All generated files should have type registration
        assert!(
            content.contains("SPACETIMEDB_TYPE(") || content.contains("SPACETIMEDB_REGISTER_TYPE("),
            "File {} should contain type registration macro",
            filename
        );
        
        // Files with struct definitions should have data contract
        if content.contains("struct ") {
            assert!(
                content.contains("SPACETIMEDB_DATA_CONTRACT"),
                "File {} with struct should contain data contract macro",
                filename
            );
        }
        
        // Files with fields should have data member annotations
        if content.contains("SPACETIMEDB_DATA_MEMBER(") {
            // Count the number of field definitions vs data member macros
            let field_lines: Vec<&str> = content.lines()
                .filter(|line| line.trim().ends_with(";") && !line.contains("//") && !line.contains("default") && !line.contains("return"))
                .collect();
            
            let data_member_count = content.matches("SPACETIMEDB_DATA_MEMBER(").count();
            
            // We should have data member annotations for struct fields
            if !field_lines.is_empty() {
                assert!(
                    data_member_count > 0,
                    "File {} should have SPACETIMEDB_DATA_MEMBER annotations for fields",
                    filename
                );
            }
        }
    }
    
    // Verify specific expected files are generated
    let filenames: Vec<&String> = files.iter().map(|(name, _)| name).collect();
    
    // Should generate RawModuleDef related files
    assert!(
        filenames.iter().any(|name| name.contains("RawModuleDef")),
        "Should generate RawModuleDef related files"
    );
    
    Ok(())
}

#[test]
fn test_cpp_enhanced_type_registration_format() -> anyhow::Result<()> {
    // Create a simple test module
    let module = RawModuleDefV8::with_builder(|module| {
        module.add_type::<RawModuleDef>();
    });
    
    let module: ModuleDef = module.try_into()?;
    
    // Generate C++ code
    let files = generate(
        &module,
        &cpp_enhanced::CppEnhanced {
            namespace: "SpacetimeDb::Internal",
        },
    );
    
    // Check that the type registration follows the correct format
    for (filename, content) in &files {
        // Find all SPACETIMEDB_TYPE calls
        for line in content.lines() {
            if line.starts_with("SPACETIMEDB_TYPE(") {
                // Should follow the format: SPACETIMEDB_TYPE(TypeName)
                assert!(
                    line.ends_with(")"),
                    "Type registration in {} should end with ): {}",
                    filename,
                    line
                );
                
                // Extract type name
                let type_name = line
                    .strip_prefix("SPACETIMEDB_TYPE(")
                    .and_then(|s| s.strip_suffix(")"))
                    .expect("Should extract type name");
                
                // Type name should not be empty
                assert!(
                    !type_name.is_empty(),
                    "Type name should not be empty in {}: {}",
                    filename,
                    line
                );
                
                // Should be a valid identifier (basic check)
                assert!(
                    type_name.chars().all(|c| c.is_alphanumeric() || c == '_'),
                    "Type name should be valid identifier in {}: {}",
                    filename,
                    type_name
                );
            }
        }
    }
    
    Ok(())
}

#[test]
fn test_cpp_enhanced_data_member_format() -> anyhow::Result<()> {
    // Create a test module
    let module = RawModuleDefV8::with_builder(|module| {
        module.add_type::<RawModuleDef>();
    });
    
    let module: ModuleDef = module.try_into()?;
    
    // Generate C++ code
    let files = generate(
        &module,
        &cpp_enhanced::CppEnhanced {
            namespace: "SpacetimeDb::Internal",
        },
    );
    
    // Check SPACETIMEDB_DATA_MEMBER format
    for (filename, content) in &files {
        for line in content.lines() {
            if line.trim().starts_with("SPACETIMEDB_DATA_MEMBER(") {
                // Should follow the format: SPACETIMEDB_DATA_MEMBER("field_name")
                assert!(
                    line.contains("\"") && line.trim().ends_with(")"),
                    "Data member annotation in {} should have quoted field name: {}",
                    filename,
                    line.trim()
                );
                
                // Should have exactly two quotes
                let quote_count = line.matches('"').count();
                assert!(
                    quote_count == 2,
                    "Data member annotation in {} should have exactly two quotes: {}",
                    filename,
                    line.trim()
                );
            }
        }
    }
    
    Ok(())
}

#[test]
fn test_cpp_enhanced_namespace_consistency() -> anyhow::Result<()> {
    let test_namespace = "TestNamespace::Internal";
    
    // Create a test module
    let module = RawModuleDefV8::with_builder(|module| {
        module.add_type::<RawModuleDef>();
    });
    
    let module: ModuleDef = module.try_into()?;
    
    // Generate C++ code with custom namespace
    let files = generate(
        &module,
        &cpp_enhanced::CppEnhanced {
            namespace: test_namespace,
        },
    );
    
    // Check that the namespace is used consistently
    for (filename, content) in &files {
        // Should contain the namespace declaration
        assert!(
            content.contains(&format!("namespace {} {{", test_namespace)),
            "File {} should contain namespace declaration for {}",
            filename,
            test_namespace
        );
        
        // Should contain the namespace closing
        assert!(
            content.contains(&format!("}} // namespace {}", test_namespace)),
            "File {} should contain namespace closing for {}",
            filename,
            test_namespace
        );
        
        // Type references should use the namespace
        if content.contains("::Internal::") {
            assert!(
                content.contains(&format!("{}::", test_namespace)),
                "File {} should use the configured namespace in type references",
                filename
            );
        }
    }
    
    Ok(())
}

#[test]
fn test_cpp_enhanced_includes_present() -> anyhow::Result<()> {
    // Create a test module
    let module = RawModuleDefV8::with_builder(|module| {
        module.add_type::<RawModuleDef>();
    });
    
    let module: ModuleDef = module.try_into()?;
    
    // Generate C++ code
    let files = generate(
        &module,
        &cpp_enhanced::CppEnhanced {
            namespace: "SpacetimeDb::Internal",
        },
    );
    
    // Check that all files have the necessary includes
    for (filename, content) in &files {
        // Should have pragma once
        assert!(
            content.contains("#pragma once"),
            "File {} should have #pragma once",
            filename
        );
        
        // Should include standard headers
        assert!(
            content.contains("#include <cstdint>"),
            "File {} should include <cstdint>",
            filename
        );
        
        assert!(
            content.contains("#include <string>"),
            "File {} should include <string>",
            filename
        );
        
        // Should include BSATN header
        assert!(
            content.contains("#include \"spacetimedb/bsatn/bsatn.h\""),
            "File {} should include BSATN header",
            filename
        );
        
        // Should include macros header (added after namespace)
        assert!(
            content.contains("#include \"spacetimedb/macros.h\""),
            "File {} should include macros header",
            filename
        );
    }
    
    Ok(())
}