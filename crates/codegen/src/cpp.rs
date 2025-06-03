//! C++ code generation for SpacetimeDB module definitions.
//! Generates C++ headers for all module definition types.

use super::Lang;
use spacetimedb_schema::def::{ModuleDef, ReducerDef, ScopedTypeName, TableDef, TypeDef};
use spacetimedb_schema::identifier::Identifier;
use spacetimedb_schema::type_for_generate::{
    AlgebraicTypeDef, AlgebraicTypeUse, PlainEnumTypeDef, PrimitiveType, ProductTypeDef, SumTypeDef,
};
use spacetimedb_sats::AlgebraicTypeRef;
use std::fmt::{self, Write};
use std::collections::HashSet;


pub struct Cpp {
    pub namespace: &'static str,
}

impl Lang for Cpp {
    fn table_filename(&self, _module: &ModuleDef, table: &TableDef) -> String {
        format!("Types/{}.g.h", table.name)
    }

    fn type_filename(&self, type_name: &ScopedTypeName) -> String {
        format!("Types/{}.g.h", type_name.name())
    }

    fn reducer_filename(&self, reducer_name: &Identifier) -> String {
        format!("Types/{}.g.h", reducer_name)
    }

    fn generate_table(&self, module: &ModuleDef, table: &TableDef) -> String {
        let mut output = String::new();
        self.write_header_comment(&mut output);
        
        // For tables, we need to collect the type dependencies from the product type
        let mut include_deps = vec![];
        
        // Find the product type for this table
        if let Some(table_type) = module.typespace_for_generate().get(table.product_type_ref) {
            if let AlgebraicTypeDef::Product(product) = table_type {
                let deps = self.collect_product_dependencies(module, product);
                for dep in deps {
                    include_deps.push(format!("{}.g.h", dep));
                }
            }
        }
        
        let includes: Vec<&str> = include_deps.iter().map(|s| s.as_str()).collect();
        self.write_includes(&mut output, &includes);
        self.write_namespace_begin(&mut output);
        
        // Write the table definition
        writeln!(&mut output, "// Table definition for {}", table.name).unwrap();
        writeln!(&mut output, "// This represents the structure used in module definitions").unwrap();
        writeln!(&mut output, "// TODO: Replace with actual table type from schema").unwrap();
        writeln!(&mut output, "using TableDef_{} = void*;", table.name).unwrap();
        
        self.write_namespace_end(&mut output);
        output
    }

    fn generate_type(&self, module: &ModuleDef, typ: &TypeDef) -> String {
        let mut output = String::new();
        self.write_header_comment(&mut output);
        self.write_type_def(&mut output, module, typ);
        output
    }

    fn generate_reducer(&self, _module: &ModuleDef, reducer: &ReducerDef) -> String {
        let mut output = String::new();
        self.write_header_comment(&mut output);
        
        // For reducers, we need to collect dependencies from their arguments
        // TODO: Implement reducer argument type dependency collection
        let includes: Vec<&str> = vec![];
        
        self.write_includes(&mut output, &includes);
        self.write_namespace_begin(&mut output);
        
        writeln!(&mut output, "// Reducer definition for {}", reducer.name).unwrap();
        writeln!(&mut output, "// TODO: Replace with actual reducer type from schema").unwrap();
        writeln!(&mut output, "using ReducerDef_{} = void*;", reducer.name).unwrap();
        
        self.write_namespace_end(&mut output);
        output
    }

    fn generate_globals(&self, _module: &ModuleDef) -> Vec<(String, String)> {
        // Generate additional global headers if needed
        vec![]
    }
}

impl Cpp {
    /// Collect all type dependencies from an AlgebraicTypeUse
    fn collect_type_dependencies(&self, module: &ModuleDef, typ: &AlgebraicTypeUse, deps: &mut HashSet<String>) {
        match typ {
            AlgebraicTypeUse::Array(elem_type) => {
                self.collect_type_dependencies(module, elem_type, deps);
            }
            AlgebraicTypeUse::Option(inner_type) => {
                self.collect_type_dependencies(module, inner_type, deps);
            }
            AlgebraicTypeUse::Ref(type_ref) => {
                let resolved_type = self.resolve_type_ref(module, type_ref);
                // Don't include ProductType_, SumType_, etc placeholders
                if !resolved_type.starts_with("ProductType_") 
                    && !resolved_type.starts_with("SumType_") 
                    && !resolved_type.starts_with("PlainEnum_")
                    && !resolved_type.starts_with("TypeRef_") {
                    deps.insert(resolved_type);
                }
            }
            _ => {} // Primitives and strings don't need includes
        }
    }
    
    /// Collect all type dependencies from a ProductTypeDef
    fn collect_product_dependencies(&self, module: &ModuleDef, product: &ProductTypeDef) -> Vec<String> {
        let mut deps = HashSet::new();
        for (_, field_type) in &product.elements {
            self.collect_type_dependencies(module, field_type, &mut deps);
        }
        let mut sorted_deps: Vec<_> = deps.into_iter().collect();
        sorted_deps.sort();
        sorted_deps
    }
    
    /// Resolve a type reference to its actual C++ type name
    fn resolve_type_ref(&self, module: &ModuleDef, type_ref: &AlgebraicTypeRef) -> String {
        let typespace = module.typespace_for_generate();
        
        // Look through the types in the module to find the one with this type reference
        for typ in module.types() {
            if typ.ty == *type_ref {
                return typ.name.name().to_string();
            }
        }
        
        // If not found in user types, check if it's a built-in type in the typespace
        if let Some(algebraic_type) = typespace.get(*type_ref) {
            match algebraic_type {
                AlgebraicTypeDef::Product(_) => {
                    format!("ProductType_{}", type_ref.idx())
                }
                AlgebraicTypeDef::Sum(_) => {
                    format!("SumType_{}", type_ref.idx())
                }
                AlgebraicTypeDef::PlainEnum(_) => {
                    format!("PlainEnum_{}", type_ref.idx())
                }
            }
        } else {
            format!("TypeRef_{}", type_ref.idx())
        }
    }
    fn write_header_comment(&self, output: &mut String) {
        writeln!(
            output,
            "// THIS FILE IS AUTOMATICALLY GENERATED BY SPACETIMEDB. EDITS TO THIS FILE"
        ).unwrap();
        writeln!(output, "// WILL NOT BE SAVED. MODIFY TABLES IN YOUR MODULE SOURCE CODE INSTEAD.").unwrap();
        writeln!(output).unwrap();
        writeln!(output, "// This was generated using spacetimedb codegen.").unwrap();
        writeln!(output).unwrap();
    }

    fn write_includes(&self, output: &mut String, extra_includes: &[&str]) {
        writeln!(output, "#pragma once").unwrap();
        writeln!(output).unwrap();
        writeln!(output, "#include <cstdint>").unwrap();
        writeln!(output, "#include <string>").unwrap();
        writeln!(output, "#include <vector>").unwrap();
        writeln!(output, "#include <optional>").unwrap();
        writeln!(output, "#include <memory>").unwrap();
        writeln!(output, "#include \"spacetimedb/bsatn/bsatn.h\"").unwrap();
        
        for include in extra_includes {
            writeln!(output, "#include \"{}\"", include).unwrap();
        }
        writeln!(output).unwrap();
    }

    fn write_namespace_begin(&self, output: &mut String) {
        writeln!(output, "namespace {} {{", self.namespace).unwrap();
        writeln!(output).unwrap();
    }

    fn write_namespace_end(&self, output: &mut String) {
        writeln!(output, "}} // namespace {}", self.namespace).unwrap();
    }

    fn cpp_primitive_type(&self, primitive: &PrimitiveType) -> &'static str {
        match primitive {
            PrimitiveType::Bool => "bool",
            PrimitiveType::I8 => "int8_t",
            PrimitiveType::U8 => "uint8_t", 
            PrimitiveType::I16 => "int16_t",
            PrimitiveType::U16 => "uint16_t",
            PrimitiveType::I32 => "int32_t",
            PrimitiveType::U32 => "uint32_t",
            PrimitiveType::I64 => "int64_t",
            PrimitiveType::U64 => "uint64_t",
            PrimitiveType::I128 => "SpacetimeDb::Types::int128_t_placeholder",
            PrimitiveType::U128 => "SpacetimeDb::Types::uint128_t_placeholder",
            PrimitiveType::I256 => "SpacetimeDb::sdk::i256_placeholder", 
            PrimitiveType::U256 => "SpacetimeDb::sdk::u256_placeholder",
            PrimitiveType::F32 => "float",
            PrimitiveType::F64 => "double",
        }
    }

    fn write_algebraic_type(&self, output: &mut String, module: &ModuleDef, typ: &AlgebraicTypeUse) -> fmt::Result {
        match typ {
            AlgebraicTypeUse::Primitive(p) => write!(output, "{}", self.cpp_primitive_type(p)),
            AlgebraicTypeUse::Array(elem_type) => {
                write!(output, "std::vector<")?;
                self.write_algebraic_type(output, module, elem_type)?;
                write!(output, ">")
            }
            AlgebraicTypeUse::Option(inner_type) => {
                write!(output, "std::optional<")?;
                self.write_algebraic_type(output, module, inner_type)?;
                write!(output, ">")
            }
            AlgebraicTypeUse::String => write!(output, "std::string"),
            AlgebraicTypeUse::Ref(type_ref) => {
                let resolved_type = self.resolve_type_ref(module, type_ref);
                write!(output, "{}", resolved_type)
            }
            _ => write!(output, "/* unhandled type */"),
        }
    }

    fn write_type_def(&self, output: &mut String, module: &ModuleDef, typ: &TypeDef) {
        let type_name = &typ.name.name().to_string();
        
        // Collect type dependencies for includes
        let mut include_deps = vec![];
        match &module.typespace_for_generate()[typ.ty] {
            AlgebraicTypeDef::Product(product) => {
                let deps = self.collect_product_dependencies(module, product);
                for dep in deps {
                    include_deps.push(format!("{}.g.h", dep));
                }
            }
            AlgebraicTypeDef::Sum(_sum) => {
                // TODO: Collect sum type dependencies when we implement proper sum type support
            }
            AlgebraicTypeDef::PlainEnum(_) => {
                // Enums don't have dependencies
            }
        }
        
        let includes: Vec<&str> = include_deps.iter().map(|s| s.as_str()).collect();
        self.write_includes(output, &includes);
        self.write_namespace_begin(output);

        match &module.typespace_for_generate()[typ.ty] {
            AlgebraicTypeDef::Product(product) => {
                self.write_product_type(output, module, type_name, product);
            }
            AlgebraicTypeDef::Sum(sum) => {
                self.write_sum_type(output, module, type_name, sum);
            }
            AlgebraicTypeDef::PlainEnum(plain_enum) => {
                self.write_plain_enum(output, type_name, plain_enum);
                self.write_namespace_end(output);
                writeln!(output).unwrap();
                
                // BSATN serialization functions outside the main namespace
                writeln!(output, "// BSATN serialization for {}", type_name).unwrap();
                writeln!(output, "namespace SpacetimeDb::bsatn {{").unwrap();
                writeln!(output, "    inline void serialize(Writer& w, const {}::{}& value) {{", self.namespace, type_name).unwrap();
                writeln!(output, "        w.write_u8(static_cast<uint8_t>(value));").unwrap();
                writeln!(output, "    }}").unwrap();
                writeln!(output).unwrap();
                writeln!(output, "    template<>").unwrap();
                writeln!(output, "    inline {}::{} deserialize<{}::{}>(Reader& r) {{", self.namespace, type_name, self.namespace, type_name).unwrap();
                writeln!(output, "        return static_cast<{}::{}>(r.read_u8());", self.namespace, type_name).unwrap();
                writeln!(output, "    }}").unwrap();
                writeln!(output, "}}").unwrap();
                return; // Early return to avoid double closing namespace
            }
        }

        self.write_namespace_end(output);
    }


    fn write_product_type(&self, output: &mut String, module: &ModuleDef, type_name: &str, product: &ProductTypeDef) {
        writeln!(output, "struct {} {{", type_name).unwrap();
        
        // Write fields - elements are (Identifier, AlgebraicTypeUse) tuples
        for (field_name, field_type) in &product.elements {
            write!(output, "    ").unwrap();
            self.write_algebraic_type(output, module, field_type).unwrap();
            writeln!(output, " {};", field_name).unwrap();
        }
        
        writeln!(output).unwrap();

        // Write default constructor
        writeln!(output, "    {}() = default;", type_name).unwrap();
        writeln!(output).unwrap();

        // Write parameterized constructor if there are fields
        if !product.elements.is_empty() {
            write!(output, "    {}(", type_name).unwrap();
            for (i, (field_name, field_type)) in product.elements.iter().enumerate() {
                if i > 0 {
                    write!(output, ", ").unwrap();
                }
                self.write_algebraic_type(output, module, field_type).unwrap();
                write!(output, " {}", field_name).unwrap();
            }
            writeln!(output, ")").unwrap();
            
            // Constructor body
            write!(output, "        : ").unwrap();
            for (i, (field_name, _)) in product.elements.iter().enumerate() {
                if i > 0 {
                    write!(output, ", ").unwrap();
                }
                write!(output, "{}({})", field_name, field_name).unwrap();
            }
            writeln!(output, " {{}}").unwrap();
        }

        writeln!(output).unwrap();
        writeln!(output, "    // BSATN serialization support").unwrap();
        writeln!(output, "    void bsatn_serialize(SpacetimeDb::bsatn::Writer& writer) const;").unwrap();
        writeln!(output, "    void bsatn_deserialize(SpacetimeDb::bsatn::Reader& reader);").unwrap();
        writeln!(output).unwrap();
        writeln!(output, "    // Static factory method for BSATN deserialization").unwrap();
        writeln!(output, "    static {} from_bsatn(SpacetimeDb::bsatn::Reader& reader) {{", type_name).unwrap();
        writeln!(output, "        {} result;", type_name).unwrap();
        writeln!(output, "        result.bsatn_deserialize(reader);").unwrap();
        writeln!(output, "        return result;").unwrap();
        writeln!(output, "    }}").unwrap();
        
        writeln!(output, "}};").unwrap();
    }

    fn write_sum_type(&self, output: &mut String, _module: &ModuleDef, type_name: &str, sum: &SumTypeDef) {
        // For sum types, generate a variant-like structure
        writeln!(output, "class {} {{", type_name).unwrap();
        writeln!(output, "public:").unwrap();
        
        // Enum for variant tags
        writeln!(output, "    enum class Tag {{").unwrap();
        for (i, (variant_name, _)) in sum.variants.iter().enumerate() {
            writeln!(output, "        {} = {},", variant_name, i).unwrap();
        }
        writeln!(output, "    }};").unwrap();
        writeln!(output).unwrap();

        writeln!(output, "private:").unwrap();
        writeln!(output, "    Tag tag_;").unwrap();
        writeln!(output, "    // Union of variant data would go here").unwrap();
        writeln!(output).unwrap();

        writeln!(output, "public:").unwrap();
        writeln!(output, "    Tag get_tag() const {{ return tag_; }}").unwrap();
        writeln!(output).unwrap();
        
        writeln!(output, "    // BSATN serialization support").unwrap();
        writeln!(output, "    void bsatn_serialize(SpacetimeDb::bsatn::Writer& writer) const;").unwrap();
        writeln!(output, "    void bsatn_deserialize(SpacetimeDb::bsatn::Reader& reader);").unwrap();
        
        writeln!(output, "}};").unwrap();
    }

    fn write_plain_enum(&self, output: &mut String, type_name: &str, plain_enum: &PlainEnumTypeDef) {
        writeln!(output, "enum class {} {{", type_name).unwrap();
        
        for (i, variant) in plain_enum.variants.iter().enumerate() {
            writeln!(output, "    {} = {},", variant, i).unwrap();
        }
        
        writeln!(output, "}};").unwrap();
    }

}