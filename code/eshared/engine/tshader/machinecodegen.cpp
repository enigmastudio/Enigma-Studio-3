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

#include "../../system/system.hpp"
#include "machinecodegen.hpp"

#define MOD_REGISTER_ADDRESSING_MODE 0xC0
#define MOD_REGISTER_INDIRECT_ADRESSING 0x00
#define MOD_DISPLACEMENT_ONLY_ADRESSING 0x05
#define MOD_ONE_BYTE_DISPLACEMENT 0x40
#define MOD_FOUR_BYTE_DISPLACEMENT 0x80

#define OPCODE_EXTENSION(x) (((eU8)x)<<3)
#define REG(x) (((eU8)x)<<3)
#define RM(x) ((eU8)x)

eMachineCodeGen::eMachineCodeGen()
{
    init();
}

void eMachineCodeGen::init()
{
    m_machineCode.clear();
    m_data.clear();
}

void eMachineCodeGen::data(eF32 val)
{
    _pushData32(*(eU32*)&val);
}

void eMachineCodeGen::_push8(eU8 v)
{
    m_machineCode.append(v);
}

void eMachineCodeGen::_push2x8(eU8 v1, eU8 v2)
{
    _push8(v1);
    _push8(v2);
}

void eMachineCodeGen::_push3x8(eU8 v1, eU8 v2, eU8 v3)
{
    _push8(v1);
    _push8(v2);
    _push8(v3);
}

void eMachineCodeGen::_push32(eU32 v)
{
    eU8* p = (eU8*)&v;
    _push8(p[0]);
    _push8(p[1]);
    _push8(p[2]);
    _push8(p[3]);
}

void eMachineCodeGen::_pushData8(eU8 v)
{
    m_data.append(v);
}

void eMachineCodeGen::_pushData32(eU32 v)
{
    eU8* p = (eU8*)&v;
    _pushData8(p[0]);
    _pushData8(p[1]);
    _pushData8(p[2]);
    _pushData8(p[3]);
}

void eMachineCodeGen::getCode(eByteArray &out) const
{
    eU32 dataSize = m_data.size();
    eU8* p = (eU8*)&dataSize;
    out.append(p[0]);
    out.append(p[1]);
    out.append(p[2]);
    out.append(p[3]);
    out.append(m_data);
    out.append(m_machineCode);
}

// ############################################################################
//  STANDARD INSTRUCTIONS
// ############################################################################

void eMachineCodeGen::mov(Register reg, eU32 value)
{
    _push8(0xb8 + (eU8)reg);    // mov + reg
    _push32(value);
}

void eMachineCodeGen::ret()
{
    _push8(0xc3);
}

void eMachineCodeGen::int3()
{
    _push8(0xcc);
}

// ############################################################################
//  FPU INSTRUCTIONS
// ############################################################################

void eMachineCodeGen::fld(Register reg1, eS32 displacement)
{
    _push8(0xd9);
    _push8(MOD_FOUR_BYTE_DISPLACEMENT + OPCODE_EXTENSION(0) + RM(reg1));
    _push32(displacement);
}

void eMachineCodeGen::fstp(Register reg1, eS32 displacement)
{
    _push8(0xd9);
    _push8(MOD_FOUR_BYTE_DISPLACEMENT + OPCODE_EXTENSION(3) + RM(reg1));
    _push32(displacement);
}

void eMachineCodeGen::fsin()
{
    _push8(0xd9);
    _push8(0xfe);
}

void eMachineCodeGen::fcos()
{
    _push8(0xd9);
    _push8(0xff);
}

// ############################################################################
//  SSE INSTRUCTIONS
// ############################################################################

void eMachineCodeGen::movss(Register reg1, Register reg2, eS32 displacement)
{
    _push3x8(0xf3, 0x0f, 0x10);   
    _push8(MOD_FOUR_BYTE_DISPLACEMENT + REG(reg1) + RM(reg2));  
    _push32(displacement);
}

void eMachineCodeGen::movss(Register reg1, eS32 displacement, Register reg2)
{
    _push3x8(0xf3, 0x0f, 0x11);  
    _push8(MOD_FOUR_BYTE_DISPLACEMENT + REG(reg2) + RM(reg1));  
    _push32(displacement);
}

void eMachineCodeGen::movaps(Register reg1, Register reg2, eS32 displacement)
{
    _push2x8(0x0f, 0x28);  
    _push8(MOD_FOUR_BYTE_DISPLACEMENT + REG(reg1) + RM(reg2));  
    _push32(displacement);
}

void eMachineCodeGen::movaps(Register reg1, eS32 displacement, Register reg2)
{
    _push2x8(0x0f, 0x29);   
    _push8(MOD_FOUR_BYTE_DISPLACEMENT + REG(reg2) + RM(reg1));  
    _push32(displacement);
}

void eMachineCodeGen::movdqa(Register reg1, Register reg2)
{
    _push3x8(0x66, 0x0f, 0x6f);  
    _push8(MOD_REGISTER_ADDRESSING_MODE + REG(reg1) + RM(reg2));  
}

void eMachineCodeGen::addps(Register reg1, Register reg2)
{
    _push2x8(0x0f, 0x58);  
    _push8(MOD_REGISTER_ADDRESSING_MODE + REG(reg1) + RM(reg2));  
}

void eMachineCodeGen::subps(Register reg1, Register reg2)
{
    _push2x8(0x0f, 0x5c);  
    _push8(MOD_REGISTER_ADDRESSING_MODE + REG(reg1) + RM(reg2));  
}

void eMachineCodeGen::mulps(Register reg1, Register reg2)
{
    _push2x8(0x0f, 0x59);  
    _push8(MOD_REGISTER_ADDRESSING_MODE + REG(reg1) + RM(reg2));  
}

void eMachineCodeGen::divps(Register reg1, Register reg2)
{
    _push2x8(0x0f, 0x5e);  
    _push8(MOD_REGISTER_ADDRESSING_MODE + REG(reg1) + RM(reg2));  
}

void eMachineCodeGen::minps(Register reg1, Register reg2)
{
    _push2x8(0x0f, 0x5d);  
    _push8(MOD_REGISTER_ADDRESSING_MODE + REG(reg1) + RM(reg2));  
}

void eMachineCodeGen::maxps(Register reg1, Register reg2)
{
    _push2x8(0x0f, 0x5f);  
    _push8(MOD_REGISTER_ADDRESSING_MODE + REG(reg1) + RM(reg2));  
}

void eMachineCodeGen::pshufd(Register reg1, Register reg2, eU8 shuffle)
{
    _push3x8(0x66, 0x0f, 0x70);  
    _push8(MOD_REGISTER_ADDRESSING_MODE + REG(reg1) + RM(reg2)); 
    _push8(shuffle);
}



