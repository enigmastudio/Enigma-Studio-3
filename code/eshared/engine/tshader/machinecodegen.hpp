/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *   This file is part of
 *       _______   ______________  ______     _____
 *      / ____/ | / /  _/ ____/  |/  /   |   |__  /
 *     / __/ /  |/ // // / __/ /|_/ / /| |    /_ <
 *    / /___/ /|  // // /_/ / /  / / ___ |  ___/ /
 *   /_____/_/ |_/___/\____/_/  /_/_/  |_| /____/.
 *
 *   Copyright © 2003-2010 Brain Control, all rights reserved.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef MACHINECODEGEN_HPP
#define MACHINECODEGEN_HPP

class eMachineCodeGen
{
public:
    enum Register
    {
        REG_EAX = 0,
        REG_ECX = 1,
        REG_EDX = 2,
        REG_EBX = 3,
        REG_ESP = 4,
        REG_EBP = 5,
        REG_ESI = 6,
        REG_EDI = 7,

        REG_XMM0 = 0,
        REG_XMM1 = 1,
        REG_XMM2 = 2,
        REG_XMM3 = 3,
        REG_XMM4 = 4,
        REG_XMM5 = 5,
        REG_XMM6 = 6,
        REG_XMM7 = 7,
    };

public:
    eMachineCodeGen();

    void                        data(eF32 val);

    // x86 standard instructions
    void                        mov(Register reg, eU32 value);
    void                        ret();
    void                        int3();

    // FPU instructions
    void                        fld(Register reg1, eS32 displacement);
    void                        fstp(Register reg1, eS32 displacement);
    void                        fsin();
    void                        fcos();

    // SSE instructions
    void                        movss(Register reg1, Register reg2, eS32 displacement);
    void                        movss(Register reg1, eS32 displacement, Register reg2);
    void                        movaps(Register reg1, Register reg2, eS32 displacement);
    void                        movaps(Register reg1, eS32 displacement, Register reg2);
    void                        movdqa(Register reg1, Register reg2);
    void                        addps(Register reg1, Register reg2);
    void                        subps(Register reg1, Register reg2);
    void                        mulps(Register reg1, Register reg2);
    void                        divps(Register reg1, Register reg2);
    void                        minps(Register reg1, Register reg2);
    void                        maxps(Register reg1, Register reg2);
    void                        pshufd(Register reg1, Register reg2, eU8 shuffle);
   
    void                        init();
    void                        getCode(eByteArray &out) const;
private:

    void                        _push8(eU8 v);
    void                        _push2x8(eU8 v1, eU8 v2);
    void                        _push3x8(eU8 v1, eU8 v2, eU8 v3);
    void                        _push32(eU32 v);

    void                        _pushData8(eU8 v);
    void                        _pushData32(eU32 v);

    eByteArray                  m_machineCode;
    eByteArray                  m_data;
};

#endif // MACHINECODEGEN_HPP