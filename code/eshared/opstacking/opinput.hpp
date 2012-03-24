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

#ifndef OP_INPUT_HPP
#define OP_INPUT_HPP

class eIOperator;

#ifdef eEDITOR
// Class used for determining if a certain
// operator type or category is allowed as
// input for an operator.
class eAllowedOpInput
{
public:
    eAllowedOpInput(const eString &config="");
    ~eAllowedOpInput();

    void                parseConfig(const eString &config);

    eBool               isAllowed(const eString &what) const;
    eBool               isAllowed(const eIOperator *op) const;

    eBool               isAllowedAt(eU32 index, const eString &what) const;
    eBool               isAllowedAt(eU32 index, const eIOperator *op) const;

    eStringPtrArray     getAllAllowed() const;

private:
    void                _clear();

private:
    struct WhereWhat
    {
        eInt            index;
        eString         what;
    };

    typedef eArray<WhereWhat *> WhereWhatPtrArray;

private:
    WhereWhatPtrArray   m_allowed;
    eStringPtrArray     m_allAllowed;
};
#endif

#endif