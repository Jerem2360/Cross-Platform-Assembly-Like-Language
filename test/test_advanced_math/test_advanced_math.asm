bits 64
default rel
global main
extern ExitProcess
extern print_float
extern print_int
SECTION .text
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
;   (INSTR '<-'; line 11)
    MOV EAX, 100; move
;   (INSTR '/='; line 12)
    MOV R15, RDX; move
    MOV RDX, 0; move
    MOV RDI, 10; move
    IDIV EDI; div
    MOV RDX, R15; move
;   (INSTR '*='; line 13)
    IMUL EAX, EAX, 10; mul
;   (INSTR 'call'; line 15)
;     { PUSH LOCALS }
;     -> varf: OK
    SUB RSP, 8; push_amount
    MOVQ QWORD [RSP], XMM0; push
;     -> varf: OK    -> vari: OK
    PUSH RAX; push
;     { ALIGN stack to 16 }
    SUB RSP, 8; push_amount
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
    ADD RSP, 8; pop_amount
;     { FETCH RESULT } 
;     { POP LOCALS }
    POP RAX; pop
    MOVQ XMM0, QWORD [RSP]; pop
    ADD RSP, 8; pop_amount
;   (INSTR '<-'; line 17)
    MOV R15, 4636761920415098470; move
    MOVQ XMM0, R15; move
;   (INSTR '/='; line 18)
    MOV R15, 4621819117588971520; move
    MOVQ XMM15, R15; move
    DIVSD XMM0, XMM15; div
;   (INSTR '*='; line 19)
    MOV R15, 4621819117588971520; move
    MOVQ XMM15, R15; move
    MULSD XMM0, XMM15; mul
;   (INSTR 'call'; line 23)
;     { PUSH LOCALS }
;     -> varf: OK
    SUB RSP, 8; push_amount
    MOVQ QWORD [RSP], XMM0; push
;     -> varf: OK    -> vari: OK
    PUSH RAX; push
;     { ALIGN stack to 16 }
    SUB RSP, 8; push_amount
;     { PUSH SHADOW SPACE (if applicable) }
    SUB RSP, 32; push_amount
;     { WRITE REG ARGS }
;     { WRITE STACK ARGS }
;     { CALL }
    CALL $print_float; call
;     { POP STACK ARGS }
;     { POP SHADOW SPACE (if applicable) }
    ADD RSP, 32; pop_amount
;     { POP ALIGN PADDING }
    ADD RSP, 8; pop_amount
;     { FETCH RESULT } 
;     { POP LOCALS }
    POP RAX; pop
    MOVQ XMM0, QWORD [RSP]; pop
    ADD RSP, 8; pop_amount
; [EPILOG]
.__epilog@cf5f190:
;   ( EXIT )
;     { PUSH LOCALS }
;     -> varf: OK
    SUB RSP, 8; push_amount
    MOVQ QWORD [RSP], XMM0; push
;     -> varf: OK    -> vari: OK
    PUSH RAX; push
;     { ALIGN stack to 16 }
    SUB RSP, 8; push_amount
;     { PUSH SHADOW SPACE (if applicable) }
    SUB RSP, 32; push_amount
;     { WRITE REG ARGS }
    MOV RCX, 0; move
;     { WRITE STACK ARGS }
;     { CALL }
    CALL $ExitProcess; call
