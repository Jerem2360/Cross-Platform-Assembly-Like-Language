bits 64
default rel
global main
extern ExitProcess
extern _printf
SECTION .data
    db "This is AMD64!"
input:
    dd 12
output:
    resd 1
string:
    db "Output is %i."
SECTION .text
fibo2:
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
;     {PUSH 'i'}
    PUSH RBX; push
; [USER CODE]
;   (INSTR 'if goto'; line 51)
    CMP ECX, 2; jump_if: compare
    JA .body; jump_if
;   (INSTR '<-'; line 52)
    MOV EAX, 2; move
;   (INSTR 'if goto'; line 53)
    CMP ECX, 2; jump_if: compare
    JE .end; jump_if
;   (INSTR '<-'; line 54)
    MOV EAX, 1; move
;   (INSTR 'goto'; line 55)
    JMP .end; jump
.body:
;   (INSTR '<-'; line 59)
    MOV EBX, 2; move
;   (INSTR '<-'; line 60)
    MOV EAX, 2; move
;   (INSTR '<-'; line 61)
    MOV R8D, 1; move
.loop:
;   (INSTR 'if goto'; line 64)
    CMP EBX, ECX; jump_if: compare
    JGE .end; jump_if
;   (INSTR '++'; line 65)
    INC EBX; inc
;   (INSTR '<-'; line 67)
    MOV EDX, R8D; move
;   (INSTR '<-'; line 68)
    MOV R8D, EAX; move
;   (INSTR '+='; line 69)
    ADD EAX, EDX; add
;   (INSTR 'goto'; line 71)
    JMP .loop; jump
.end:
;   (INSTR 'return'; line 74)
    JMP $.__epilog@cf5f190; jump
; [EPILOG]
.__epilog@cf5f190:
;   (POP TMP+LOCALS)
    POP RBX; pop
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
fibo:
__func@cf5f191:
; [USER CODE]
;   (INSTR '<-'; line 87)
    MOV EAX, DWORD [$input]; move
;   (INSTR 'if goto'; line 89)
    CMP EAX, 2; jump_if: compare
    JA .body; jump_if
;   (INSTR '<-'; line 90)
    MOV EBX, 2; move
;   (INSTR 'if goto'; line 91)
    CMP EAX, 2; jump_if: compare
    JE .end; jump_if
;   (INSTR '<-'; line 92)
    MOV EBX, 1; move
;   (INSTR 'goto'; line 93)
    JMP .end; jump
.body:
;   (INSTR '<-'; line 97)
    MOV ECX, 2; move
;   (INSTR '<-'; line 98)
    MOV EBX, 2; move
;   (INSTR '<-'; line 99)
    MOV R8D, 1; move
.loop:
;   (INSTR 'if goto'; line 102)
    CMP ECX, EAX; jump_if: compare
    JGE .end; jump_if
;   (INSTR '++'; line 103)
    INC ECX; inc
;   (INSTR '<-'; line 105)
    MOV EDX, R8D; move
;   (INSTR '<-'; line 106)
    MOV R8D, EBX; move
;   (INSTR '+='; line 107)
    ADD EBX, EDX; add
;   (INSTR 'goto'; line 109)
    JMP .loop; jump
.end:
;   (INSTR '<-'; line 112)
    MOV DWORD [$output], EBX; move
;   (INSTR 'return'; line 113)
    RET ; return
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
;   (INSTR 'call'; line 120)
;     { PUSH LOCALS }
;     { CALL }
    CALL $fibo; call
;   (INSTR '<-'; line 121)
    MOV EAX, DWORD [$output]; move
;   (INSTR 'call'; line 122)
;     { PUSH LOCALS }
;     -> fibo_res: OK
    PUSH RAX; push
;     { ALIGN stack to 16 }
;     { PUSH SHADOW SPACE (if applicable) }
    SUB RSP, 32; push_amount
;     { WRITE REG ARGS }
    MOV RCX, $string; move
    MOV EDX, EAX; move
;     { WRITE STACK ARGS }
;     { CALL }
    CALL $_printf; call
;     { POP STACK ARGS }
;     { POP SHADOW SPACE (if applicable) }
    ADD RSP, 32; pop_amount
;     { POP ALIGN PADDING }
;     { FETCH RESULT } 
;     { POP LOCALS }
    POP RAX; pop
;   (INSTR 'call'; line 124)
;     { PUSH LOCALS }
;     -> fibo_res: SKIP; this is the return location
;     { ALIGN stack to 16 }
    SUB RSP, 8; push_amount
;     { PUSH SHADOW SPACE (if applicable) }
    SUB RSP, 32; push_amount
;     { WRITE REG ARGS }
    MOV RCX, 12; move
;     { WRITE STACK ARGS }
;     { CALL }
    CALL $fibo2; call
;     { POP STACK ARGS }
;     { POP SHADOW SPACE (if applicable) }
    ADD RSP, 32; pop_amount
;     { POP ALIGN PADDING }
    ADD RSP, 8; pop_amount
;     { FETCH RESULT } 
;     { POP LOCALS }
;   (INSTR 'call'; line 125)
;     { PUSH LOCALS }
;     -> fibo_res: OK
    PUSH RAX; push
;     { ALIGN stack to 16 }
;     { PUSH SHADOW SPACE (if applicable) }
    SUB RSP, 32; push_amount
;     { WRITE REG ARGS }
    MOV RCX, $string; move
    MOV EDX, EAX; move
;     { WRITE STACK ARGS }
;     { CALL }
    CALL $_printf; call
;     { POP STACK ARGS }
;     { POP SHADOW SPACE (if applicable) }
    ADD RSP, 32; pop_amount
;     { POP ALIGN PADDING }
;     { FETCH RESULT } 
;     { POP LOCALS }
    POP RAX; pop
;   (INSTR 'exit'; line 126)
;     { PUSH LOCALS }
;     -> fibo_res: OK
    PUSH RAX; push
;     { ALIGN stack to 16 }
;     { PUSH SHADOW SPACE (if applicable) }
    SUB RSP, 32; push_amount
;     { WRITE REG ARGS }
    MOV RCX, 0; move
;     { WRITE STACK ARGS }
;     { CALL }
    CALL $ExitProcess; call
; [EPILOG]
.__epilog@cf5f193:
;   ( EXIT )
;     { PUSH LOCALS }
;     -> fibo_res: OK
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
