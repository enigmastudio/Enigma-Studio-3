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

#ifndef eHWSYNTH
#ifdef eEDITOR
#include <fstream>
#endif

#include "../eshared.hpp"

using namespace std;

eFile::eFile(const eChar *fileName) :
    m_fileName(fileName),
    m_data(eNULL),
    m_size(0),
    m_pos(0)
{
    eASSERT(fileName != eNULL);
}

eFile::~eFile()
{
    close();
}

eBool eFile::open()
{
    close();

#ifdef eEDITOR
    ifstream f(m_fileName, ifstream::binary | ifstream::in);

    if (!f.is_open())
    {
        return eFALSE;
    }

    // Retrieve file size.
    f.seekg(0, ifstream::end);
    m_size = (eU32)f.tellg();
    f.seekg(0, ifstream::beg);

    // Buffer the file.
    m_data = new eU8[m_size];
    eASSERT(m_data != eNULL);
    f.read((eChar *)m_data, m_size);
#else
    const eByteArray *fileData = eDemoData::getVirtualFile(m_fileName);

    if (fileData == eNULL)
    {
        return eFALSE;
    }

    m_size = fileData->size();
    m_data = &(*fileData)[0];
#endif

    return eTRUE;
}

void eFile::close()
{
#ifdef eEDITOR
    eSAFE_DELETE(m_data);
#else
    m_data = eNULL;
#endif

    m_size = 0;
    m_pos = 0;
}

eU32 eFile::read(ePtr buffer, eU32 byteCount)
{
    eASSERT(m_data != eNULL);

    eU32 readCount = byteCount;

    if (m_pos+byteCount > m_size)
    {
        readCount = m_size-m_pos;
    }

    eMemCopy(buffer, m_data+m_pos, readCount);
    m_pos += readCount;

    return readCount;
}

void eFile::readAll(eByteArray &buffer)
{
    buffer.resize(m_size);
    read(&buffer[0], m_size);
}

// Returns new absolute file position.
eU32 eFile::seek(SeekMode seekMode, eU32 offset)
{
    switch (seekMode)
    {
        case SEEKMODE_SET:
        {
            m_pos = eMin(offset, m_size);
            break;
        }

        case SEEKMODE_CUR:
        {
            m_pos = eMin(m_pos+offset, m_size);
            break;
        }

        case SEEKMODE_END:
        {
            m_pos = eMax(0, (eInt)m_size-(eInt)offset);
            break;
        }
    }

    return m_pos;
}

eU32 eFile::tell() const
{
    return m_pos;
}

eU32 eFile::getSize() const
{
    return m_size;
}

#endif // HWSYNTH
