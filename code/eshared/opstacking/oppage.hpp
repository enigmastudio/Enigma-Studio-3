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

#ifndef OP_PAGE_HPP
#define OP_PAGE_HPP

// A page where operators can be placed on.
class eOperatorPage
{
    friend class eDemoData;

public:
    static const eInt   WIDTH = 65*3-1;
    static const eInt   HEIGHT = 95;

public:
    eOperatorPage(eID pageId=eNOID);
    ~eOperatorPage();

#ifdef eEDITOR
    eIOperator *        addOperator(const eString &opType, const ePoint &pos, eInt width=-1, eBool updateOpLinks=eTRUE, eID opId=eNOID);
#endif

protected:
    void                addOperator(eIOperator *op);

public:
    eBool               removeOperator(eID opId, eBool updateOpLinks=eTRUE);
    void                clearOperators();

    void                updateLinks();

    eID                 getId() const;
    eU32                getOperatorCount() const;
    eIOperator *        getOperatorById(eID opId) const;
    eIOperator *        getOperatorByPos(const ePoint &pos, eU32 width) const;
    eIOperator *        getOperatorByIndex(eU32 index) const;

#ifdef eEDITOR
    eBool               getFreeInputPosition(const eIOperator *op, ePoint &pos) const;
    eBool               areOpsMovable(const eIOperatorPtrArray &ops, const ePoint &dist) const;

    void                setUserName(const eString &userName);
    const eString &     getUserName() const;
#endif

private:
    eID                 _generateNewId() const;
    eInt                _findOperatorIndex(eID opId, eInt startIndex, eInt stopIndex) const;
    void                _buildOpField(eIOperator *opField[WIDTH][HEIGHT], eBool excludeHiddenOps=eTRUE) const;

private:
    static eBool        _sortByOpCoordY(eIOperator * const &a, eIOperator * const &b);

private:
    eID                 m_id;
    eIOperatorPtrArray  m_ops;

#ifdef eEDITOR
    eString             m_userName;
#endif
};

typedef eArray<eOperatorPage *> eOpPagePtrArray;

#endif // OP_PAGE_HPP