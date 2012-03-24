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
#include "../../math/math.hpp"

#include "tshader.hpp"

#include <windows.h>

eTShader::eTShader() :
    m_machineCode(eNULL)
{

}

eTShader::~eTShader()
{
    free();
}

void eTShader::load(const eByteArray &code)
{
    DWORD oldprotect;
    free();
    eU32 codeSize = code.size();
    if (codeSize > 4)
    {
        m_machineCode = VirtualAlloc(NULL, codeSize, MEM_COMMIT, PAGE_READWRITE);
        eMemCopy(m_machineCode, code.m_data, codeSize);
        VirtualProtect(m_machineCode, codeSize, PAGE_EXECUTE, &oldprotect);
    }
}

void eTShader::free()
{
    if (m_machineCode)
    {
        VirtualFree(m_machineCode, 0, MEM_RELEASE);
        m_machineCode = eNULL;
    }
}

void eTShader::run(eConstPtr callerId, eVector4 &out)
{
    ePROFILER_ZONE("Run shader script");

    if (!m_machineCode)
        return;

    State &tss = _getStateForCaller(callerId);

	// ------------------------------------------------------------
	//	Get pointers to machine code and data section from array
	// ------------------------------------------------------------
    eU32 dataSize = *(eU32*)m_machineCode;
    ePtr code = ((eU8*)m_machineCode) + dataSize + 4;
    ePtr data = ((eU8*)m_machineCode) + 4;
	   
	// ------------------------------------------------------------
	//  Copy variable and register contents to __m128 array
	// ------------------------------------------------------------
	eU32 mvarsize = sizeof(__m128) * (State::REGS_COUNT + State::VARS_COUNT);
	__m128 *mvars = (__m128*)eMemAllocAlignedAndZero(mvarsize, 16);

    for(eU32 i=0; i<State::REGS_COUNT; i++)
	{
		eVector4 &v = tss.regs[i];
        mvars[i] = _mm_set_ps(v.x, v.y, v.z, v.w);
	}

    for(eU32 i=0; i<State::VARS_COUNT; i++)
	{
		eVector4 &v = tss.vars[i];
        mvars[State::REGS_COUNT + i] = _mm_set_ps(v.x, v.y, v.z, v.w);
	}

	// ------------------------------------------------------------
	//	call shader
	// ------------------------------------------------------------
    __asm 
    {
        push eax
        push edx
        mov eax, mvars
        mov edx, dword ptr data
        call dword ptr code;
        pop edx
        pop eax
    }

	// ------------------------------------------------------------
	//	copy results from __m128 array back to variables
	// ------------------------------------------------------------
    for(eU32 i=0;i<State::VARS_COUNT;i++)
	{
		eF32A store_out[4];
		_mm_store_ps(store_out, mvars[State::REGS_COUNT + i]);   
		eVector4 &v = tss.vars[i];
		v.x = store_out[3];
		v.y = store_out[2];
		v.z = store_out[1];
		v.w = store_out[0];
	}

	eFreeAligned(mvars);

    // v0 is result
    out = tss.vars[0];
}

void eTShader::setTime(eConstPtr callerId, eF32 time)
{
    eASSERT(time >= 0.0f);

    State &tss = _getStateForCaller(callerId);
    const eF32 oldTime = tss.regs[0].x;

    tss.regs[0] = eVector4(time);
    tss.regs[1] = eVector4(time-oldTime);
}

void eTShader::setAudio(eConstPtr callerId, eU32 channel, eF32 value)
{
    if (channel < State::AUDIO_CHANNELS)
    {
        State &tss = _getStateForCaller(callerId);
        tss.regs[channel+2] = eVector4(value);
    }
}

eTShader::State & eTShader::_getStateForCaller(eConstPtr callerId)
{
    // Try to find an existing shader state.
    for (eU32 i=0; i<m_states.size(); i++)
    {
        if (m_states[i].callerId == callerId)
        {
            return m_states[i];
        }
    }

    // Not found => create and append new shader state.
    State tss;

    eMemSet(&tss.vars, 0, sizeof(tss.vars));
    eMemSet(&tss.regs, 0, sizeof(tss.regs));
    tss.callerId = callerId;
    tss.ts = this;

    m_states.append(tss);
    return m_states[m_states.size()-1];
}

