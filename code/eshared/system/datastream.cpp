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

#include "system.hpp"

eDataStream::eDataStream(eConstPtr mem, eU32 length) :
    m_isReading(eFALSE),
    m_readIndex(0),
    m_bitCount(0),
    m_curByte(0)
{
    attach(mem, length);
}

void eDataStream::attach(eConstPtr mem, eU32 length)
{
    if (mem && length)
    {
        m_data.reserve(length);
        eMemCopy(m_data.m_data, mem, length);
        m_data.m_size = length;
        m_isReading = eTRUE;
        m_bitCount = 8;
    }
}

void eDataStream::writeBit(eBool bit)
{
    eASSERT(m_isReading == eFALSE);

    eSetBit(m_curByte, m_bitCount, bit);
    m_bitCount++;

    if (m_bitCount == 8)
    {
        m_data.append(m_curByte);
        m_bitCount = 0;
        m_curByte = 0;
    }
}

void eDataStream::writeByte(eU8 byte)
{
	writeBits(byte, 8);
}

void eDataStream::writeWord(eU16 word)
{
	writeBits(word, 16);
}

void eDataStream::writeDword(eU32 dword)
{
	writeBits(dword, 32);
}

void eDataStream::writeBits(eU32 dword, eU32 bitCount)
{
    for (eU32 i=0; i<bitCount; i++)
    {
        writeBit(eGetBit(dword, i));
    }
}

// Writes value encoded using elias gamma codes.
void eDataStream::writeVbrDword(eU32 dword)
{
    eASSERT(dword < eU32_MAX);

    dword++; // 0 is not encodable.
    const eInt bitCount = eFtoL(eLog2((eF32)dword))+1; // Truncation here is crucial.

    for (eInt i=0; i<bitCount-1; i++)
    {
        writeBit(eFALSE);
    }

    for (eInt i=bitCount-1; i>=0; i--)
    {
        writeBit(eGetBit(dword, i));
    }
}

void eDataStream::writeString(const eString &str)
{
    const eU32 strLen = str.length();
    eASSERT(strLen <= eU16_MAX);

    writeWord(strLen);

    for (eU32 i=0; i<strLen; i++)
    {
        writeByte(str[i]);
    }
}

eBool eDataStream::readBit()
{
    eASSERT(m_isReading != eFALSE);
    eASSERT(m_readIndex <= m_data.size());

    if (m_bitCount == 8)
    {
        m_curByte = m_data[m_readIndex++];
        m_bitCount = 0;
    }

    return eGetBit(m_curByte, m_bitCount++);
}

eU32 eDataStream::readBits(eU32 bitCount)
{
    eU32 result = 0;

    for (eU32 i=0; i<bitCount; i++)
    {
        result |= (readBit()<<i);
    }

    return result;
}

eU8 eDataStream::readByte()
{
	return readBits(8);
}

eU16 eDataStream::readWord()
{
	return readBits(16);
}

eU32 eDataStream::readDword()
{
	return readBits(32);
}

// Reads integer encoded using elias gamma codes.
eU32 eDataStream::readVbrDword()
{
    eInt bitCount = 0;

    while (readBit() == eFALSE)
    {
        bitCount++;
    }

    eU32 dword = 0;

    for (eInt i=bitCount-1; i>=0; i--)
    {
        eSetBit(dword, i, readBit());
    }

    eSetBit(dword, bitCount, eTRUE);
	return dword-1; // Because of +1 while encoding.
}

eString eDataStream::readString()
{
    const eU32 strLen = readWord();
    eString str;

    for (eU32 i=0; i<strLen; i++)
    {
        str += (eChar)readByte();
    }

    return str;
}

void eDataStream::advance(eU32 offset) 
{
    m_readIndex += offset;
}

eByteArray eDataStream::getData() const
{
    eByteArray finalData = m_data;
    finalData.append(m_curByte);

    return finalData;
}

eU32 eDataStream::getReadIndex() const
{
    return m_readIndex;
}

eU32 eDataStream::getWriteIndex() const
{
    return m_data.size();
}