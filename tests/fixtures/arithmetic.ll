; ModuleID = 'test_arithmetic'
; A minimal straight-line arithmetic function for integration testing.
define i32 @add_and_xor(i32 %a, i32 %b) {
entry:
  %sum = add i32 %a, %b
  %diff = sub i32 %a, %b
  %x = xor i32 %sum, %diff
  ret i32 %x
}
