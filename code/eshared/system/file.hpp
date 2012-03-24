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

#ifndef FILE_HPP
#define FILE_HPP

#ifndef eHWSYNTH

// Class used for loading files. In the editor the
// files are loaded from hard-disk, in the editor
// the files are loaded from the data array.
class eFile
{
public:
    enum SeekMode
    {
        SEEKMODE_SET,
        SEEKMODE_CUR,
        SEEKMODE_END
    };

public:
    eFile(const eChar *fileName);
    ~eFile();

    eBool           open();
    void            close();
    eU32            read(ePtr buffer, eU32 byteCount);
    void            readAll(eByteArray &buffer);
    eU32            seek(SeekMode seekMode, eU32 offset);

    eU32            tell() const;
    eU32            getSize() const;

private:
    const eChar *   m_fileName;
    const eU8 *     m_data;
    eU32            m_size;
    eU32            m_pos;
};

#endif //HWSYNTH

#endif // FILE_HPP
