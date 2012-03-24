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

#ifndef DEMO_SCRIPT_HPP
#define DEMO_SCRIPT_HPP

// Class used for loading and storing the demo script.
// The data values are managed in multiple data streams,
// depending on the data type. This way the final stream
// can be compressed a lot better, than just storing all
// different values in a row.
class eDemoScript
{
public:
    eDemoScript(eConstPtr script=eNULL, eU32 length=0);

#ifdef eEDITOR
    void                addUsedOp(const eIOperator *op);

    eString             getUsedOpNames() const;
    eByteArray          getFinalScript();
    eU32                m_byteLength;
#endif

    void                operator << (eU8 byte);
    void                operator << (eS16 shrt);
    void                operator << (eU16 word);
    void                operator << (eS32 intgr);
    void                operator << (eU32 dword);
    void                operator << (eF32 flt);
    void                operator << (eBool bln);
    void                operator << (const eString &str);
    void                operator << (const eRect &rect);
    void                operator << (const ePoint &point);
    void                operator << (const eVector2 &vec2);
    void                operator << (const eVector3 &vec3);
    void                operator << (const eVector4 &vec4);
    void                operator << (const eByteArray &block);

    void                operator >> (eU8 &byte);
    void                operator >> (eS16 &shrt);
    void                operator >> (eU16 &word);
    void                operator >> (eS32 &intgr);
    void                operator >> (eU32 &dword);
    void                operator >> (eF32 &flt);
    void                operator >> (eBool &bln);
    void                operator >> (eString &str);
    void                operator >> (eRect &rect);
    void                operator >> (ePoint &point);
    void                operator >> (eVector2 &vec2);
    void                operator >> (eVector3 &vec3);
    void                operator >> (eVector4 &vec4);
    void                operator >> (eByteArray &block);

private:
    enum StreamType
    {
        STREAM_BOOL,
        STREAM_INT8,
        STREAM_INT16_0,
        STREAM_INT16_1,
        STREAM_INT32_0,
        STREAM_INT32_1,
        STREAM_INT32_2,
        STREAM_INT32_3,
        STREAM_FIX24,
        STREAM_STRING,
        STREAM_BLOCKS,
        STREAM_COUNT
    };

private:
    eDataStream         m_streams[STREAM_COUNT];

#ifdef eEDITOR
    eIOpConstPtrArray   m_usedOps;
#endif
};

#endif // DEMO_SCRIPT_HPP