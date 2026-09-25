; ModuleID = 'test_loop'
; A function with a while-style loop (back-edge to loop header) for
; integration testing — stresses control-flow-flattening's entry-block
; handling and back-edge rewriting.
;
; C equivalent:
;   int sum_up_to(int n) {
;       int acc = 0;
;       int i = 0;
;       while (i < n) {
;           acc = acc + i;
;           i = i + 1;
;       }
;       return acc;
;   }
define i32 @sum_up_to(i32 %n) {
entry:
  br label %loop.header

loop.header:
  %acc = phi i32 [ 0, %entry ], [ %acc.next, %loop.body ]
  %i   = phi i32 [ 0, %entry ], [ %i.next,   %loop.body ]
  %cmp = icmp slt i32 %i, %n
  br i1 %cmp, label %loop.body, label %exit

loop.body:
  %acc.next = add i32 %acc, %i
  %i.next   = add i32 %i, 1
  br label %loop.header

exit:
  ret i32 %acc
}
