use std::fs::File;
use serde_json;

fn main () {
    let path: &str = "./random-test/config.json";
    println!("{:?}", get_json(path));
}

fn get_json (path: &str) -> serde_json::Value {
    let file = File::open(path).unwrap();
    serde_json::from_reader(file).unwrap()
}

/*
fn main() {
    let settings = Config::builder()
        // Add in `./Settings.toml`
        .add_source(config::File::with_name("config"))
        // Add in settings from the environment (with a prefix of APP)
        // Eg.. `APP_DEBUG=1 ./target/app` would set the `debug` key
        .build()
        .unwrap();

    // Print out our settings (as a HashMap)
    println!(
        "{:?}",
        settings
            .try_deserialize::<HashMap<String, String>>()
            .unwrap()
    );
}
*/