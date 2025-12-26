unsafe extern "C" {
    pub fn simulate(
        shader_src: *const u8,
        shader_len: u64,
        tree_grid_size: u32,
        starting_factor: u8,
        print_enabled: bool
    ) -> u64;
}