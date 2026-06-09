bits 64
default rel
global main
extern ExitProcess
extern puts
extern print_int
SECTION .text
add:
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
;   (INSTR '<-'; line 8)
    MOV AX, 0; move
;   (INSTR '+='; line 9)
    ADD AX, CX; add
;   (INSTR '+='; line 10)
    ADD AX, DX; add
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
;   (INSTR '<-'; line 17)
    MOV EAX, 15; move
;   (INSTR 'call'; line 18)
;     { PUSH LOCALS }
;     -> value: SKIP; this is the return location
;     { ALIGN stack to 16 }
    SUB RSP, 8; push_amount
;     { PUSH SHADOW SPACE (if applicable) }
    SUB RSP, 32; push_amount
;     { WRITE REG ARGS }
    MOV ECX, EAX; move
    MOV RDX, 10; move
;     { WRITE STACK ARGS }
;     { CALL }
    CALL $add; call
;     { POP STACK ARGS }
;     { POP SHADOW SPACE (if applicable) }
    ADD RSP, 32; pop_amount
;     { POP ALIGN PADDING }
    ADD RSP, 8; pop_amount
;     { FETCH RESULT } 
;     { POP LOCALS }
;   (INSTR 'call'; line 20)
;     { PUSH LOCALS }
;     -> value: OK
    PUSH RAX; push
;     { ALIGN stack to 16 }
;     { PUSH SHADOW SPACE (if applicable) }
    SUB RSP, 32; push_amount
;     { WRITE REG ARGS }
    MOV ECX, EAX; move
;     { WRITE STACK ARGS }
;     { CALL }
    CALL $print_int; call
;     { POP STACK ARGS }
;     { POP SHADOW SPACE (if applicable) }
    ADD RSP, 32; pop_amount
;     { POP ALIGN PADDING }
;     { FETCH RESULT } 
;     { POP LOCALS }
    POP RAX; pop
; [EPILOG]
.__epilog@cf5f191:
;   ( EXIT )
;     { PUSH LOCALS }
;     -> value: OK
    PUSH RAX; push
;     { ALIGN stack to 16 }
;     { PUSH SHADOW SPACE (if applicable) }
    SUB RSP, 32; push_amount
;     { WRITE REG ARGS }
    MOV RCX, 0; move
;     { WRITE STACK ARGS }
;     { CALL }
    CALL $ExitProcess; call
