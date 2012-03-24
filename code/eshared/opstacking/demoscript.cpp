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

#include "../eshared.hpp"

#ifdef eEDITOR
#include <Windows.h>
#include <stdio.h>
#endif

eDemoScript::eDemoScript(eConstPtr script, eU32 length)
{
#ifdef eEDITOR
    m_byteLength = 0;
#endif

    if (script && length)
    {
        eDataStream stream(script, length);

        for (eU32 i=0; i<STREAM_COUNT; i++)
        {
            const eU32 length = stream.readDword();
            m_streams[i].attach((eU8 *)script+stream.getReadIndex(), length);
            stream.advance(length);
        }
    }
}

#ifdef eEDITOR
void eDemoScript::addUsedOp(const eIOperator *op)
{
    eASSERT(op != eNULL);

    for (eU32 i=0; i<m_usedOps.size(); i++)
    {
        if (m_usedOps[i]->getRealType() == op->getRealType())
        {
            return;
        }
    }

    m_usedOps.append(op);
}

eString eDemoScript::getUsedOpNames() const
{
    eString usedOpNames;
    eString name;
    eArray<const eString*> categories;

    for (eU32 i=0; i<m_usedOps.size(); i++)
    {
        const eIOperator *op = m_usedOps[i];
        eASSERT(op != eNULL);

        name = op->getCategory();
        name += '_';
        name += op->getName();

        name.makeUpper();

        // Replace white spaces and dots with under scores.
        for (eU32 j=0; j<name.length(); j++)
        {
            eChar &c = name[j];

            if (c == ' ' || c == '.')
            {
                c = '_';
            }
        }

        eString opClassName = op->getMetaInfos().classNameString;
        usedOpNames += "#define HAVE_OP_";
        usedOpNames += name;
        usedOpNames += "\r\n";
        usedOpNames += "#define ";
        usedOpNames += opClassName;
        usedOpNames += "_ID ";
        char buf[1000];
        sprintf(buf,"%i", i);
        usedOpNames += buf;
        usedOpNames += "\r\n";

        // check categories
        eBool catFound = eFALSE;
        for(eU32 k = 0; k < categories.size(); k++)
            if(*categories[k] == op->getCategory())
                catFound = eTRUE;
        if(!catFound) {
            usedOpNames += "#define ";
            usedOpNames += op->getCategory();
            usedOpNames += "_CID ";
            sprintf(buf,"%i", categories.size());
            usedOpNames += buf;
            usedOpNames += "\r\n";
            categories.append(&op->getCategory());
        }
    }
    usedOpNames += "\r\n";
    usedOpNames += "#define TOTAL_OP_TYPE_COUNT ";
    char buf[1000];
    sprintf(buf,"%i", m_usedOps.size());
    usedOpNames += buf;
    usedOpNames += "\r\n";
    usedOpNames += "#ifndef SCRIPT_OPS_HPP\r\n";
    usedOpNames += "#define SCRIPT_OPS_HPP\r\n";
    usedOpNames += "\r\n";
    usedOpNames += "class eIOperator;\r\n";
    usedOpNames += "class SCRIPT_OP_CREATOR {\r\n";
    usedOpNames += "public:\r\n";
    usedOpNames += "    static eIOperator* createOp(unsigned int nr);\r\n";
    usedOpNames += "};\r\n";
    usedOpNames += "\r\n";
    usedOpNames += "#endif\r\n";

    return usedOpNames;
}


// Returns final demo script by concatenating all
// data streams into one stream. The "format" is
// (length stream 0, data stream 0) - ... -
// (length stream N - data stream N).
eByteArray eDemoScript::getFinalScript()
{
    eDataStream script;

    for (eU32 i=0; i<STREAM_COUNT; i++)
    {
        // Append stream length to script.
        const eByteArray &data = m_streams[i].getData();
        script.writeDword(data.size());

        // Append stream data to script.
        for (eU32 j=0; j<data.size(); j++)
        {
            script.writeByte(data[j]);
        }
    }

    return script.getData();
}

void eDemoScript::operator << (eU8 byte)
{
    m_streams[STREAM_INT8].writeByte(byte);
    m_byteLength += 1;
}

void eDemoScript::operator << (eS16 shrt)
{
    *this << (eU16)shrt;
}

void eDemoScript::operator << (eU16 word)
{
    m_streams[STREAM_INT16_0].writeByte(eLobyte(word));
    m_streams[STREAM_INT16_1].writeByte(eHibyte(word));
    m_byteLength += 2;
}

void eDemoScript::operator << (eS32 intgr)
{
    *this << (eU32)intgr;
}

void eDemoScript::operator << (eU32 dword)
{
    const eU16 lo = eLoword(dword);
    const eU16 hi = eHiword(dword);

    m_streams[STREAM_INT32_0].writeByte(eLobyte(lo));
    m_streams[STREAM_INT32_1].writeByte(eHibyte(lo));
    m_streams[STREAM_INT32_2].writeByte(eLobyte(hi));
    m_streams[STREAM_INT32_3].writeByte(eHibyte(hi));

    m_byteLength += 4;
}

void eDemoScript::operator << (const eString &str)
{
    m_streams[STREAM_STRING].writeString(str);
    m_byteLength += str.length() + 2;
}

void eDemoScript::operator << (const eRect &rect)
{
    *this << rect.left;
    *this << rect.top;
    *this << rect.right;
    *this << rect.bottom;
}

void eDemoScript::operator << (const ePoint &point)
{
    *this << point.x;
    *this << point.y;
}

void eDemoScript::operator << (const eVector2 &vec2)
{
    *this << vec2.x;
    *this << vec2.y;
}

void eDemoScript::operator << (const eVector3 &vec3)
{
    *this << vec3.x;
    *this << vec3.y;
    *this << vec3.z;
}

void eDemoScript::operator << (const eVector4 &vec4)
{
    *this << vec4.x;
    *this << vec4.y;
    *this << vec4.z;
    *this << vec4.w;
}

void eDemoScript::operator << (const eByteArray &block)
{
    *this << (eU16)block.size();

    for (eU32 i=0; i<block.size(); i++)
    {
        *this << block[i];
    }
}

void eDemoScript::operator << (eF32 flt)
{
    flt *= 256.0f*100.0f;

    const eU32 val = *(eU32 *)&flt;
    const eU8 *c = (eU8 *)&val;

    m_streams[STREAM_FIX24].writeByte(c[1]);
    m_streams[STREAM_FIX24].writeByte(c[2]);
    m_streams[STREAM_FIX24].writeByte(c[3]);
    
    m_byteLength += 3;
}

void eDemoScript::operator << (eBool bln)
{
	m_streams[STREAM_INT8].writeByte(bln);
    m_byteLength += 1;
}

#endif

void eDemoScript::operator >> (eU8 &byte)
{
    byte = m_streams[STREAM_INT8].readByte();
}

void eDemoScript::operator >> (eS16 &shrt)
{
    eU16 temp;

    *this >> temp;
    shrt = (eS16)temp;
}

void eDemoScript::operator >> (eU16 &word)
{
    const eU8 lo = m_streams[STREAM_INT16_0].readByte();
    const eU8 hi = m_streams[STREAM_INT16_1].readByte();

    word = eMakeWord(lo, hi);
}

void eDemoScript::operator >> (eS32 &intgr)
{
    eU32 temp;

    *this >> temp;
    intgr = (eS32)temp;
}

void eDemoScript::operator >> (eU32 &dword)
{
    const eU8 lo0 = m_streams[STREAM_INT32_0].readByte();
    const eU8 hi0 = m_streams[STREAM_INT32_1].readByte();
    const eU8 lo1 = m_streams[STREAM_INT32_2].readByte();
    const eU8 hi1 = m_streams[STREAM_INT32_3].readByte();

    dword = eMakeDword(eMakeWord(lo0, hi0), eMakeWord(lo1, hi1));
}

void eDemoScript::operator >> (eF32 &flt)
{
    eU32 val = 0;
    eU8 *c = (eU8 *)&val;

    c[1] = m_streams[STREAM_FIX24].readByte();
    c[2] = m_streams[STREAM_FIX24].readByte();
    c[3] = m_streams[STREAM_FIX24].readByte();

    flt = (*(eF32 *)&val)/256.0f/100.0f;
}

void eDemoScript::operator >> (eBool &bln)
{
    bln = m_streams[STREAM_INT8].readByte();
}

void eDemoScript::operator >> (eString &str)
{
    str = m_streams[STREAM_STRING].readString();
}

void eDemoScript::operator >> (eRect &rect)
{
    *this >> rect.left;
    *this >> rect.top;
    *this >> rect.right;
    *this >> rect.bottom;
}

void eDemoScript::operator >> (ePoint &point)
{
    *this >> point.x;
    *this >> point.y;
}

void eDemoScript::operator >> (eVector2 &vec2)
{
    *this >> vec2.x;
    *this >> vec2.y;
}

void eDemoScript::operator >> (eVector3 &vec3)
{
    *this >> vec3.x;
    *this >> vec3.y;
    *this >> vec3.z;
}

void eDemoScript::operator >> (eVector4 &vec4)
{
    *this >> vec4.x;
    *this >> vec4.y;
    *this >> vec4.z;
    *this >> vec4.w;
}

void eDemoScript::operator >> (eByteArray &block)
{
    eASSERT(block.size() == 0);

    eU16 size;
    eU8 byte;

    *this >> size;

    for (eU32 i=0; i<size; i++)
    {
        *this >> byte;
        block.append(byte);
    }
}