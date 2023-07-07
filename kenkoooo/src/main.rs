fn main() -> Result<(), Box<dyn std::error::Error>> {
    env_logger::init();
    let args: Vec<String> = std::env::args().collect();
    kenkoooo::run(&args[1], &args[2])?;
    Ok(())
}
