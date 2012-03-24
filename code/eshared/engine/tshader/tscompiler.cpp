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

#include <sstream>

#include "../../system/system.hpp"
#include "../../math/math.hpp"

#include "machinecodegen.hpp"
#include "tscompiler.hpp"

//#define TSHADER_DEBUG

#define DISPLACEMENT(index, component) ((index) * sizeof(eVector4) + (component) * sizeof(eF32))
#define DATA_DISPLACEMENT(offset) ((offset) * sizeof(eF32))

using namespace std;

const eTShaderCompiler::Instruction eTShaderCompiler::INSTRUCTIONS[] =
{
    { "mov",    2, eTRUE  },
    { "add",    2, eTRUE  },
    { "sub",    2, eTRUE  },
    { "mul",    2, eTRUE  },
    { "div",    2, eTRUE  },
    { "min",    2, eTRUE  },
    { "max",    2, eTRUE  },
    { "sin",    2, eTRUE  },
    { "cos",    2, eTRUE  },

    { "end",    0, eFALSE }
};

eTShaderCompiler::eTShaderCompiler() :
    m_lineNum(1),
    m_dataOffset(0)
{
}

eTShaderCompiler::~eTShaderCompiler()
{

}

eBool eTShaderCompiler::compile(const eString &source)
{
    // clear generator
    m_gen.init();

    if (source != "")
    {
		_argCacheReset();

        // for debugging if necessary
#ifdef TSHADER_DEBUG
        m_gen.int3();    
#endif

        if (_compilePass(source))
        {
            // generate return statement to jump back at the end of the execution
            m_gen.ret();

            return eTRUE;
        }

        // clear generator, since we have errors
        m_gen.init();
    }
    else
    {
        m_error = "No source to compile!";
    }

    return eFALSE;
}

void eTShaderCompiler::getCode(eByteArray &out) const
{
    m_gen.getCode(out);
}

const eString & eTShaderCompiler::getErrors() const
{
    return m_error;
}

eU32 eTShaderCompiler::getInstructionCount()
{
    return sizeof(INSTRUCTIONS)/sizeof(Instruction);
}

const eString & eTShaderCompiler::getInstructionName(eU32 index)
{
    eASSERT(index < getInstructionCount());
    return INSTRUCTIONS[index].name;
}

eBool eTShaderCompiler::_compilePass(const eString &source)
{
    const eChar *ptr = source;

    m_lineNum = 1;
    m_dataOffset = 0;

    while (*ptr)
    {
        eString line;

        // Read a line of code
        while (*ptr != 10 && *ptr != 13 && *ptr != 0 && *ptr != '/')
        {
            line += *ptr++;
        }

        // Skip comment if any.
        if (*ptr == '/')
        {
            // Check if second slash is present.
            if (*++ptr != '/')
            {
                _setError("Single slash found in line. Comments have to start with a double-slash!");
                return eFALSE;
            }

            // Skip the rest of the line.
            while (*ptr != 10 && *ptr != 13 && *ptr != 0)
            {
                ptr++;
            }
        }

        // Skip white spaces.
        while (*ptr == 10 || *ptr == 13)
        {
            ptr++;
        }

        // Compile current line.
        if (!_compileLine(line.simplified()))
        {
            return eFALSE;
        }

        m_lineNum++;
    }

    return eTRUE;
}

eBool eTShaderCompiler::_compileLine(const eString &line)
{
    const eChar *ptr = line;
    eASSERT(ptr != eNULL);

    // Is line empty?
    if (line == "")
    {
        return eTRUE;
    }

    // Read the command (only human readable
    // characters which come after character 33).
    eString element;

    while (*ptr >= 33) 
    { 
        element += *ptr++;  
    } 

    while (*ptr < 33 && *ptr != '\0')
    {
        ptr++;
    }

    eU32 instrIndex = 0;

    if (!_findInstruction(element, instrIndex))
    {
        return eFALSE;
    }

    const Instruction &instr = INSTRUCTIONS[instrIndex];
    eBool singleChannel = eTRUE;
    ArgumentArray args(instr.numArgs);
    eBool isTargetArg = eTRUE;

    for (eU32 i=0; i<instr.numArgs; i++)
    {
        element = "";

        // Read the argument.
        while (*ptr >= 33 && *ptr != ',' && *ptr != '\0') 
        { 
            element += *ptr++;  
        } 

        // Skip white spaces.
        while ((*ptr < 33 || *ptr == ',') && *ptr != '\0')
        {
            ptr++;
        }

        eU32 channels;

        if (!_compileArgument(element, args[i], channels, isTargetArg))
        {
            return eFALSE;
        }

        isTargetArg = eFALSE;

        if (channels == 1 && instr.singleChannel == eFALSE)
        {
            _setError("Arguments must not be single-channel!");
            return eFALSE;
        }

        if (channels != 1)
            singleChannel = eFALSE;
    }

    // Write command
    _writeInstruction(instrIndex, singleChannel, args);

    return eTRUE;
}

eBool eTShaderCompiler::_compileArgument(const eString &arg, Argument &sa, eU32 &channels, eBool isTargetArg)
{
    if (arg == "")
    {
        _setError("Argument is missing!");
        return eFALSE;
    }

    const eChar *argPtr = arg;

    if (_isNumeric(arg)) // Is argument a scalar?
    {
        if (isTargetArg)
        {
            _setError("Cannot use a scalar constant in the target argument!");
            return eFALSE;
        }

        sa.number = _strToFloat(arg);
        sa.type = Argument::TYPE_SCALAR;
        channels = 1;
        return eTRUE;
    }
    else if (argPtr[0] == 'v') // Is argument a variable?
    {
        argPtr++; // Remove 'v' character.

        // Split the argument into variable index and shuffle.
        eString varIndex, varComponent;

        while (*argPtr != '.' && *argPtr != '\0')
        {
            varIndex += *argPtr++;
        }

        if (varIndex.length() == 0 && *argPtr == '.')
        {
            _setError("Argument is invalid. Dot without component!");
            return eFALSE;
        }

        if (*argPtr == '.')
        {
            argPtr++;

            while (*argPtr != '\0')
            {
                varComponent += *argPtr++;
            }
            
            if (varComponent.length() != 1)
            {
                _setError("Component invalid. Must be 1 character!");
                return eFALSE;
            }

            eChar s = 0;
            switch(varComponent[0])
            {
            case 'x': sa.component = 0; break;
            case 'y': sa.component = 1; break;
            case 'z': sa.component = 2; break;
            case 'w': sa.component = 3; break;
            default:
                _setError("Component is invalid. Use characters x, y, z, w only!");
                return eFALSE;
            }
        }
        else
        {
            // no component selection
            sa.component = -1;
        }

        if (!_isNumeric(varIndex))
        {
            _setError("Variable has invalid index!");
            return eFALSE;
        }

        const eU32 vi = eStrToInt(varIndex);

        if (vi < 0 || vi > 31)
        {
            _setError("Variable has invalid index. Only v0-v31 supported!");
            return eFALSE;
        }

        sa.index = 35+vi; // variables start at index 35
        sa.type = Argument::TYPE_VARIABLE;

        return eTRUE;
    }
    else // The argument is a register 
    {
        sa.component = -1;

        if (arg == "time")
        {
            sa.index = 0;
            sa.type = Argument::TYPE_VARIABLE;
            channels = 1; 
            return eTRUE;
        }
        else if (arg == "time_rel")
        {
            sa.index = 1;
            sa.type = Argument::TYPE_VARIABLE;
            channels = 1; 
            return eTRUE;
        }
        else if (arg == "audio_peak")
        {
            sa.index = 2;
            sa.type = Argument::TYPE_VARIABLE;
            channels = 1; 
            return eTRUE;
        }
        else if (arg.equals("audio", 5))
        {
            argPtr += 5;

            if (eStrLength(argPtr) == 0)
            {
                _setError("Audio register is missing its index!");
                return eFALSE;
            }
            else if (!_isNumeric(argPtr))
            {
                _setError("Audio register has non-numeric index!");
                return eFALSE;
            }

            const eInt index = eStrToInt(argPtr);

            if (index < 0 || index > 31)
            {
                _setError("Audio register has invalid index. Only 0-31 allowed!");
                return eFALSE;
            }

            sa.index = 3+index;
            sa.type = Argument::TYPE_VARIABLE;
            channels = 1; 

            return eTRUE;
        }
    }

    _setError("Argument is invalid. It is neither numeric nor variable nor register!");
    return eFALSE;
}

void eTShaderCompiler::_movArgToRegister(eMachineCodeGen::Register reg, Argument arg)
{
    if (arg.type == Argument::TYPE_SCALAR)
    {
        m_gen.data(arg.number);
        m_gen.movss(reg, eMachineCodeGen::REG_EDX, DATA_DISPLACEMENT(m_dataOffset));
        m_gen.pshufd(reg, reg, 0);
        m_dataOffset++;
    }
    else
    {
        if (arg.component == -1)
        {
            m_gen.movaps(reg, eMachineCodeGen::REG_EAX, DISPLACEMENT(arg.index, 0));
        }
        else
        {
            m_gen.movss(reg, eMachineCodeGen::REG_EAX, DISPLACEMENT(arg.index, 3-arg.component));
        }
    }
}

void eTShaderCompiler::_movRegisterToArg(eMachineCodeGen::Register reg, Argument arg)
{
    if (arg.component == -1)
    {
        m_gen.movaps(eMachineCodeGen::REG_EAX, DISPLACEMENT(arg.index, 0), reg);
    }
    else
    {
        m_gen.movss(eMachineCodeGen::REG_EAX, DISPLACEMENT(arg.index, 3-arg.component), reg);
    }
}

void eTShaderCompiler::_argCacheReset()
{
	for(eU32 i=0; i<TSHADER_COMPILER_ARG_CACHE_SIZE; i++)
	{
		m_argsInReg[i].type = Argument::TYPE_UNUSED;
	}
}

eMachineCodeGen::Register eTShaderCompiler::_argCacheGetRegToUse(Argument arg)
{
	eMachineCodeGen::Register regToUse = eMachineCodeGen::REG_XMM0;

	for(eU32 i=0; i<TSHADER_COMPILER_ARG_CACHE_SIZE; i++)
	{
		if (m_argsInReg[i].type == Argument::TYPE_UNUSED)
		{
			m_argsInReg[i] = arg;
			regToUse = (eMachineCodeGen::Register)(eMachineCodeGen::REG_XMM0 + i);
			return regToUse;
		}
	}

	m_argsInReg[0] = arg;
	_movArgToRegister(regToUse, arg);
	return regToUse;
}

void eTShaderCompiler::_argCacheUpdate(eMachineCodeGen::Register reg, Argument arg)
{
	_movRegisterToArg(reg, arg);
	m_argsInReg[(eU32)reg] = arg;
}

void eTShaderCompiler::_writeInstruction(eU8 instr, eBool singleChannel, const ArgumentArray &args)
{
    switch (instr)
    {
    case 0: // mov
		{
			_movArgToRegister(eMachineCodeGen::REG_XMM0, args[1]);
			_movRegisterToArg(eMachineCodeGen::REG_XMM0, args[0]);
			//eMachineCodeGen::Register reg = _argCacheGetRegToUse(args[1]);
			//_argCacheUpdate(reg, args[0]);
		}
        break;
    case 1: // add
		{
			_movArgToRegister(eMachineCodeGen::REG_XMM0, args[0]);
			_movArgToRegister(eMachineCodeGen::REG_XMM1, args[1]);
			m_gen.addps(eMachineCodeGen::REG_XMM0, eMachineCodeGen::REG_XMM1);
			_movRegisterToArg(eMachineCodeGen::REG_XMM0, args[0]);
			//eMachineCodeGen::Register reg0 = _argCacheGetRegToUse(args[0]);
			//eMachineCodeGen::Register reg1 = _argCacheGetRegToUse(args[1]);
			//m_gen.addps(reg0, reg1);
			//_argCacheUpdate(reg0, args[0]);
		}
        break;
    case 2: // sub
		{
			_movArgToRegister(eMachineCodeGen::REG_XMM0, args[0]);
			_movArgToRegister(eMachineCodeGen::REG_XMM1, args[1]);
			m_gen.subps(eMachineCodeGen::REG_XMM0, eMachineCodeGen::REG_XMM1);
			_movRegisterToArg(eMachineCodeGen::REG_XMM0, args[0]);
			//eMachineCodeGen::Register reg0 = _argCacheGetRegToUse(args[0]);
			//eMachineCodeGen::Register reg1 = _argCacheGetRegToUse(args[1]);
			//m_gen.subps(reg0, reg1);
			//_argCacheUpdate(reg0, args[0]);
		}
        break;
    case 3: // mul
		{
			_movArgToRegister(eMachineCodeGen::REG_XMM0, args[0]);
			_movArgToRegister(eMachineCodeGen::REG_XMM1, args[1]);
			m_gen.mulps(eMachineCodeGen::REG_XMM0, eMachineCodeGen::REG_XMM1);
			_movRegisterToArg(eMachineCodeGen::REG_XMM0, args[0]);
			//eMachineCodeGen::Register reg0 = _argCacheGetRegToUse(args[0]);
			//eMachineCodeGen::Register reg1 = _argCacheGetRegToUse(args[1]);
			//m_gen.mulps(reg0, reg1);
			//_argCacheUpdate(reg0, args[0]);
		}
        break;
    case 4: // div
		{
			_movArgToRegister(eMachineCodeGen::REG_XMM0, args[0]);
			_movArgToRegister(eMachineCodeGen::REG_XMM1, args[1]);
			m_gen.divps(eMachineCodeGen::REG_XMM0, eMachineCodeGen::REG_XMM1);
			_movRegisterToArg(eMachineCodeGen::REG_XMM0, args[0]);
			//eMachineCodeGen::Register reg0 = _argCacheGetRegToUse(args[0]);
			//eMachineCodeGen::Register reg1 = _argCacheGetRegToUse(args[1]);
			//m_gen.divps(reg0, reg1);
			//_argCacheUpdate(reg0, args[0]);
		}
        break;
    case 5: // min
		{
			_movArgToRegister(eMachineCodeGen::REG_XMM0, args[0]);
			_movArgToRegister(eMachineCodeGen::REG_XMM1, args[1]);
			m_gen.minps(eMachineCodeGen::REG_XMM0, eMachineCodeGen::REG_XMM1);
			_movRegisterToArg(eMachineCodeGen::REG_XMM0, args[0]);
			//eMachineCodeGen::Register reg0 = _argCacheGetRegToUse(args[0]);
			//eMachineCodeGen::Register reg1 = _argCacheGetRegToUse(args[1]);
			//m_gen.minps(reg0, reg1);
			//_argCacheUpdate(reg0, args[0]);
		}
        break;
    case 6: // max
		{
			_movArgToRegister(eMachineCodeGen::REG_XMM0, args[0]);
			_movArgToRegister(eMachineCodeGen::REG_XMM1, args[1]);
			m_gen.maxps(eMachineCodeGen::REG_XMM0, eMachineCodeGen::REG_XMM1);
			_movRegisterToArg(eMachineCodeGen::REG_XMM0, args[0]);
			//eMachineCodeGen::Register reg0 = _argCacheGetRegToUse(args[0]);
			//eMachineCodeGen::Register reg1 = _argCacheGetRegToUse(args[1]);
			//m_gen.maxps(reg0, reg1);
			//_argCacheUpdate(reg0, args[0]);
		}
        break;
    case 7: // sin
		{
			_movArgToRegister(eMachineCodeGen::REG_XMM0, args[1]);
			_movRegisterToArg(eMachineCodeGen::REG_XMM0, args[0]);
			if (args[0].component == -1 || args[0].component == 0)
			{
				m_gen.fld(eMachineCodeGen::REG_EAX, DISPLACEMENT(args[0].index, 3));
				m_gen.fsin();
				m_gen.fstp(eMachineCodeGen::REG_EAX, DISPLACEMENT(args[0].index, 3));
			}
			if (args[0].component == -1 || args[0].component == 1)
			{
				m_gen.fld(eMachineCodeGen::REG_EAX, DISPLACEMENT(args[0].index, 2));
				m_gen.fsin();
				m_gen.fstp(eMachineCodeGen::REG_EAX, DISPLACEMENT(args[0].index, 2));
			}
			if (args[0].component == -1 || args[0].component == 2)
			{
				m_gen.fld(eMachineCodeGen::REG_EAX, DISPLACEMENT(args[0].index, 1));
				m_gen.fsin();
				m_gen.fstp(eMachineCodeGen::REG_EAX, DISPLACEMENT(args[0].index, 1));
			}
			if (args[0].component == -1 || args[0].component == 3)
			{
				m_gen.fld(eMachineCodeGen::REG_EAX, DISPLACEMENT(args[0].index, 0));
				m_gen.fsin();
				m_gen.fstp(eMachineCodeGen::REG_EAX, DISPLACEMENT(args[0].index, 0));
			}
		}
        break;
    case 8: // cos
		{
			_movArgToRegister(eMachineCodeGen::REG_XMM0, args[1]);
			_movRegisterToArg(eMachineCodeGen::REG_XMM0, args[0]);
			if (args[0].component == -1 || args[0].component == 0)
			{
				m_gen.fld(eMachineCodeGen::REG_EAX, DISPLACEMENT(args[0].index, 3));
				m_gen.fcos();
				m_gen.fstp(eMachineCodeGen::REG_EAX, DISPLACEMENT(args[0].index, 3));
			}
			if (args[0].component == -1 || args[0].component == 1)
			{
				m_gen.fld(eMachineCodeGen::REG_EAX, DISPLACEMENT(args[0].index, 2));
				m_gen.fcos();
				m_gen.fstp(eMachineCodeGen::REG_EAX, DISPLACEMENT(args[0].index, 2));
			}
			if (args[0].component == -1 || args[0].component == 2)
			{
				m_gen.fld(eMachineCodeGen::REG_EAX, DISPLACEMENT(args[0].index, 1));
				m_gen.fcos();
				m_gen.fstp(eMachineCodeGen::REG_EAX, DISPLACEMENT(args[0].index, 1));
			}
			if (args[0].component == -1 || args[0].component == 3)
			{
				m_gen.fld(eMachineCodeGen::REG_EAX, DISPLACEMENT(args[0].index, 0));
				m_gen.fcos();
				m_gen.fstp(eMachineCodeGen::REG_EAX, DISPLACEMENT(args[0].index, 0));
			}
		}
        break;
    }
}

eBool eTShaderCompiler::_findInstruction(const eString &instrName, eU32 &index)
{
    for (eU32 i=0; i<getInstructionCount(); i++)
    {
        if (instrName == INSTRUCTIONS[i].name)
        {
            index = i;
            return eTRUE;
        }
    }

    _setError(eString("Command '")+instrName+"' is unknown!");
    return eFALSE;
}

eBool eTShaderCompiler::_isNumeric(const eString &str) const
{
    istringstream ss((const eChar *)str);
    eF32 val = 0.0;

    if (ss >> val)
    {
        return eTRUE;
    }

    return eFALSE;
}

eF32 eTShaderCompiler::_strToFloat(const eString &str) const
{
    stringstream ss;
    eF32 val;

    ss << str;
    ss >> val;

    return val;
}

void eTShaderCompiler::_setError(const eString &msg)
{
    m_error = eString("Line ")+eIntToStr(m_lineNum)+": "+msg;
}