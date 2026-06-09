bits 64
default rel
global print_int
global print_float
extern ExitProcess
extern puts
extern _itoa
extern _gcvt_s
SECTION .bss
buffer:
    resb 64
SECTION .text
print_int:
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
;   (INSTR 'call'; line 30)
;     { PUSH LOCALS }
;     -> val: OK
    PUSH RCX; push
;     { ALIGN stack to 16 }
;     { PUSH SHADOW SPACE (if applicable) }
    SUB RSP, 32; push_amount
;     { WRITE REG ARGS }
    MOV RDX, $buffer; move
    MOV R8, 10; move
;     { WRITE STACK ARGS }
;     { CALL }
    CALL $_itoa; call
;     { POP STACK ARGS }
;     { POP SHADOW SPACE (if applicable) }
    ADD RSP, 32; pop_amount
;     { POP ALIGN PADDING }
;     { FETCH RESULT } 
;     { POP LOCALS }
    POP RCX; pop
;   (INSTR 'call'; line 31)
;     { PUSH LOCALS }
;     -> val: OK
    PUSH RCX; push
;     { ALIGN stack to 16 }
;     { PUSH SHADOW SPACE (if applicable) }
    SUB RSP, 32; push_amount
;     { WRITE REG ARGS }
    MOV RCX, $buffer; move
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
print_float:
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
;   (INSTR 'call'; line 35)
;     { PUSH LOCALS }
;     -> val: OK
    SUB RSP, 8; push_amount
    MOVQ QWORD [RSP], XMM0; push
;     { ALIGN stack to 16 }
;     { PUSH SHADOW SPACE (if applicable) }
    SUB RSP, 32; push_amount
;     { WRITE REG ARGS }
    MOV RCX, $buffer; move
    MOV RDX, 64; move
    MOVSD XMM2, XMM0; move
    MOV R9, 20; move
;     { WRITE STACK ARGS }
;     { CALL }
    CALL $_gcvt_s; call
;     { POP STACK ARGS }
;     { POP SHADOW SPACE (if applicable) }
    ADD RSP, 32; pop_amount
;     { POP ALIGN PADDING }
;     { FETCH RESULT } 
;     { POP LOCALS }
    MOVQ XMM0, QWORD [RSP]; pop
    ADD RSP, 8; pop_amount
;   (INSTR 'call'; line 36)
;     { PUSH LOCALS }
;     -> val: OK
    SUB RSP, 8; push_amount
    MOVQ QWORD [RSP], XMM0; push
;     { ALIGN stack to 16 }
;     { PUSH SHADOW SPACE (if applicable) }
    SUB RSP, 32; push_amount
;     { WRITE REG ARGS }
    MOV RCX, $buffer; move
;     { WRITE STACK ARGS }
;     { CALL }
    CALL $puts; call
;     { POP STACK ARGS }
;     { POP SHADOW SPACE (if applicable) }
    ADD RSP, 32; pop_amount
;     { POP ALIGN PADDING }
;     { FETCH RESULT } 
;     { POP LOCALS }
    MOVQ XMM0, QWORD [RSP]; pop
    ADD RSP, 8; pop_amount
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
