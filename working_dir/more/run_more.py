import os
import glob

def create_run_script(wast_path):
    base_name = os.path.splitext(os.path.basename(wast_path))[0]
    dir_path = os.path.dirname(wast_path)
    
    script_content = f'''#!/bin/bash
cd "{dir_path}"
wat2wasm {base_name}.wast -o {base_name}.wasm
~/workspace/course_project/wabt/bin/wasm2c {base_name}.wasm -o {base_name}.c
gcc {base_name}.c main.c \\
    ~/workspace/course_project/wabt/wasm2c/wasm-rt-impl.o \\
    ~/workspace/course_project/wabt/wasm2c/wasm-rt-mem-impl.o \\
    -I~/workspace/course_project/wabt/wasm2c \\
    -pthread \\
    -Wno-int-to-pointer-cast \\
    -Wno-format \\
    -lm \\
    -o {base_name}
./{base_name}
'''
    
    script_path = os.path.join(dir_path, f'run_{base_name}.sh')
    print(f"Creating script: {script_path}")
    with open(script_path, 'w') as f:
        f.write(script_content)
    os.chmod(script_path, 0o755)
    print(f"Script created: {script_path}")
    
    
    # Execute the script and redirect output to a file
    output_file = os.path.join("./output", f"{base_name.split('.')[0]}.txt")
    os.system(f"{script_path} > {output_file}")
    
    
    # Delete the script after execution
    os.remove(script_path)
    print(f"Deleted script: {script_path}")
    
    
def create_main_c(wast_path):
    base_name = os.path.splitext(os.path.basename(wast_path))[0]
    dir_path = os.path.dirname(wast_path)
    
    main_content = MainContent[os.path.basename(wast_path)]
    main_content = main_content.format(base_name=base_name)  # Format the content with base_name
    
    try:
        with open(os.path.join(dir_path, "main.c"), "w") as f:
            f.write(main_content)
        print(f"Main.c created: {os.path.join(dir_path, 'main.c')}")
        return os.path.join(dir_path, "main.c")
    except Exception as e:
        print(f"Error creating main.c: {e}")
        return None

def process_directory(directory):
    wast_files = glob.glob(os.path.join(directory, '**/*.wast'), recursive=True)
    
    for i, wast_file in enumerate(wast_files):
        file_name = os.path.basename(wast_file)
        print(file_name)
        if file_name in get_main_content():
            print(f"Processing {wast_file}")
            main_path = create_main_c(wast_file)
            create_run_script(wast_file)
            # Delete main.c after processing
            os.remove(main_path)
            print(f"Deleted main.c: {main_path}")

def main():
    base_dir = os.path.expanduser('~/workspace/course_project/working_dir/more')
    subdirs = ['']
    
    for subdir in subdirs:
        dir_path = os.path.join(base_dir, subdir)
        if os.path.exists(dir_path):
            print(f"\nProcessing directory: {dir_path}")
            process_directory(dir_path)
        else:
            print(f"Directory not found: {dir_path}")
        

def get_main_content():
    return MainContent.keys()


MainContent = {
    "loop.wast": '''#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "{base_name}.h"
#include "wasm-rt.h"

static struct w2c_{base_name}* active_instance;

int main(int argc, char* argv[]) {{
    wasm_rt_init();
    struct w2c_{base_name} instance;
    active_instance = &instance;
    wasm2c_{base_name}_instantiate(&instance);
    
    printf("\\nTesting all exported functions:\\n");
    
    // Basic functions
    printf("empty:\\n");
    w2c_{base_name}_empty(&instance);
    
    printf("singular: %d\\n", w2c_{base_name}_singular(&instance));
    printf("multi: %d\\n", w2c_{base_name}_multi(&instance));
    printf("nested: %d\\n", w2c_{base_name}_nested(&instance));
    printf("deep: %d\\n", w2c_{base_name}_deep(&instance));
    
    // as-select functions
    printf("as-select-first: %d\\n", w2c_{base_name}_as0x2Dselect0x2Dfirst(&instance));
    printf("as-select-mid: %d\\n", w2c_{base_name}_as0x2Dselect0x2Dmid(&instance));
    printf("as-select-last: %d\\n", w2c_{base_name}_as0x2Dselect0x2Dlast(&instance));
    
    // as-if functions
    printf("as-if-condition:\\n");
    w2c_{base_name}_as0x2Dif0x2Dcondition(&instance);
    printf("as-if-then: %d\\n", w2c_{base_name}_as0x2Dif0x2Dthen(&instance));
    printf("as-if-else: %d\\n", w2c_{base_name}_as0x2Dif0x2Delse(&instance));
    
    // as-br functions
    printf("as-br_if-first: %d\\n", w2c_{base_name}_as0x2Dbr_if0x2Dfirst(&instance));
    printf("as-br_if-last: %d\\n", w2c_{base_name}_as0x2Dbr_if0x2Dlast(&instance));
    printf("as-br_table-first: %d\\n", w2c_{base_name}_as0x2Dbr_table0x2Dfirst(&instance));
    printf("as-br_table-last: %d\\n", w2c_{base_name}_as0x2Dbr_table0x2Dlast(&instance));
    
    // as-call_indirect functions
    printf("as-call_indirect-first: %d\\n", w2c_{base_name}_as0x2Dcall_indirect0x2Dfirst(&instance));
    printf("as-call_indirect-mid: %d\\n", w2c_{base_name}_as0x2Dcall_indirect0x2Dmid(&instance));
    printf("as-call_indirect-last: %d\\n", w2c_{base_name}_as0x2Dcall_indirect0x2Dlast(&instance));
    
    // as-store functions
    printf("as-store-first:\\n");
    w2c_{base_name}_as0x2Dstore0x2Dfirst(&instance);
    printf("as-store-last:\\n");
    w2c_{base_name}_as0x2Dstore0x2Dlast(&instance);
    
    // as-memory functions
    printf("as-memory.grow-value: %d\\n", w2c_{base_name}_as0x2Dmemory0x2Egrow0x2Dvalue(&instance));
    
    // as-call functions
    printf("as-call-value: %d\\n", w2c_{base_name}_as0x2Dcall0x2Dvalue(&instance));
    
    // as-return functions
    printf("as-return-value: %d\\n", w2c_{base_name}_as0x2Dreturn0x2Dvalue(&instance));
    
    // as-drop functions
    printf("as-drop-operand:\\n");
    w2c_{base_name}_as0x2Ddrop0x2Doperand(&instance);
    
    // as-br functions
    printf("as-br-value: %d\\n", w2c_{base_name}_as0x2Dbr0x2Dvalue(&instance));
    
    // as-local functions
    printf("as-local.set-value: %d\\n", w2c_{base_name}_as0x2Dlocal0x2Eset0x2Dvalue(&instance));
    printf("as-local.tee-value: %d\\n", w2c_{base_name}_as0x2Dlocal0x2Etee0x2Dvalue(&instance));
    
    // as-global functions
    printf("as-global.set-value: %d\\n", w2c_{base_name}_as0x2Dglobal0x2Eset0x2Dvalue(&instance));
    
    // as-load functions
    printf("as-load-operand: %d\\n", w2c_{base_name}_as0x2Dload0x2Doperand(&instance));
    
    // as-unary/binary/test/compare functions
    printf("as-unary-operand: %d\\n", w2c_{base_name}_as0x2Dunary0x2Doperand(&instance));
    printf("as-binary-operand: %d\\n", w2c_{base_name}_as0x2Dbinary0x2Doperand(&instance));
    printf("as-test-operand: %d\\n", w2c_{base_name}_as0x2Dtest0x2Doperand(&instance));
    printf("as-compare-operand: %d\\n", w2c_{base_name}_as0x2Dcompare0x2Doperand(&instance));
    
    // break functions
    printf("break-bare: %d\\n", w2c_{base_name}_break0x2Dbare(&instance));
    printf("break-value: %d\\n", w2c_{base_name}_break0x2Dvalue(&instance));
    printf("break-repeated: %d\\n", w2c_{base_name}_break0x2Drepeated(&instance));
    printf("break-inner: %d\\n", w2c_{base_name}_break0x2Dinner(&instance));
    
    // param functions
    printf("param: %d\\n", w2c_{base_name}_param(&instance));
    printf("params: %d\\n", w2c_{base_name}_params(&instance));
    printf("params-id: %d\\n", w2c_{base_name}_params0x2Did(&instance));
    printf("param-break: %d\\n", w2c_{base_name}_param0x2Dbreak(&instance));
    printf("params-break: %d\\n", w2c_{base_name}_params0x2Dbreak(&instance));
    printf("params-id-break: %d\\n", w2c_{base_name}_params0x2Did0x2Dbreak(&instance));
    
    // effects function
    printf("effects: %d\\n", w2c_{base_name}_effects(&instance));
    
    // while/for functions
    printf("while(5): %lld\\n", w2c_{base_name}_while(&instance, 5));
    printf("for(5): %lld\\n", w2c_{base_name}_for(&instance, 5));
    
    // nesting function
    printf("nesting(2,3): %f\\n", w2c_{base_name}_nesting(&instance, 2.0f, 3.0f));
    
    // type-use function
    printf("type-use:\\n");
    w2c_{base_name}_type0x2Duse(&instance);

    wasm2c_{base_name}_free(&instance);
    wasm_rt_free();
    return 0;
}}
''',
    "address.wast": '''#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "{base_name}.h"
#include "wasm-rt.h"

static struct w2c_{base_name}* active_instance;

int main(int argc, char* argv[]) {{
    wasm_rt_init();
    struct w2c_{base_name} instance;
    active_instance = &instance;
    wasm2c_{base_name}_instantiate(&instance);
    
    printf("\\nTesting all exported functions:\\n");
    
    // Test 8-bit memory access (unsigned)
    printf("8u_good1: %d\\n", w2c_address_8u_good1(&instance, 0));
    printf("8u_good1: %d\\n", w2c_address_8u_good1(&instance, 1));
    printf("8u_good1: %d\\n", w2c_address_8u_good1(&instance, 2));
    printf("8u_good1: %d\\n", w2c_address_8u_good1(&instance, 3));
    
    printf("8u_good2: %d\\n", w2c_address_8u_good2(&instance, 0));
    printf("8u_good2: %d\\n", w2c_address_8u_good2(&instance, 1));
    printf("8u_good2: %d\\n", w2c_address_8u_good2(&instance, 2));
    printf("8u_good2: %d\\n", w2c_address_8u_good2(&instance, 3));
    
    printf("8u_good3: %d\\n", w2c_address_8u_good3(&instance, 0));
    printf("8u_good3: %d\\n", w2c_address_8u_good3(&instance, 1));
    printf("8u_good3: %d\\n", w2c_address_8u_good3(&instance, 2));
    printf("8u_good3: %d\\n", w2c_address_8u_good3(&instance, 3));
    
    printf("8u_good4: %d\\n", w2c_address_8u_good4(&instance, 0));
    printf("8u_good4: %d\\n", w2c_address_8u_good4(&instance, 1));
    printf("8u_good4: %d\\n", w2c_address_8u_good4(&instance, 2));
    printf("8u_good4: %d\\n", w2c_address_8u_good4(&instance, 3));
    
    printf("8u_good5: %d\\n", w2c_address_8u_good5(&instance, 0));
    printf("8u_good5: %d\\n", w2c_address_8u_good5(&instance, 1));
    printf("8u_good5: %d\\n", w2c_address_8u_good5(&instance, 2));
    printf("8u_good5: %d\\n", w2c_address_8u_good5(&instance, 3));
    
    // Test 8-bit memory access (signed)
    printf("8s_good1: %d\\n", w2c_address_8s_good1(&instance, 0));
    printf("8s_good2: %d\\n", w2c_address_8s_good2(&instance, 0));
    printf("8s_good3: %d\\n", w2c_address_8s_good3(&instance, 0));
    printf("8s_good4: %d\\n", w2c_address_8s_good4(&instance, 0));
    printf("8s_good5: %d\\n", w2c_address_8s_good5(&instance, 0));
    
    // Test 16-bit memory access (unsigned)
    printf("16u_good1: %d\\n", w2c_address_16u_good1(&instance, 0));
    printf("16u_good2: %d\\n", w2c_address_16u_good2(&instance, 0));
    printf("16u_good3: %d\\n", w2c_address_16u_good3(&instance, 0));
    printf("16u_good4: %d\\n", w2c_address_16u_good4(&instance, 0));
    printf("16u_good5: %d\\n", w2c_address_16u_good5(&instance, 0));
    
    // Test 16-bit memory access (signed)
    printf("16s_good1: %d\\n", w2c_address_16s_good1(&instance, 0));
    printf("16s_good2: %d\\n", w2c_address_16s_good2(&instance, 0));
    printf("16s_good3: %d\\n", w2c_address_16s_good3(&instance, 0));
    printf("16s_good4: %d\\n", w2c_address_16s_good4(&instance, 0));
    printf("16s_good5: %d\\n", w2c_address_16s_good5(&instance, 0));
    
    // Test 32-bit memory access
    printf("32_good1: %d\\n", w2c_address_32_good1(&instance, 0));
    printf("32_good2: %d\\n", w2c_address_32_good2(&instance, 0));
    printf("32_good3: %d\\n", w2c_address_32_good3(&instance, 0));
    printf("32_good4: %d\\n", w2c_address_32_good4(&instance, 0));
    printf("32_good5: %d\\n", w2c_address_32_good5(&instance, 0));

    wasm2c_{base_name}_free(&instance);
    wasm_rt_free();
    return 0;
}}
''',
    "br_if.wast": '''#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "{base_name}.h"
#include "wasm-rt.h"

int main(int argc, char* argv[]) {{
    wasm_rt_init();
    struct w2c_br__if instance;  // Note the double underscore
    wasm2c_br__if_instantiate(&instance);  // Note the double underscore
    
    printf("\\nTesting br_if operations:\\n");
    
    // Test type operations
    printf("type-i32:\\n");
    w2c_br__if_type0x2Di32(&instance);
    printf("type-i64:\\n");
    w2c_br__if_type0x2Di64(&instance);
    printf("type-f32:\\n");
    w2c_br__if_type0x2Df32(&instance);
    printf("type-f64:\\n");
    w2c_br__if_type0x2Df64(&instance);
    
    // Test type value operations
    printf("type-i32-value: %d\\n", w2c_br__if_type0x2Di320x2Dvalue(&instance));
    printf("type-i64-value: %lld\\n", w2c_br__if_type0x2Di640x2Dvalue(&instance));
    printf("type-f32-value: %f\\n", w2c_br__if_type0x2Df320x2Dvalue(&instance));
    printf("type-f64-value: %lf\\n", w2c_br__if_type0x2Df640x2Dvalue(&instance));
    
    // Test block operations
    printf("as-block-first(0): %d\\n", w2c_br__if_as0x2Dblock0x2Dfirst(&instance, 0));
    printf("as-block-first(1): %d\\n", w2c_br__if_as0x2Dblock0x2Dfirst(&instance, 1));
    printf("as-block-mid(0): %d\\n", w2c_br__if_as0x2Dblock0x2Dmid(&instance, 0));
    printf("as-block-mid(1): %d\\n", w2c_br__if_as0x2Dblock0x2Dmid(&instance, 1));
    
    // Test loop operations
    printf("as-loop-first(0): %d\\n", w2c_br__if_as0x2Dloop0x2Dfirst(&instance, 0));
    printf("as-loop-first(1): %d\\n", w2c_br__if_as0x2Dloop0x2Dfirst(&instance, 1));
    printf("as-loop-mid(0): %d\\n", w2c_br__if_as0x2Dloop0x2Dmid(&instance, 0));
    printf("as-loop-mid(1): %d\\n", w2c_br__if_as0x2Dloop0x2Dmid(&instance, 1));
    
    // Test memory operations
    printf("as-load-address: %d\\n", w2c_br__if_as0x2Dload0x2Daddress(&instance));
    printf("as-loadN-address: %d\\n", w2c_br__if_as0x2DloadN0x2Daddress(&instance));
    printf("as-store-address: %d\\n", w2c_br__if_as0x2Dstore0x2Daddress(&instance));
    printf("as-store-value: %d\\n", w2c_br__if_as0x2Dstore0x2Dvalue(&instance));
    printf("as-storeN-address: %d\\n", w2c_br__if_as0x2DstoreN0x2Daddress(&instance));
    printf("as-storeN-value: %d\\n", w2c_br__if_as0x2DstoreN0x2Dvalue(&instance));
    
    // Test br_if operations
    printf("as-br_if-cond:\\n");
    w2c_br__if_as0x2Dbr_if0x2Dcond(&instance);
    printf("as-br_if-value: %d\\n", w2c_br__if_as0x2Dbr_if0x2Dvalue(&instance));
    printf("as-br_if-value-cond(0): %d\\n", w2c_br__if_as0x2Dbr_if0x2Dvalue0x2Dcond(&instance, 0));
    printf("as-br_if-value-cond(1): %d\\n", w2c_br__if_as0x2Dbr_if0x2Dvalue0x2Dcond(&instance, 1));
    

    wasm2c_br__if_free(&instance);  // Note the double underscore
    wasm_rt_free();
    return 0;
}}
''',
    "endianness.wast": '''#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "{base_name}.h"
#include "wasm-rt.h"

int main(int argc, char* argv[]) {{
    wasm_rt_init();
    struct w2c_endianness instance;
    wasm2c_endianness_instantiate(&instance);
    
    printf("\\nTesting endianness operations:\\n");
    
    // Test i32 load operations
    printf("i32_load16_s(-1): %d\\n", w2c_endianness_i32_load16_s(&instance, -1));
    printf("i32_load16_s(-4242): %d\\n", w2c_endianness_i32_load16_s(&instance, -4242));
    printf("i32_load16_s(42): %d\\n", w2c_endianness_i32_load16_s(&instance, 42));
    printf("i32_load16_s(0x3210): %d\\n", w2c_endianness_i32_load16_s(&instance, 0x3210));

    printf("i32_load16_u(-1): %u\\n", w2c_endianness_i32_load16_u(&instance, -1));
    printf("i32_load16_u(-4242): %u\\n", w2c_endianness_i32_load16_u(&instance, -4242));
    printf("i32_load16_u(42): %u\\n", w2c_endianness_i32_load16_u(&instance, 42));
    printf("i32_load16_u(0xCAFE): %u\\n", w2c_endianness_i32_load16_u(&instance, 0xCAFE));

    printf("i32_load(-1): %d\\n", w2c_endianness_i32_load(&instance, -1));
    printf("i32_load(-42424242): %d\\n", w2c_endianness_i32_load(&instance, -42424242));
    printf("i32_load(42424242): %d\\n", w2c_endianness_i32_load(&instance, 42424242));
    printf("i32_load(0xABAD1DEA): %d\\n", w2c_endianness_i32_load(&instance, 0xABAD1DEA));

    // Test i64 load operations
    printf("i64_load16_s(-1): %lld\\n", w2c_endianness_i64_load16_s(&instance, -1LL));
    printf("i64_load16_s(-4242): %lld\\n", w2c_endianness_i64_load16_s(&instance, -4242LL));
    printf("i64_load16_s(42): %lld\\n", w2c_endianness_i64_load16_s(&instance, 42LL));
    printf("i64_load16_s(0x3210): %lld\\n", w2c_endianness_i64_load16_s(&instance, 0x3210LL));

    printf("i64_load32_s(-1): %lld\\n", w2c_endianness_i64_load32_s(&instance, -1LL));
    printf("i64_load32_s(-42424242): %lld\\n", w2c_endianness_i64_load32_s(&instance, -42424242LL));
    printf("i64_load32_s(42424242): %lld\\n", w2c_endianness_i64_load32_s(&instance, 42424242LL));
    printf("i64_load32_s(0x12345678): %lld\\n", w2c_endianness_i64_load32_s(&instance, 0x12345678LL));

    // Test store operations
    printf("i32_store16(-1): %d\\n", w2c_endianness_i32_store16(&instance, -1));
    printf("i32_store16(-4242): %d\\n", w2c_endianness_i32_store16(&instance, -4242));
    printf("i32_store16(42): %d\\n", w2c_endianness_i32_store16(&instance, 42));
    printf("i32_store16(0xCAFE): %d\\n", w2c_endianness_i32_store16(&instance, 0xCAFE));

    printf("i32_store(-1): %d\\n", w2c_endianness_i32_store(&instance, -1));
    printf("i32_store(-4242): %d\\n", w2c_endianness_i32_store(&instance, -4242));
    printf("i32_store(42424242): %d\\n", w2c_endianness_i32_store(&instance, 42424242));
    printf("i32_store(0xDEADCAFE): %d\\n", w2c_endianness_i32_store(&instance, 0xDEADCAFE));

    // Test floating point operations
    printf("f32_store(-1): %f\\n", w2c_endianness_f32_store(&instance, -1.0f));
    printf("f32_store(1234e-5): %f\\n", w2c_endianness_f32_store(&instance, 1234e-5f));
    printf("f32_store(4242.4242): %f\\n", w2c_endianness_f32_store(&instance, 4242.4242f));

    printf("f64_store(-1): %lf\\n", w2c_endianness_f64_store(&instance, -1.0));
    printf("f64_store(123456789e-5): %lf\\n", w2c_endianness_f64_store(&instance, 123456789e-5));
    printf("f64_store(424242.424242): %lf\\n", w2c_endianness_f64_store(&instance, 424242.424242));

    wasm2c_endianness_free(&instance);
    wasm_rt_free();
    return 0;
}}
''',
    "if.wast": '''#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "{base_name}.h"
#include "wasm-rt.h"

int main(int argc, char* argv[]) {{
    wasm_rt_init();
    struct w2c_if instance;
    wasm2c_if_instantiate(&instance);
    
    printf("\\nTesting if operations:\\n");
    
    // Test empty if statements
    printf("empty(0):\\n");
    w2c_if_empty(&instance, 0);
    printf("empty(1):\\n");
    w2c_if_empty(&instance, 1);
    
    // Test singular if statements
    printf("\\nsingular tests:\\n");
    printf("singular(0): %d\\n", w2c_if_singular(&instance, 0));
    printf("singular(1): %d\\n", w2c_if_singular(&instance, 1));
    
    // Test multi-value if statements
    printf("\\nmulti tests:\\n");
    printf("multi(0): %d\\n", w2c_if_multi(&instance, 0));
    printf("multi(1): %d\\n", w2c_if_multi(&instance, 1));
    
    // Test nested if statements
    printf("\\nnested tests:\\n");
    printf("nested(0,0): %d\\n", w2c_if_nested(&instance, 0, 0));
    printf("nested(0,1): %d\\n", w2c_if_nested(&instance, 0, 1));
    printf("nested(1,0): %d\\n", w2c_if_nested(&instance, 1, 0));
    printf("nested(1,1): %d\\n", w2c_if_nested(&instance, 1, 1));
    
    // Test if as select operations
    printf("\\nas-select tests:\\n");
    printf("as-select-first(0): %d\\n", w2c_if_as0x2Dselect0x2Dfirst(&instance, 0));
    printf("as-select-first(1): %d\\n", w2c_if_as0x2Dselect0x2Dfirst(&instance, 1));
    // load, store
    printf("as-load-operand(0): %d\\n", w2c_if_as0x2Dload0x2Doperand(&instance, 0));
    printf("as-load-operand(1): %d\\n", w2c_if_as0x2Dload0x2Doperand(&instance, 1));
    
    wasm2c_if_free(&instance);
    wasm_rt_free();
    return 0;
}}
''',
    "call.wast": '''#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "{base_name}.h"
#include "wasm-rt.h"

int main(int argc, char* argv[]) {{
    wasm_rt_init();
    struct w2c_call instance;
    wasm2c_call_instantiate(&instance);
    
    printf("\\nTesting basic type calls:\\n");
    printf("type-i32: %d\\n", w2c_call_type0x2Di32(&instance));
    printf("type-i64: %lld\\n", w2c_call_type0x2Di64(&instance));
    printf("type-f32: %f\\n", w2c_call_type0x2Df32(&instance));
    printf("type-f64: %f\\n", w2c_call_type0x2Df64(&instance));
    
    printf("\\nTesting parameter passing:\\n");
    printf("type-first-i32: %d\\n", w2c_call_type0x2Dfirst0x2Di32(&instance));
    printf("type-first-i64: %lld\\n", w2c_call_type0x2Dfirst0x2Di64(&instance));
    printf("type-first-f32: %f\\n", w2c_call_type0x2Dfirst0x2Df32(&instance));
    printf("type-first-f64: %f\\n", w2c_call_type0x2Dfirst0x2Df64(&instance));
    
    printf("\\nTesting composition:\\n");
    printf("as-binary-all-operands: %d\\n", w2c_call_as0x2Dbinary0x2Dall0x2Doperands(&instance));
    printf("as-mixed-operands: %d\\n", w2c_call_as0x2Dmixed0x2Doperands(&instance));
    
    printf("\\nTesting control structures:\\n");
    printf("as-if-condition: %d\\n", w2c_call_as0x2Dif0x2Dcondition(&instance));
    printf("as-br_if-first: %d\\n", w2c_call_as0x2Dbr_if0x2Dfirst(&instance));
    printf("as-br_table-first: %d\\n", w2c_call_as0x2Dbr_table0x2Dfirst(&instance));
    
    printf("\\nTesting memory operations:\\n");
    printf("as-memory.grow-value: %d\\n", w2c_call_as0x2Dmemory0x2Egrow0x2Dvalue(&instance));
    printf("as-load-operand: %d\\n", w2c_call_as0x2Dload0x2Doperand(&instance));
    
    wasm2c_call_free(&instance);
    wasm_rt_free();
    return 0;
}}
'''
}

if __name__ == '__main__':
    main()