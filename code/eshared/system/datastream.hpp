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

#ifndef DATA_STREAM_HPP
#define DATA_STREAM_HPP



// Class encapsulates a data stream used for easy
// reading/writing to a byte buffer. The streams
// are used to separate the different data types
// when serializing the demo data (this results
// in better compression ratios).
class eDataStream
{
public:
    eDataStream(eConstPtr mem=eNULL, eU32 length=0);

    void        attach(eConstPtr mem, eU32 length);

    void        writeBit(eBool bit);
	void		writeBits(eU32 dword, eU32 bitCount);
	void        writeByte(eU8 byte);
    void        writeWord(eU16 word);
    void        writeDword(eU32 dword);
    void        writeVbrDword(eU32 dword);
    void        writeString(const eString &str);

    eBool       readBit();
	eU32		readBits(eU32 bitCount);
    eU8         readByte();
    eU16        readWord();
    eU32        readDword();
    eU32        readVbrDword();
    eString     readString();

    void        advance(eU32 offset);

    eByteArray  getData() const;
    eU32        getReadIndex() const;
    eU32        getWriteIndex() const;

private:
    eByteArray  m_data;
    eBool       m_isReading;
    eU32        m_readIndex;
    eU32        m_bitCount;
    eU8         m_curByte;
};

#endif // DATA_STREAM_HPP