use simulator::{Config, ffi::simulate, read_shader};

fn main() {
    let config = Config::get();

    let shader_src = read_shader();

    if config.print_enabled {
        println!("CONVENTION: 0 -> Burning tree, 1 -> Alive n well, 3 -> Burnt.\n");
    }

    let epoch_count = unsafe {
        simulate(
            shader_src.as_ptr(),
            shader_src.len() as u64,
            config.tree_grid_size,
            config.starting_factor,
            config.print_enabled
        )
    };

    if epoch_count == 1 {
        println!("The simulation ended in: 1 epoch.");
    }
    else {
        println!("The simulation ended in: {} epochs.", epoch_count);
    }
}