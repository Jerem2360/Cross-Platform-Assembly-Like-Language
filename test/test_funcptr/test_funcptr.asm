bits 64
default rel
global funtable
global foo2
global function@namespace
global main
extern ExitProcess
extern puts
SECTION .data
foo0_msg:
    db "Hello from foo0!", 0
foo1_msg:
    db "Hello from foo1!", 0
foo2_msg:
    db "Hello from foo2!", 0
foo3_msg:
    db "Hello from foo3!", 0
general_msg:
    db "The message is:", 0
funtable:
    dq $foo0
    dq $foo1
    dq $foo2
    dq $foo3
SECTION .text
foo0:
; [PROLOG]
;   (push temp registers)
    PUSH R15; push
    PUSH RDI; push
    PUSH RSI; push
    SUB RSP, 8; push_amount
    MOVQ QWORD [RSP], XMM15; push
    SUB RSP, 8; push_amount
    MOVQ QWORD [RSP], XMM14; push
    SUB RSP, 8; push_amount
    MOVQ QWORD [RSP], XMM13; push
;   (push user boxes)
; [USER CODE]
;   (INSTR 'call'; line 28)
;     { PUSH LOCALS }
;     { ALIGN stack to 16 }
    SUB RSP, 8; push_amount
;     { PUSH SHADOW SPACE (if applicable) }
    SUB RSP, 32; push_amount
;     { WRITE REG ARGS }
    MOV RCX, $foo0_msg; move
;     { WRITE STACK ARGS }
;     { CALL }
    CALL $puts; call
;     { POP STACK ARGS }
;     { POP SHADOW SPACE (if applicable) }
    ADD RSP, 32; pop_amount
;     { POP ALIGN PADDING }
    ADD RSP, 8; pop_amount
;     { FETCH RESULT } 
;     { POP LOCALS }
; [EPILOG]
.__epilog@cf5f190:
;   (POP TMP+LOCALS)
    MOVQ XMM13, QWORD [RSP]; pop
    ADD RSP, 8; pop_amount
    MOVQ XMM14, QWORD [RSP]; pop
    ADD RSP, 8; pop_amount
    MOVQ XMM15, QWORD [RSP]; pop
    ADD RSP, 8; pop_amount
    POP RSI; pop
    POP RDI; pop
    POP R15; pop
;   ( RETURN )
    RET ; return
foo1:
; [PROLOG]
;   (push temp registers)
    PUSH R15; push
    PUSH RDI; push
    PUSH RSI; push
    SUB RSP, 8; push_amount
    MOVQ QWORD [RSP], XMM15; push
    SUB RSP, 8; push_amount
    MOVQ QWORD [RSP], XMM14; push
    SUB RSP, 8; push_amount
    MOVQ QWORD [RSP], XMM13; push
;   (push user boxes)
; [USER CODE]
;   (INSTR 'call'; line 31)
;     { PUSH LOCALS }
;     { ALIGN stack to 16 }
    SUB RSP, 8; push_amount
;     { PUSH SHADOW SPACE (if applicable) }
    SUB RSP, 32; push_amount
;     { WRITE REG ARGS }
    MOV RCX, $foo1_msg; move
;     { WRITE STACK ARGS }
;     { CALL }
    CALL $puts; call
;     { POP STACK ARGS }
;     { POP SHADOW SPACE (if applicable) }
    ADD RSP, 32; pop_amount
;     { POP ALIGN PADDING }
    ADD RSP, 8; pop_amount
;     { FETCH RESULT } 
;     { POP LOCALS }
; [EPILOG]
.__epilog@cf5f191:
;   (POP TMP+LOCALS)
    MOVQ XMM13, QWORD [RSP]; pop
    ADD RSP, 8; pop_amount
    MOVQ XMM14, QWORD [RSP]; pop
    ADD RSP, 8; pop_amount
    MOVQ XMM15, QWORD [RSP]; pop
    ADD RSP, 8; pop_amount
    POP RSI; pop
    POP RDI; pop
    POP R15; pop
;   ( RETURN )
    RET ; return
foo2:
; [PROLOG]
;   (push temp registers)
    PUSH R15; push
    PUSH RDI; push
    PUSH RSI; push
    SUB RSP, 8; push_amount
    MOVQ QWORD [RSP], XMM15; push
    SUB RSP, 8; push_amount
    MOVQ QWORD [RSP], XMM14; push
    SUB RSP, 8; push_amount
    MOVQ QWORD [RSP], XMM13; push
;   (push user boxes)
; [USER CODE]
;   (INSTR 'call'; line 34)
;     { PUSH LOCALS }
;     { ALIGN stack to 16 }
    SUB RSP, 8; push_amount
;     { PUSH SHADOW SPACE (if applicable) }
    SUB RSP, 32; push_amount
;     { WRITE REG ARGS }
    MOV RCX, $foo2_msg; move
;     { WRITE STACK ARGS }
;     { CALL }
    CALL $puts; call
;     { POP STACK ARGS }
;     { POP SHADOW SPACE (if applicable) }
    ADD RSP, 32; pop_amount
;     { POP ALIGN PADDING }
    ADD RSP, 8; pop_amount
;     { FETCH RESULT } 
;     { POP LOCALS }
; [EPILOG]
.__epilog@cf5f192:
;   (POP TMP+LOCALS)
    MOVQ XMM13, QWORD [RSP]; pop
    ADD RSP, 8; pop_amount
    MOVQ XMM14, QWORD [RSP]; pop
    ADD RSP, 8; pop_amount
    MOVQ XMM15, QWORD [RSP]; pop
    ADD RSP, 8; pop_amount
    POP RSI; pop
    POP RDI; pop
    POP R15; pop
;   ( RETURN )
    RET ; return
foo3:
; [PROLOG]
;   (push temp registers)
    PUSH R15; push
    PUSH RDI; push
    PUSH RSI; push
    SUB RSP, 8; push_amount
    MOVQ QWORD [RSP], XMM15; push
    SUB RSP, 8; push_amount
    MOVQ QWORD [RSP], XMM14; push
    SUB RSP, 8; push_amount
    MOVQ QWORD [RSP], XMM13; push
;   (push user boxes)
; [USER CODE]
;   (INSTR 'call'; line 37)
;     { PUSH LOCALS }
;     { ALIGN stack to 16 }
    SUB RSP, 8; push_amount
;     { PUSH SHADOW SPACE (if applicable) }
    SUB RSP, 32; push_amount
;     { WRITE REG ARGS }
    MOV RCX, $foo3_msg; move
;     { WRITE STACK ARGS }
;     { CALL }
    CALL $puts; call
;     { POP STACK ARGS }
;     { POP SHADOW SPACE (if applicable) }
    ADD RSP, 32; pop_amount
;     { POP ALIGN PADDING }
    ADD RSP, 8; pop_amount
;     { FETCH RESULT } 
;     { POP LOCALS }
; [EPILOG]
.__epilog@cf5f193:
;   (POP TMP+LOCALS)
    MOVQ XMM13, QWORD [RSP]; pop
    ADD RSP, 8; pop_amount
    MOVQ XMM14, QWORD [RSP]; pop
    ADD RSP, 8; pop_amount
    MOVQ XMM15, QWORD [RSP]; pop
    ADD RSP, 8; pop_amount
    POP RSI; pop
    POP RDI; pop
    POP R15; pop
;   ( RETURN )
    RET ; return
function@namespace:
; [PROLOG]
;   (push temp registers)
    PUSH R15; push
    PUSH RDI; push
    PUSH RSI; push
    SUB RSP, 8; push_amount
    MOVQ QWORD [RSP], XMM15; push
    SUB RSP, 8; push_amount
    MOVQ QWORD [RSP], XMM14; push
    SUB RSP, 8; push_amount
    MOVQ QWORD [RSP], XMM13; push
;   (push user boxes)
; [USER CODE]
;   (INSTR 'call'; line 41)
;     { PUSH LOCALS }
;     -> arg: OK
    PUSH RCX; push
;     { ALIGN stack to 16 }
;     { PUSH SHADOW SPACE (if applicable) }
    SUB RSP, 32; push_amount
;     { WRITE REG ARGS }
;     { WRITE STACK ARGS }
;     { CALL }
    CALL $puts; call
;     { POP STACK ARGS }
;     { POP SHADOW SPACE (if applicable) }
    ADD RSP, 32; pop_amount
;     { POP ALIGN PADDING }
;     { FETCH RESULT } 
;     { POP LOCALS }
    POP RCX; pop
; [EPILOG]
.__epilog@cf5f194:
;   (POP TMP+LOCALS)
    MOVQ XMM13, QWORD [RSP]; pop
    ADD RSP, 8; pop_amount
    MOVQ XMM14, QWORD [RSP]; pop
    ADD RSP, 8; pop_amount
    MOVQ XMM15, QWORD [RSP]; pop
    ADD RSP, 8; pop_amount
    POP RSI; pop
    POP RDI; pop
    POP R15; pop
;   ( RETURN )
    RET ; return
main:
; [PROLOG]
;   (push temp registers)
    PUSH R15; push
    PUSH RDI; push
    PUSH RSI; push
    SUB RSP, 8; push_amount
    MOVQ QWORD [RSP], XMM15; push
    SUB RSP, 8; push_amount
    MOVQ QWORD [RSP], XMM14; push
    SUB RSP, 8; push_amount
    MOVQ QWORD [RSP], XMM13; push
;   (push user boxes)
; [USER CODE]
;   (INSTR '<-'; line 49)
    MOV R15, $funtable; move
    MOV RAX, QWORD [R15 + 24]; move
;   (INSTR 'call'; line 51)
;     { PUSH LOCALS }
;     -> func: OK
    PUSH RAX; push
;     { ALIGN stack to 16 }
;     { PUSH SHADOW SPACE (if applicable) }
    SUB RSP, 32; push_amount
;     { WRITE REG ARGS }
    MOV RCX, $general_msg; move
;     { WRITE STACK ARGS }
;     { CALL }
    CALL $function@namespace; call
;     { POP STACK ARGS }
;     { POP SHADOW SPACE (if applicable) }
    ADD RSP, 32; pop_amount
;     { POP ALIGN PADDING }
;     { FETCH RESULT } 
;     { POP LOCALS }
    POP RAX; pop
;   (INSTR 'call'; line 53)
;     { PUSH LOCALS }
;     -> func: OK
    PUSH RAX; push
;     { ALIGN stack to 16 }
;     { PUSH SHADOW SPACE (if applicable) }
    SUB RSP, 32; push_amount
;     { WRITE REG ARGS }
;     { WRITE STACK ARGS }
;     { CALL }
    CALL RAX; call
;     { POP STACK ARGS }
;     { POP SHADOW SPACE (if applicable) }
    ADD RSP, 32; pop_amount
;     { POP ALIGN PADDING }
;     { FETCH RESULT } 
;     { POP LOCALS }
    POP RAX; pop
    push RAX; #push
    pop RAX; #pop
; [EPILOG]
.__epilog@cf5f195:
;   ( EXIT )
;     { PUSH LOCALS }
;     -> func: OK
    PUSH RAX; push
;     { ALIGN stack to 16 }
;     { PUSH SHADOW SPACE (if applicable) }
    SUB RSP, 32; push_amount
;     { WRITE REG ARGS }
    MOV RCX, 0; move
;     { WRITE STACK ARGS }
;     { CALL }
    CALL $ExitProcess; call
