#include "../architecture.hpp"
#include "../register_macros.hpp"


namespace cpasm::x86_64 {
    
    // available in 16-bit mode and above
    _DECL_REG(AL);
    _DECL_REG(AH);
    _DECL_REG(BL);
    _DECL_REG(BH);
    _DECL_REG(CL);
    _DECL_REG(CH);
    _DECL_REG(DL);
    _DECL_REG(DH);

    _DECL_REG(AX);
    _DECL_REG(BX);
    _DECL_REG(CX);
    _DECL_REG(DX);
    _DECL_REG(SI);
    _DECL_REG(DI);
    _DECL_REG(SP);
    _DECL_REG(BP);
    
    _DECL_REG(CS);
    _DECL_REG(DS);
    _DECL_REG(SS);
    _DECL_REG(ES);
    _DECL_REG(FS);
    _DECL_REG(GS);

    _DECL_REG(FLAGS);

    // available in 32-bit mode and above
    _DECL_REG(EAX);
    _DECL_REG(EBX);
    _DECL_REG(ECX);
    _DECL_REG(EDX);
    _DECL_REG(ESI);
    _DECL_REG(EDI);
    _DECL_REG(ESP);
    _DECL_REG(EBP);

    _DECL_REG(XMM0);
    _DECL_REG(XMM1);
    _DECL_REG(XMM2);
    _DECL_REG(XMM3);
    _DECL_REG(XMM4);
    _DECL_REG(XMM5);
    _DECL_REG(XMM6);
    _DECL_REG(XMM7);
    
    _DECL_REG(EFLAGS);

    _DECL_REG(CR0);
    _DECL_REG(CR2);
    _DECL_REG(CR3);
    _DECL_REG(CR4);

    // available in 64-bit mode
    _DECL_REG(R8B);
    _DECL_REG(R9B);
    _DECL_REG(R10B);
    _DECL_REG(R11B);
    _DECL_REG(R12B);
    _DECL_REG(R13B);
    _DECL_REG(R14B);
    _DECL_REG(R15B);

    _DECL_REG(R8W);
    _DECL_REG(R9W);
    _DECL_REG(R10W);
    _DECL_REG(R11W);
    _DECL_REG(R12W);
    _DECL_REG(R13W);
    _DECL_REG(R14W);
    _DECL_REG(R15W);
    
    _DECL_REG(R8D);
    _DECL_REG(R9D);
    _DECL_REG(R10D);
    _DECL_REG(R11D);
    _DECL_REG(R12D);
    _DECL_REG(R13D);
    _DECL_REG(R14D);
    _DECL_REG(R15D);

    _DECL_REG(RAX);
    _DECL_REG(RBX);
    _DECL_REG(RCX);
    _DECL_REG(RDX);
    _DECL_REG(RSI);
    _DECL_REG(RDI);
    _DECL_REG(R8);
    _DECL_REG(R9);
    _DECL_REG(R10);
    _DECL_REG(R11);
    _DECL_REG(R12);
    _DECL_REG(R13);
    _DECL_REG(R14);
    _DECL_REG(R15);
    _DECL_REG(RSP);
    _DECL_REG(RBP);
    
    _DECL_REG(XMM8);
    _DECL_REG(XMM9);
    _DECL_REG(XMM10);
    _DECL_REG(XMM11);
    _DECL_REG(XMM12);
    _DECL_REG(XMM13);
    _DECL_REG(XMM14);
    _DECL_REG(XMM15);
    
    _DECL_REG(RFLAGS);
    
    _DECL_REG(CR8);

    static constexpr CPURegisterFlags FL_GENERAL_PURPOSE = CPURegisterFlags(true, false, true);
    static constexpr CPURegisterFlags FL_MISC = CPURegisterFlags(false, false, true);
    static constexpr CPURegisterFlags FL_SSE = CPURegisterFlags(false, true, true);
    static constexpr CPURegisterFlags FL_KERNEL = CPURegisterFlags(false, false, false);

    _DEF_REG(RAX, 8, FL_GENERAL_PURPOSE, REG_NONE, EAX, REG_NONE); 
    _DEF_REG(RBX, 8, FL_GENERAL_PURPOSE, REG_NONE, EBX, REG_NONE); 
    _DEF_REG(RCX, 8, FL_GENERAL_PURPOSE, REG_NONE, ECX, REG_NONE); 
    _DEF_REG(RDX, 8, FL_GENERAL_PURPOSE, REG_NONE, EDX, REG_NONE); 
    _DEF_REG(RSI, 8, FL_GENERAL_PURPOSE, REG_NONE, ESI, REG_NONE);
    _DEF_REG(RDI, 8, FL_GENERAL_PURPOSE, REG_NONE, EDI, REG_NONE);
    _DEF_REG(R8, 8, FL_GENERAL_PURPOSE, REG_NONE, R8D, REG_NONE);
    _DEF_REG(R9, 8, FL_GENERAL_PURPOSE, REG_NONE, R9W, REG_NONE);
    _DEF_REG(R10, 8, FL_GENERAL_PURPOSE, REG_NONE, R10D, REG_NONE);
    _DEF_REG(R11, 8, FL_GENERAL_PURPOSE, REG_NONE, R11D, REG_NONE);
    _DEF_REG(R12, 8, FL_GENERAL_PURPOSE, REG_NONE, R12D, REG_NONE);
    _DEF_REG(R13, 8, FL_GENERAL_PURPOSE, REG_NONE, R13D, REG_NONE);
    _DEF_REG(R14, 8, FL_GENERAL_PURPOSE, REG_NONE, R14D, REG_NONE);
    _DEF_REG(R15, 8, FL_GENERAL_PURPOSE, REG_NONE, R15D, REG_NONE);
    _DEF_REG(RSP, 8, FL_MISC, REG_NONE, ESP, REG_NONE);
    _DEF_REG(RBP, 8, FL_MISC, REG_NONE, EBP, REG_NONE);
    _DEF_REG(RFLAGS, 8, FL_MISC, REG_NONE, EFLAGS, REG_NONE);
    
    _DEF_REG(EAX, 4, FL_GENERAL_PURPOSE, RAX, AX, REG_NONE);
    _DEF_REG(EBX, 4, FL_GENERAL_PURPOSE, RBX, BX, REG_NONE);
    _DEF_REG(ECX, 4, FL_GENERAL_PURPOSE, RCX, CX, REG_NONE);
    _DEF_REG(EDX, 4, FL_GENERAL_PURPOSE, RDX, DX, REG_NONE);
    _DEF_REG(ESI, 4, FL_GENERAL_PURPOSE, RSI, SI, REG_NONE);
    _DEF_REG(EDI, 4, FL_GENERAL_PURPOSE, RDI, DI, REG_NONE);
    _DEF_REG(R8D, 4, FL_GENERAL_PURPOSE, R8, R8W, REG_NONE);
    _DEF_REG(R9D, 4, FL_GENERAL_PURPOSE, R9, R9W, REG_NONE);
    _DEF_REG(R10D, 4, FL_GENERAL_PURPOSE, R10, R10W, REG_NONE);
    _DEF_REG(R11D, 4, FL_GENERAL_PURPOSE, R11, R11W, REG_NONE);
    _DEF_REG(R12D, 4, FL_GENERAL_PURPOSE, R12, R12W, REG_NONE);
    _DEF_REG(R13D, 4, FL_GENERAL_PURPOSE, R13, R13W, REG_NONE);
    _DEF_REG(R14D, 4, FL_GENERAL_PURPOSE, R14, R14W, REG_NONE);
    _DEF_REG(R15D, 4, FL_GENERAL_PURPOSE, R15, R15W, REG_NONE);
    _DEF_REG(ESP, 4, FL_MISC, RSP, SP, REG_NONE);
    _DEF_REG(EBP, 4, FL_MISC, RBP, BP, REG_NONE);
    _DEF_REG(EFLAGS, 4, FL_MISC, RFLAGS, FLAGS, REG_NONE);

    _DEF_REG(AX, 2, FL_GENERAL_PURPOSE, EAX, AL, AH);
    _DEF_REG(BX, 2, FL_GENERAL_PURPOSE, EBX, BL, BH);
    _DEF_REG(CX, 2, FL_GENERAL_PURPOSE, ECX, CL, CH);
    _DEF_REG(DX, 2, FL_GENERAL_PURPOSE, EDX, DL, DH);
    _DEF_REG(SI, 2, FL_GENERAL_PURPOSE, ESI, REG_NONE, REG_NONE);
    _DEF_REG(DI, 2, FL_GENERAL_PURPOSE, EDI, REG_NONE, REG_NONE);
    _DEF_REG(R8W, 2, FL_GENERAL_PURPOSE, R8D, R8B, REG_NONE);
    _DEF_REG(R9W, 2, FL_GENERAL_PURPOSE, R9D, R9B, REG_NONE);
    _DEF_REG(R10W, 2, FL_GENERAL_PURPOSE, R10D, R10B, REG_NONE);
    _DEF_REG(R11W, 2, FL_GENERAL_PURPOSE, R11D, R11B, REG_NONE);
    _DEF_REG(R12W, 2, FL_GENERAL_PURPOSE, R12D, R12B, REG_NONE);
    _DEF_REG(R13W, 2, FL_GENERAL_PURPOSE, R13D, R13B, REG_NONE);
    _DEF_REG(R14W, 2, FL_GENERAL_PURPOSE, R14D, R14B, REG_NONE);
    _DEF_REG(R15W, 2, FL_GENERAL_PURPOSE, R15D, R15B, REG_NONE);
    _DEF_REG(SP, 2, FL_MISC, ESP, REG_NONE, REG_NONE);
    _DEF_REG(BP, 2, FL_MISC, EBP, REG_NONE, REG_NONE);
    _DEF_REG(FLAGS, 2, FL_MISC, EFLAGS, REG_NONE, REG_NONE);
    
    _DEF_REG(CS, 2, FL_MISC, REG_NONE, REG_NONE, REG_NONE);
    _DEF_REG(DS, 2, FL_MISC, REG_NONE, REG_NONE, REG_NONE);
    _DEF_REG(SS, 2, FL_MISC, REG_NONE, REG_NONE, REG_NONE);
    _DEF_REG(ES, 2, FL_MISC, REG_NONE, REG_NONE, REG_NONE);
    _DEF_REG(FS, 2, FL_MISC, REG_NONE, REG_NONE, REG_NONE);
    _DEF_REG(GS, 2, FL_MISC, REG_NONE, REG_NONE, REG_NONE);
    
    _DEF_REG(AL, 1, FL_GENERAL_PURPOSE, AX, REG_NONE, REG_NONE);
    _DEF_REG(BL, 1, FL_GENERAL_PURPOSE, BX, REG_NONE, REG_NONE);
    _DEF_REG(CL, 1, FL_GENERAL_PURPOSE, CX, REG_NONE, REG_NONE);
    _DEF_REG(DL, 1, FL_GENERAL_PURPOSE, DX, REG_NONE, REG_NONE);
    _DEF_REG(R8B, 1, FL_GENERAL_PURPOSE, R8W, REG_NONE, REG_NONE);
    _DEF_REG(R9B, 1, FL_GENERAL_PURPOSE, R9W, REG_NONE, REG_NONE);
    _DEF_REG(R10B, 1, FL_GENERAL_PURPOSE, R10W, REG_NONE, REG_NONE);
    _DEF_REG(R11B, 1, FL_GENERAL_PURPOSE, R11W, REG_NONE, REG_NONE);
    _DEF_REG(R12B, 1, FL_GENERAL_PURPOSE, R12W, REG_NONE, REG_NONE);
    _DEF_REG(R13B, 1, FL_GENERAL_PURPOSE, R13W, REG_NONE, REG_NONE);
    _DEF_REG(R14B, 1, FL_GENERAL_PURPOSE, R14W, REG_NONE, REG_NONE);
    _DEF_REG(R15B, 1, FL_GENERAL_PURPOSE, R15W, REG_NONE, REG_NONE);
    
    _DEF_REG(AH, 1, FL_GENERAL_PURPOSE, AX, REG_NONE, REG_NONE);
    _DEF_REG(BH, 1, FL_GENERAL_PURPOSE, BX, REG_NONE, REG_NONE);
    _DEF_REG(CH, 1, FL_GENERAL_PURPOSE, CX, REG_NONE, REG_NONE);
    _DEF_REG(DH, 1, FL_GENERAL_PURPOSE, DX, REG_NONE, REG_NONE);

    _DEF_REG(CR0, 4, FL_KERNEL, REG_NONE, REG_NONE, REG_NONE);
    _DEF_REG(CR2, 4, FL_KERNEL, REG_NONE, REG_NONE, REG_NONE);
    _DEF_REG(CR3, 4, FL_KERNEL, REG_NONE, REG_NONE, REG_NONE);
    _DEF_REG(CR4, 4, FL_KERNEL, REG_NONE, REG_NONE, REG_NONE);
    _DEF_REG(CR8, 8, FL_KERNEL, REG_NONE, REG_NONE, REG_NONE);

    _DEF_REG(XMM0, 16, FL_SSE, REG_NONE, REG_NONE, REG_NONE);
    _DEF_REG(XMM1, 16, FL_SSE, REG_NONE, REG_NONE, REG_NONE);
    _DEF_REG(XMM2, 16, FL_SSE, REG_NONE, REG_NONE, REG_NONE);
    _DEF_REG(XMM3, 16, FL_SSE, REG_NONE, REG_NONE, REG_NONE);
    _DEF_REG(XMM4, 16, FL_SSE, REG_NONE, REG_NONE, REG_NONE);
    _DEF_REG(XMM5, 16, FL_SSE, REG_NONE, REG_NONE, REG_NONE);
    _DEF_REG(XMM6, 16, FL_SSE, REG_NONE, REG_NONE, REG_NONE);
    _DEF_REG(XMM7, 16, FL_SSE, REG_NONE, REG_NONE, REG_NONE);
    _DEF_REG(XMM8, 16, FL_SSE, REG_NONE, REG_NONE, REG_NONE);
    _DEF_REG(XMM9, 16, FL_SSE, REG_NONE, REG_NONE, REG_NONE);
    _DEF_REG(XMM10, 16, FL_SSE, REG_NONE, REG_NONE, REG_NONE);
    _DEF_REG(XMM11, 16, FL_SSE, REG_NONE, REG_NONE, REG_NONE);
    _DEF_REG(XMM12, 16, FL_SSE, REG_NONE, REG_NONE, REG_NONE);
    _DEF_REG(XMM13, 16, FL_SSE, REG_NONE, REG_NONE, REG_NONE);
    _DEF_REG(XMM14, 16, FL_SSE, REG_NONE, REG_NONE, REG_NONE);
    _DEF_REG(XMM15, 16, FL_SSE, REG_NONE, REG_NONE, REG_NONE);


    static const CPURegister* _16bit_registers[] = {
        _REG(AL),
        _REG(AH),
        _REG(BL),
        _REG(BH),
        _REG(CL),
        _REG(CH),
        _REG(DL),
        _REG(DH),
        _REG(AX),
        _REG(BX),
        _REG(CX),
        _REG(DX),
        _REG(SI),
        _REG(DI),
        _REG(SP),
        _REG(BP),
        _REG(CS),
        _REG(DS),
        _REG(SS),
        _REG(ES),
        _REG(FS),
        _REG(GS),
        _REG(FLAGS),
    };

    static const CPURegister* _32bit_registers[] = {
        _REG(AL),
        _REG(AH),
        _REG(BL),
        _REG(BH),
        _REG(CL),
        _REG(CH),
        _REG(DL),
        _REG(DH),
        _REG(AX),
        _REG(BX),
        _REG(CX),
        _REG(DX),
        _REG(SI),
        _REG(DI),
        _REG(SP),
        _REG(BP),
        _REG(CS),
        _REG(DS),
        _REG(SS),
        _REG(ES),
        _REG(FS),
        _REG(GS),
        _REG(FLAGS),

        _REG(EAX),
        _REG(EBX),
        _REG(ECX),
        _REG(EDX),
        _REG(ESI),
        _REG(EDI),
        _REG(ESP),
        _REG(EBP),
        _REG(XMM0),
        _REG(XMM1),
        _REG(XMM2),
        _REG(XMM3),
        _REG(XMM4),
        _REG(XMM5),
        _REG(XMM6),
        _REG(XMM7),
        _REG(EFLAGS),
        _REG(CR0),
        _REG(CR2),
        _REG(CR3),
        _REG(CR4),
    };

    
    static const CPURegister* _64bit_registers[] = {
        _REG(AL),
        _REG(AH),
        _REG(BL),
        _REG(BH),
        _REG(CL),
        _REG(CH),
        _REG(DL),
        _REG(DH),
        _REG(AX),
        _REG(BX),
        _REG(CX),
        _REG(DX),
        _REG(SI),
        _REG(DI),
        _REG(SP),
        _REG(BP),
        _REG(CS),
        _REG(DS),
        _REG(SS),
        _REG(ES),
        _REG(FS),
        _REG(GS),
        _REG(FLAGS),
        
        _REG(EAX),
        _REG(EBX),
        _REG(ECX),
        _REG(EDX),
        _REG(ESI),
        _REG(EDI),
        _REG(ESP),
        _REG(EBP),
        _REG(XMM0),
        _REG(XMM1),
        _REG(XMM2),
        _REG(XMM3),
        _REG(XMM4),
        _REG(XMM5),
        _REG(XMM6),
        _REG(XMM7),
        _REG(EFLAGS),
        _REG(CR0),
        _REG(CR2),
        _REG(CR3),
        _REG(CR4),

        _REG(R8B),
        _REG(R9B),
        _REG(R10B),
        _REG(R11B),
        _REG(R12B),
        _REG(R13B),
        _REG(R14B),
        _REG(R15B),
        _REG(R8W),
        _REG(R9W),
        _REG(R10W),
        _REG(R11W),
        _REG(R12W),
        _REG(R13W),
        _REG(R14W),
        _REG(R15W),
        _REG(R8D),
        _REG(R9D),
        _REG(R10D),
        _REG(R11D),
        _REG(R12D),
        _REG(R13D),
        _REG(R14D),
        _REG(R15D),
        _REG(RAX),
        _REG(RBX),
        _REG(RCX),
        _REG(RDX),
        _REG(RSI),
        _REG(RDI),
        _REG(R8),
        _REG(R9),
        _REG(R10),
        _REG(R11),
        _REG(R12),
        _REG(R13),
        _REG(R14),
        _REG(R15),
        _REG(RSP),
        _REG(RBP),
        _REG(XMM8),
        _REG(XMM9),
        _REG(XMM10),
        _REG(XMM11),
        _REG(XMM12),
        _REG(XMM13),
        _REG(XMM14),
        _REG(XMM15),
        _REG(RFLAGS),
        _REG(CR8),
    };


    array_view<const CPURegister*> _available_registers(u8 bitness) {
        switch (bitness) {
        case 2:
            return _16bit_registers;
        case 4:
            return _32bit_registers;
        case 8:
            return _64bit_registers;
        default:
            return {};
        }
    }
}

