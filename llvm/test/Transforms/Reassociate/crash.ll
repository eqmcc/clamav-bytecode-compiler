; RUN: opt -reassociate -disable-output %s


; rdar://7507855
define fastcc i32 @test1() nounwind {
entry:
  %cond = select i1 undef, i32 1, i32 -1          ; <i32> [#uses=2]
  br label %for.cond

for.cond:                                         ; preds = %for.body, %entry
  %sub889 = sub i32 undef, undef                  ; <i32> [#uses=1]
  %sub891 = sub i32 %sub889, %cond                ; <i32> [#uses=0]
  %add896 = sub i32 0, %cond                      ; <i32> [#uses=0]
  ret i32 undef
}

; PR5981
define i32 @test2() nounwind ssp {
entry:
  %0 = load i32* undef, align 4
  %1 = mul nsw i32 undef, %0
  %2 = mul nsw i32 undef, %0
  %3 = add nsw i32 undef, %1
  %4 = add nsw i32 %3, %2
  %5 = add nsw i32 %4, 4
  %6 = shl i32 %0, 3
  %7 = add nsw i32 %5, %6
  br label %bb4.i9

bb4.i9:
  %8 = add nsw i32 undef, %1
  ret i32 0
}


define i32 @test3(i32 %Arg, i32 %x1, i32 %x2, i32 %x3) {
 %A = mul i32 %x1, %Arg
 %B = mul i32 %Arg, %x2 ;; Part of add operation being factored, also used by C
 %C = mul i32 %x3, %B

 %D = add i32 %A, %B
 %E = add i32 %D, %C
  ret i32 %E
}
