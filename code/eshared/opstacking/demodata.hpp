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

#ifndef DEMO_DATA_HPP
#define DEMO_DATA_HPP

class eDemoData
{
public:

#ifdef ePLAYER
    static void                 load(eDemoScript &script);

    static void                 addVirtualFile(const eString &fileName, const eByteArray &block);
    static eBool                removeVirtualFile(const eString &fileName);
    static const eByteArray *   getVirtualFile(const eString &fileName);

    static eIDemoOp *           getMainDemoOperator();
#else
    static void                 free();
    static void                 store(eDemoScript &script, const eIDemoOp *demoOp);
    static eString              eDemoData::getUsedOpCpp();

    static eOperatorPage *      addPage(eID pageId=eNOID);
    static eBool                removePage(eID pageId);
    static void                 clearPages();
    static void                 updateAllPageLinks();

    static eBool                existsPage(eID pageId);
    static eU32                 getPageCount();
    static eOperatorPage *      getPageByIndex(eU32 index);
    static eOperatorPage *      getPageById(eID pageId);
#endif

    static eBool                existsOperator(eID opId);
    static eU32                 getTotalOpCount();
    static eIOperator *         findOperator(eID opId);

    static tfSong *             newSong();
    static void                 removeSong(eID songId);
    static void                 clearSongs();
    static tfSong *             getSongByIndex(eU32 index);
    static tfSong *             getSongById(eU32 songId);
    static eU32                 getSongCount();

private:
#ifdef ePLAYER
    static eIOperator *         m_mainDemoOp;
    static eIOperator *         _loadOperator(eDemoScript &script, eU32 opTypeHashIdIndex);
#else
    static const eIOperator *   _resolveOperator(const eIOperator *op);
    static void                 _storeOperator(eDemoScript &script, const eIOperator *op, eID &idCounter, eArray<const eIOperator*>& ops);
    static void                 _preprocessOpTree(const eIOperator *op);
public:
    struct tStatRecord {
        const eString* name;
        eU32    cnt;
        eU32    accsize;
    };

    static eArray<tStatRecord>  stats;

#endif

private:
#ifdef ePLAYER
    struct VirtualFile
    {
        eString                 fileName;
        eByteArray              data;
    };

    typedef eArray<VirtualFile *> VirtualFilePtrArray;
#endif

private:
    static tfSongPtrArray       m_songs;

#ifdef eEDITOR
    static eOpPagePtrArray      m_pages;
#endif

#ifdef ePLAYER
    static VirtualFilePtrArray  m_virtFiles;
#endif
};

#endif // DEMO_DATA_HPP