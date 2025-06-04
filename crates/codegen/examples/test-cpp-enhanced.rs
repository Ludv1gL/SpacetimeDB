//! Test the enhanced C++ code generation with type registration macros

use spacetimedb_codegen::{cpp_enhanced, generate};
use spacetimedb_lib::{RawModuleDef, RawModuleDefV8};
use spacetimedb_schema::def::ModuleDef;

fn main() -> anyhow::Result<()> {
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
    
    // Print generated files - only show the first 100 lines of each
    for (filename, content) in files {
        println!("=== {} ===", filename);
        let lines: Vec<&str> = content.lines().collect();
        for (i, line) in lines.iter().take(100).enumerate() {
            println!("{:3}: {}", i + 1, line);
        }
        if lines.len() > 100 {
            println!("... ({} more lines)", lines.len() - 100);
        }
        println!();
    }
    
    Ok(())
}