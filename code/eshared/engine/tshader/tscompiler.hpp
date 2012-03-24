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

#ifndef TS_COMPILER_HPP
#define TS_COMPILER_HPP

#define TSHADER_COMPILER_ARG_CACHE_SIZE 8

class eTShaderCompiler
{
private:
    struct Argument
    {
        enum Type
        {
            TYPE_UNUSED,
            TYPE_SCALAR,
            TYPE_VARIABLE
        };

        Type                    type;
        eU8                     index;
        eS8                     component;
        eF32                    number;
    };

    struct Instruction
    {
        eString                 name;
        eU32                    numArgs;
        eBool                   singleChannel;
    };

    struct Label
    {
        eString                 name;
        eU32                    pos;
    };

    typedef eArray<Label *> LabelPtrArray;
    typedef eArray<Argument> ArgumentArray;

public:
    eTShaderCompiler();
    ~eTShaderCompiler();

    eBool                       compile(const eString &source);
    void                        getCode(eByteArray &out) const;
    const eString &             getErrors() const;

public:
    static eU32                 getInstructionCount();
    static const eString &      getInstructionName(eU32 index);
    
private:
    eBool                       _compilePass(const eString &source);
    eBool                       _compileLine(const eString &line);
    eBool                       _compileArgument(const eString &arg, Argument &sa, eU32 &channels, eBool isTargetArg);
    eBool                       _findInstruction(const eString &instrName, eU32 &index);

    void                        _movArgToRegister(eMachineCodeGen::Register reg, Argument arg);
    void                        _movRegisterToArg(eMachineCodeGen::Register reg, Argument arg);
    void                        _writeInstruction(eU8 inst, eBool singleChannel, const ArgumentArray &args);

    eBool                       _isNumeric(const eString &str) const;
    eF32                        _strToFloat(const eString &str) const;

    void                        _setError(const eString &msg);

	void						_argCacheReset();
	eMachineCodeGen::Register   _argCacheGetRegToUse(Argument arg);
	void						_argCacheUpdate(eMachineCodeGen::Register reg, Argument arg);

public:
    static const Instruction    INSTRUCTIONS[];

private:
    eMachineCodeGen             m_gen;
    eU32                        m_dataOffset;
    eString                     m_error;
    eU32                        m_lineNum;
	Argument					m_argsInReg[TSHADER_COMPILER_ARG_CACHE_SIZE];
};

#endif // TS_COMPILER_HPP