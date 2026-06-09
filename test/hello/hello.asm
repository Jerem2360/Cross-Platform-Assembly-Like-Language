bits 64
default rel
global main
extern ExitProcess
extern puts
SECTION .data
msg_hello:
    db "Hello World!", 13, 10, 0
label_text:
    db "The message is:", 13, 10, 0
SECTION .text
print_msg:
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
;   (INSTR 'call'; line 15)
;     { PUSH LOCALS }
;     -> msg: OK
    PUSH RCX; push
;     { ALIGN stack to 16 }
;     { PUSH SHADOW SPACE (if applicable) }
    SUB RSP, 32; push_amount
;     { WRITE REG ARGS }
    MOV RCX, $label_text; move
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
;   (INSTR 'call'; line 16)
;     { PUSH LOCALS }
;     -> msg: OK
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
;   (INSTR 'call'; line 21)
;     { PUSH LOCALS }
;     { ALIGN stack to 16 }
    SUB RSP, 8; push_amount
;     { PUSH SHADOW SPACE (if applicable) }
    SUB RSP, 32; push_amount
;     { WRITE REG ARGS }
    MOV RCX, $msg_hello; move
;     { WRITE STACK ARGS }
;     { CALL }
    CALL $print_msg; call
;     { POP STACK ARGS }
;     { POP SHADOW SPACE (if applicable) }
    ADD RSP, 32; pop_amount
;     { POP ALIGN PADDING }
    ADD RSP, 8; pop_amount
;     { FETCH RESULT } 
;     { POP LOCALS }
; [EPILOG]
.__epilog@cf5f191:
;   ( EXIT )
;     { PUSH LOCALS }
;     { ALIGN stack to 16 }
    SUB RSP, 8; push_amount
;     { PUSH SHADOW SPACE (if applicable) }
    SUB RSP, 32; push_amount
;     { WRITE REG ARGS }
    MOV RCX, 0; move
;     { WRITE STACK ARGS }
;     { CALL }
    CALL $ExitProcess; call
