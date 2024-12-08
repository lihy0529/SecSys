const fs = require('fs');

function print(message) {
    console.log(message);
}

async function runTest() {
    const wasmBuffer = fs.readFileSync('./perf_test.wasm');
    const module = await WebAssembly.compile(wasmBuffer);
    const instance = await WebAssembly.instantiate(module, {});
    
    let startTime = process.hrtime.bigint();
    let result = instance.exports.calculate(32);
    let endTime = process.hrtime.bigint();
    let duration = Number(endTime - startTime) / 1e9;
    
    print(`Size 32: ${duration} seconds`);

    startTime = process.hrtime.bigint();
    result = instance.exports.calculate(64);
    endTime = process.hrtime.bigint();
    duration = Number(endTime - startTime) / 1e9;
    print(`Size 64: ${duration} seconds`);

    startTime = process.hrtime.bigint();
    result = instance.exports.calculate(128);
    endTime = process.hrtime.bigint();
    duration = Number(endTime - startTime) / 1e9;
    print(`Size 128: ${duration} seconds`);

    startTime = process.hrtime.bigint();
    result = instance.exports.calculate(256);
    endTime = process.hrtime.bigint();
    duration = Number(endTime - startTime) / 1e9;
    print(`Size 256: ${duration} seconds`);

    startTime = process.hrtime.bigint();
    result = instance.exports.calculate(512);
    endTime = process.hrtime.bigint();
    duration = Number(endTime - startTime) / 1e9;
    print(`Size 512: ${duration} seconds`);

    startTime = process.hrtime.bigint();
    result = instance.exports.calculate(1024);
    endTime = process.hrtime.bigint();
    duration = Number(endTime - startTime) / 1e9;
    print(`Size 1024: ${duration} seconds`);

    startTime = process.hrtime.bigint();
    result = instance.exports.calculate(2048);
    endTime = process.hrtime.bigint();
    duration = Number(endTime - startTime) / 1e9;
    print(`Size 2048: ${duration} seconds`);
}

// 确保输出目录存在
if (!fs.existsSync('./tmp')) {
    fs.mkdirSync('./tmp');
}

runTest().catch(err => {
    print(`Error: ${err.message}`);
    console.error(err);
});