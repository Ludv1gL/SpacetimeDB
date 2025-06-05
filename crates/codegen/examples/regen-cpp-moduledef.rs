//! This script is used to generate the C++ bindings for the `RawModuleDef` type.
//! Run `cargo run --example regen-cpp-moduledef` to update C++ bindings whenever the module definition changes.

use fs_err as fs;
use spacetimedb_codegen::{cpp_enhanced, generate};
use spacetimedb_lib::{RawModuleDef, RawModuleDefV8};
use spacetimedb_schema::def::ModuleDef;
use std::path::Path;

fn main() -> anyhow::Result<()> {
    let module = RawModuleDefV8::with_builder(|module| {
        module.add_type::<RawModuleDef>();
    });

    // Build relative path from the codegen crate to the C++ Module Library autogen directory
    let manifest_dir = env!("CARGO_MANIFEST_DIR");
    let dir = Path::new(manifest_dir)
        .parent()
        .unwrap()
        .parent()
        .unwrap()
        .join("bindings-cpp/library/include/spacetimedb/internal/autogen");
    
    println!("Target directory path: {}", dir.display());

    // Create the autogen directory if it doesn't exist
    if dir.exists() {
        fs::remove_dir_all(&dir)?;
    }
    fs::create_dir_all(&dir)?;

    let module: ModuleDef = module.try_into()?;
    generate(
        &module,
        &cpp_enhanced::CppEnhanced {
            namespace: "SpacetimeDb::Internal",
        },
    )
    .into_iter()
    .try_for_each(|(filename, code)| {
        // Remove any prefix and just use the filename
        let filename = if let Some(name) = filename.strip_prefix("Types/") {
            name
        } else {
            &filename
        };

        println!("Generating {}", filename);
        fs::write(dir.join(filename), code)
    })?;

    println!("C++ autogen files written to: {}", dir.display());
    Ok(())
}