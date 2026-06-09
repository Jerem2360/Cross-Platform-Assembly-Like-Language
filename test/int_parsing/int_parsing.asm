bits 64
default rel
global main
extern ExitProcess
extern print_int
extern print_float
SECTION .data
integers:
    dd 9
    dd 342391
    dd 123456789
    dd 20281789
floats:
    dq 3.5
    dq 668.732
    dq 12345.7
    dq 9320.67
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
;   (INSTR '<-'; line 31)
    MOV RAX, 0; move
.loop1:
;   (INSTR 'if goto'; line 34)
    CMP RAX, 4; jump_if: compare
    JAE .end1; jump_if
;   (INSTR 'call'; line 35)
;     { PUSH LOCALS }
;     -> fval: OK
    SUB RSP, 8; push_amount
    MOVQ QWORD [RSP], XMM0; push
;     -> fval: OK    -> i: OK
    PUSH RAX; push
;     { ALIGN stack to 16 }
    SUB RSP, 8; push_amount
;     { PUSH SHADOW SPACE (if applicable) }
    SUB RSP, 32; push_amount
;     { WRITE REG ARGS }
    MOV R15, $integers; move
    MOV ECX, DWORD [R15 + RAX*4]; move
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
;   (INSTR '++'; line 37)
    INC RAX; inc
;   (INSTR 'goto'; line 38)
    JMP .loop1; jump
.end1:
;   (INSTR '<-'; line 40)
    MOV RAX, 0; move
.loop2:
;   (INSTR 'if goto'; line 43)
    CMP RAX, 4; jump_if: compare
    JAE .end2; jump_if
;   (INSTR '<-'; line 44)
    MOV R15, $floats; move
    MOVQ XMM0, QWORD [R15 + RAX*8]; move
;   (INSTR 'call'; line 45)
;     { PUSH LOCALS }
;     -> fval: OK
    SUB RSP, 8; push_amount
    MOVQ QWORD [RSP], XMM0; push
;     -> fval: OK    -> i: OK
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
;   (INSTR '++'; line 47)
    INC RAX; inc
;   (INSTR 'goto'; line 48)
    JMP .loop2; jump
.end2:
; [EPILOG]
.__epilog@cf5f190:
;   ( EXIT )
;     { PUSH LOCALS }
;     -> fval: OK
    SUB RSP, 8; push_amount
    MOVQ QWORD [RSP], XMM0; push
;     -> fval: OK    -> i: OK
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
