//! Example of instantiating a wasm module which uses WASI imports.
use wasi_common::sync::WasiCtxBuilder;
use wasmtime::*;

fn run_test(guard_size: u64, name: &str) -> Result<()> {
    // Configure the engine with specified memory guard size
    let mut config = Config::new();
    config.memory_guard_size(guard_size);
    let engine = Engine::new(&config)?;

    let mut linker = Linker::new(&engine);
    wasi_common::sync::add_to_linker(&mut linker, |s| s)?;

    let wasi = WasiCtxBuilder::new()
        .inherit_stdio()
        .inherit_args()?
        .build();
    let mut store = Store::new(&engine, wasi);

    let module = Module::from_file(&engine, "./perf_test.wasm")?;
    linker.module(&mut store, "", &module)?;

    println!("----{} (guard_size: {} bytes)----", name, guard_size);
    let sizes = [32, 64, 128, 256, 512, 1024, 2048];
    
    for size in sizes {
        let start = std::time::Instant::now();
        let func = linker.get(&mut store, "", "calculate")
            .ok_or(anyhow::Error::msg("calculate function not found"))?
            .into_func()
            .ok_or(anyhow::Error::msg("not a function"))?;
            
        let result = func.typed::<i32, i32>(&store)?
            .call(&mut store, size)?;
        println!("Size {}: {:.6} seconds", size, start.elapsed().as_secs_f64());
    }

    Ok(())
}

fn main() -> Result<()> {
    // 运行两组测试
    run_test(0, "Without Guard Pages")?;
    println!("\n"); // 添加空行分隔
    run_test(65536, "With Guard Pages")?;  // 使用默认的 64KB guard size
    Ok(())
}