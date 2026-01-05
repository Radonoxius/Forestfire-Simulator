use std::{env, fs::OpenOptions, io::Read, path::PathBuf};

pub mod ffi;

pub fn read_shader() -> Vec<u8> {
    let shader_path: PathBuf;

    if cfg!(feature = "appimage") {
        shader_path = PathBuf::from(
            format!(
                "{}/usr/bin/shaders/simulate.cl",
                env::var("APPDIR").unwrap()
            )
        );
    }
    else {
        shader_path = [".", "shaders", "simulate.cl"].iter().collect();
    }

    let mut src = Vec::new();
    let mut handle = OpenOptions::new().read(true).open(shader_path).unwrap();

    handle.read_to_end(&mut src).unwrap();

    src
}

pub struct Config {
    pub tree_grid_size: u32,
    pub starting_factor: u8,
    pub print_enabled: bool
}

impl Config {
    pub fn get() -> Self {
        let args_string: Vec<String> = env::args().collect();

        if args_string.len() < 3 || args_string.len() > 4 {
            panic!("Too few/many arguments were provided!")
        }
        else {
            let tree_grid_size_res = args_string[1].parse::<u32>();
            let tree_grid_size: u32;
            if let Err(_) = tree_grid_size_res {
                panic!("First argument (TREE-GRID-SIZE), MUST be an unsigned 32-bit integer!");
            }
            tree_grid_size = tree_grid_size_res.unwrap();

            let starting_factor_res = args_string[2].parse::<u8>();
            let starting_factor: u8;
            if let Err(_) = starting_factor_res {
                panic!("Second argument (STARTING-FACTOR), MUST be an unsigned 8-bit integer! \
                    NOTE: 255 means all trees start off in alive state, \
                    0 means all trees start off in burning state."
                );
            }
            starting_factor = starting_factor_res.unwrap();
            
            let mut print_enabled: bool = false;
            if args_string.len() == 4 {
                let print_enabled_res = &args_string[3];
                if print_enabled_res == "t" ||
                    print_enabled_res == "T" ||
                    print_enabled_res == "true" ||
                    print_enabled_res == "y" ||
                    print_enabled_res == "Y" ||
                    print_enabled_res == "yes" {
                    print_enabled = true;
                }
            }

            Self {
                tree_grid_size,
                starting_factor,
                print_enabled
            }
        }
    }
}