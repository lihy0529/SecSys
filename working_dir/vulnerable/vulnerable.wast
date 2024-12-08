(module
  ;; 导入 WASI 函数
  (import "wasi_unstable" "args_get" (func $wasi_args_get (param i32 i32) (result i32)))
  (import "wasi_unstable" "args_sizes_get" (func $wasi_args_sizes_get (param i32 i32) (result i32)))
  (import "wasi_unstable" "fd_write" (func $wasi_fd_write (param i32 i32 i32 i32) (result i32)))
  (import "wasi_unstable" "proc_exit" (func $wasi_proc_exit (param i32)))

  (memory (export "memory") 2)
  
  ;; 内存布局
  (global $input_buf i32 (i32.const 0))    ;; 输入数据从0开始
  (global $buffer_2 i32 (i32.const 16))  ;; buffer_2 在低地址
  (global $buffer_1 i32 (i32.const 8))  ;; buffer_1 在高地址
  (global $args_ptrs i32 (i32.const 2048))
  
  ;; 打印缓冲区状态
  (func $print_state
    (local $iov i32)
    
    ;; 设置iovec指向两个缓冲区区域
    (local.set $iov (i32.const 3000))
    (i32.store (local.get $iov) (global.get $buffer_2))  ;; 从buffer_2开始打印
    (i32.store offset=4 (local.get $iov) (i32.const 16))  ;; 打印16字节，包含两个缓冲区
    
    ;; 打印当前内存状态
    (call $wasi_fd_write
      (i32.const 1)
      (local.get $iov)
      (i32.const 1)
      (i32.const 3016))
    drop
  )

  ;; vulnerable_function
  (func $vulnerable_function
    (local $input_ptr i32)
    
    ;; 初始化 buffer_2 为 "SAFE"
    (memory.copy 
      (global.get $buffer_2)
      (global.get $input_buf)  ;; 从地址0复制"SAFE"
      (i32.const 4))
    
    ;; 打印初始状态
    (call $print_state)
    
    ;; 获取第二个参数（跳过程序名）并复制到地址0
    (local.set $input_ptr (i32.load offset=4 (global.get $args_ptrs)))
    (memory.copy
      (global.get $input_buf)
      (local.get $input_ptr)
      (i32.const 16))
    
    ;; 复制输入到buffer_1（故意溢出）
    (memory.copy
      (global.get $buffer_1)
      (global.get $input_buf)
      (i32.const 16))  ;; 写入16字节到8字节缓冲区
    
    ;; 打印溢出后状态
    (call $print_state)
  )

  ;; 主函数
  (func $main (export "_start")
    ;; 获取参数
    (call $wasi_args_sizes_get
      (i32.const 3100)
      (i32.const 3104))
    drop
    
    (call $wasi_args_get
      (global.get $args_ptrs)
      (i32.add (global.get $args_ptrs) (i32.const 256)))
    drop
    
    ;; 调用漏洞函数
    (call $vulnerable_function)
    
    (call $wasi_proc_exit (i32.const 0))
  )
)