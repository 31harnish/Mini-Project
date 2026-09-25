; ModuleID = 'high_risk_test'
; A function named "encrypt_block" to trigger sensitivity detection
; (keyword "encrypt" in feature_extraction.cpp).
;
; Designed with ~10 basic blocks and cyclomatic complexity ~10 to push
; risk_score into the intensity-4 range (>= 0.7) under balanced profile.
;
; Scoring estimate:
;   sensitivity bonus                  = 0.40
;   basic_block_count (10)  * 0.01     = 0.10
;   cyclomatic_complexity (10) * 0.02  = 0.20
;   instruction_diversity (~8) * 0.005 = 0.04
;   total raw ≈ 0.74 → intensity 4
;
; C-like pseudocode:
;   int encrypt_block(int key, int data, int rounds) {
;       int state = data ^ key;
;       for (int i = 0; i < rounds; i++) {
;           if (state > 100) {
;               state = state - 50;
;               if (state & 1) {
;                   state = state ^ key;
;               } else {
;                   state = state + key;
;               }
;           } else {
;               state = state * 3 + 1;
;               if (i & 1) {
;                   state = state ^ 0xFF;
;               }
;           }
;       }
;       return state;
;   }

define i32 @encrypt_block(i32 %key, i32 %data, i32 %rounds) {
entry:
  %state.init = xor i32 %data, %key
  br label %loop.header

loop.header:                                      ; preds = %merge, %entry
  %state = phi i32 [ %state.init, %entry ], [ %state.next, %merge ]
  %i     = phi i32 [ 0, %entry ],            [ %i.next, %merge ]
  %cmp   = icmp slt i32 %i, %rounds
  br i1 %cmp, label %loop.body, label %exit

loop.body:                                        ; preds = %loop.header
  %cmp.high = icmp sgt i32 %state, 100
  br i1 %cmp.high, label %high.path, label %low.path

high.path:                                        ; preds = %loop.body
  %sub50 = sub i32 %state, 50
  %bit0  = and i32 %sub50, 1
  %is.odd = icmp ne i32 %bit0, 0
  br i1 %is.odd, label %high.odd, label %high.even

high.odd:                                         ; preds = %high.path
  %xor.key = xor i32 %sub50, %key
  br label %high.merge

high.even:                                        ; preds = %high.path
  %add.key = add i32 %sub50, %key
  br label %high.merge

high.merge:                                       ; preds = %high.even, %high.odd
  %high.result = phi i32 [ %xor.key, %high.odd ], [ %add.key, %high.even ]
  br label %merge

low.path:                                         ; preds = %loop.body
  %mul3   = mul i32 %state, 3
  %plus1  = add i32 %mul3, 1
  %i.bit  = and i32 %i, 1
  %i.odd  = icmp ne i32 %i.bit, 0
  br i1 %i.odd, label %low.xor, label %low.done

low.xor:                                          ; preds = %low.path
  %xor.ff = xor i32 %plus1, 255
  br label %low.done

low.done:                                         ; preds = %low.xor, %low.path
  %low.result = phi i32 [ %xor.ff, %low.xor ], [ %plus1, %low.path ]
  br label %merge

merge:                                            ; preds = %low.done, %high.merge
  %state.next = phi i32 [ %high.result, %high.merge ], [ %low.result, %low.done ]
  %i.next     = add i32 %i, 1
  br label %loop.header

exit:                                             ; preds = %loop.header
  ret i32 %state
}
