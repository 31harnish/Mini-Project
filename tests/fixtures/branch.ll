; ModuleID = 'test_branch'
; A function with a branch for integration testing.
define i32 @max(i32 %a, i32 %b) {
entry:
  %cmp = icmp sgt i32 %a, %b
  br i1 %cmp, label %then, label %else

then:
  br label %end

else:
  br label %end

end:
  %result = phi i32 [ %a, %then ], [ %b, %else ]
  ret i32 %result
}
