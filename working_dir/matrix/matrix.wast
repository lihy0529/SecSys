(module
  ;; 导入必要的内存和随机数函数
  (import "env" "memory" (memory 1024))  ;; 需要足够的内存来存储矩阵
  (import "env" "rand" (func $rand (result i32)))

  ;; 常量定义
  (global $SIZE i32 (i32.const 32))
  (global $MATRIX_SIZE i32 (i32.const 1024))  ;; 32 * 32
  
  ;; 矩阵在内存中的偏移量
  (global $matrixA_offset i32 (i32.const 0))
  (global $matrixB_offset i32 (i32.const 4096))    ;; 32 * 32 * 4
  (global $result_offset i32 (i32.const 8192))     ;; 2 * 32 * 32 * 4

  ;; 主函数
  (func $performance_test (export "performance_test")
    (local $i i32)
    (local $j i32)
    (local $k i32)
    (local $temp i32)
    (local $sum i32)
    (local $idx i32)
    
    ;; 初始化矩阵
    (local.set $i (i32.const 0))
    (block $init_outer_break
      (loop $init_outer_loop
        (local.set $j (i32.const 0))
        
        (block $init_inner_break
          (loop $init_inner_loop
            (local.set $idx 
              (i32.add 
                (global.get $matrixA_offset)
                (i32.mul 
                  (i32.add
                    (i32.mul (local.get $i) (global.get $SIZE))
                    (local.get $j)
                  )
                  (i32.const 4)
                )
              )
            )
            
            ;; matrixA[i][j] = rand() % 100
            (i32.store
              (local.get $idx)
              (i32.rem_u
                (call $rand)
                (i32.const 100)
              )
            )
            
            ;; matrixB[i][j] = rand() % 100
            (i32.store
              (i32.add
                (local.get $idx)
                (i32.sub 
                  (global.get $matrixB_offset)
                  (global.get $matrixA_offset)
                )
              )
              (i32.rem_u
                (call $rand)
                (i32.const 100)
              )
            )
            
            ;; result[i][j] = 0
            (i32.store
              (i32.add
                (local.get $idx)
                (i32.sub 
                  (global.get $result_offset)
                  (global.get $matrixA_offset)
                )
              )
              (i32.const 0)
            )
            
            ;; j++
            (local.set $j
              (i32.add (local.get $j) (i32.const 1))
            )
            (br_if $init_inner_loop
              (i32.lt_s (local.get $j) (global.get $SIZE))
            )
          )
        )
        
        ;; i++
        (local.set $i
          (i32.add (local.get $i) (i32.const 1))
        )
        (br_if $init_outer_loop
          (i32.lt_s (local.get $i) (global.get $SIZE))
        )
      )
    )
    
    ;; 矩阵乘法
    (local.set $i (i32.const 0))
    (block $mult_i_break
      (loop $mult_i_loop
        (local.set $j (i32.const 0))
        
        (block $mult_j_break
          (loop $mult_j_loop
            (local.set $sum (i32.const 0))
            (local.set $k (i32.const 0))
            
            (block $mult_k_break
              (loop $mult_k_loop
                ;; sum += matrixA[i][k] * matrixB[k][j]
                (local.set $sum
                  (i32.add
                    (local.get $sum)
                    (i32.mul
                      (i32.load
                        (i32.add
                          (global.get $matrixA_offset)
                          (i32.mul 
                            (i32.add
                              (i32.mul (local.get $i) (global.get $SIZE))
                              (local.get $k)
                            )
                            (i32.const 4)
                          )
                        )
                      )
                      (i32.load
                        (i32.add
                          (global.get $matrixB_offset)
                          (i32.mul 
                            (i32.add
                              (i32.mul (local.get $k) (global.get $SIZE))
                              (local.get $j)
                            )
                            (i32.const 4)
                          )
                        )
                      )
                    )
                  )
                )
                
                ;; k++
                (local.set $k
                  (i32.add (local.get $k) (i32.const 1))
                )
                (br_if $mult_k_loop
                  (i32.lt_s (local.get $k) (global.get $SIZE))
                )
              )
            )
            
            ;; result[i][j] = sum
            (i32.store
              (i32.add
                (global.get $result_offset)
                (i32.mul 
                  (i32.add
                    (i32.mul (local.get $i) (global.get $SIZE))
                    (local.get $j)
                  )
                  (i32.const 4)
                )
              )
              (local.get $sum)
            )
            
            ;; j++
            (local.set $j
              (i32.add (local.get $j) (i32.const 1))
            )
            (br_if $mult_j_loop
              (i32.lt_s (local.get $j) (global.get $SIZE))
            )
          )
        )
        
        ;; i++
        (local.set $i
          (i32.add (local.get $i) (i32.const 1))
        )
        (br_if $mult_i_loop
          (i32.lt_s (local.get $i) (global.get $SIZE))
        )
      )
    )
  )
)