use spacetimedb::{table, SpacetimeType};

#[table(name = one_u8)]
pub struct OneU8 {
    pub n: u8,
}