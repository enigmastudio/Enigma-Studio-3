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

eOperatorPage::eOperatorPage(eID pageId)
{
    if (pageId != eNOID)
    {
        // Page may not exists.
        eASSERT(eDemoData::existsPage(pageId) == eFALSE);
        m_id = pageId;
    }
    else
    {
        m_id = _generateNewId();
    }
}

eOperatorPage::~eOperatorPage()
{
    clearOperators();
}

#ifdef eEDITOR

// Adds an new operator at the given position of
// the given type to this page. The operator ID
// can be used when an operator has to have a
// specific ID, for example when loading from file.
eIOperator * eOperatorPage::addOperator(const eString &opType, const ePoint &pos, eInt width, eBool updateOpLinks, eID opId)
{
    eASSERT(pos.x >= 0 && pos.x < eOperatorPage::WIDTH);
    eASSERT(pos.y >= 0 && pos.y < eOperatorPage::HEIGHT);

    eIOperator *op = eOpFactory::get().newInstance(opType);
    eASSERT(op != eNULL);

    // Was a valid width specified?
    if (width > 0)
    {
        op->setWidth(width);
    }

    op->setPosition(pos);
    op->m_ownerPage = this;

    // Misuse areOpsMovable() to check if we can add at this position.
    eIOperatorPtrArray ops(1);
    ops[0] = op;

#ifdef eEDITOR
    if (!areOpsMovable(ops, ePoint()))
    {
        eSAFE_DELETE(op);
        return eNULL;
    }
#endif

    if (opId != eNOID && eDemoData::findOperator(opId) == eNULL)
    {
        // Operator may not exists.
        eASSERT(eDemoData::existsOperator(opId) == eFALSE);
        op->m_id = opId;
    }

    // Operator has to be inserted at correct
    // position (list has to keep sorted).
    const eInt insertAt = _findOperatorIndex(op->m_id, 0, m_ops.size()-1);
    eASSERT(insertAt < 0); // Operator shouldn't exist.
    m_ops.insert(-insertAt-1, op);

    // Update links if wanted.
    if (updateOpLinks)
    {
        updateLinks();
    }

    return op;
}
#endif


void eOperatorPage::addOperator(eIOperator *op)
{
    eASSERT(op != eNULL);

    op->m_ownerPage = this;

    const eInt insertAt = _findOperatorIndex(op->getId(), 0, m_ops.size()-1);
    eASSERT(insertAt < 0); // Operator shouldn't exist.
    m_ops.insert(-insertAt-1, op);
}

eBool eOperatorPage::removeOperator(eID opId, eBool updateOpLinks)
{
    eASSERT(opId != eNOID);

    const eInt index = _findOperatorIndex(opId, 0, m_ops.size()-1);

    if (index >= 0)
    {
        // Free memory of operator.
        eSAFE_DELETE(m_ops[index]);
        m_ops.removeAt(index);

        // Update links if wanted.
        if (updateOpLinks)
        {
            updateLinks();
        }

        return eTRUE;
    }

    return eFALSE;
}

void eOperatorPage::clearOperators()
{
    for (eU32 i=0; i<m_ops.size(); i++)
    {
        eSAFE_DELETE(m_ops[i]);
    }

    m_ops.clear();
}

// Recomputes links of all operators on this page.
#ifdef eEDITOR
void eOperatorPage::updateLinks()
{
    static eIOperator *opField[WIDTH][HEIGHT];
    _buildOpField(opField);

    // Linking operators have to be cleared before entering
    // connection loop. Operators from other pages have to
    // be kept!
    for (eU32 i=0; i<m_ops.size(); i++)
    {
        eIOperator *op = m_ops[i];
        eASSERT(op != eNULL);

        for (eInt j=op->m_linkingOps.size()-1; j>=0; j--)
        {
            eIOperator *linkingOp = eDemoData::findOperator(op->m_linkingOps[j]);

            if (linkingOp == eNULL || linkingOp->getOwnerPage() == this)
            {
                op->m_linkingOps.removeAt(j);
            }
        }
    }

    // First sort all operators by y-coordinate,
    // because bypassed operators have to be
    // processed before their outputs are processed
    // in order to be able to copy their inputs over.
    eIOperatorPtrArray ySortedOps(m_ops);
    ySortedOps.sort(_sortByOpCoordY);

    // Now do the connections on this page.
    eIOperatorPtrArray changedOps;

    for (eU32 i=0; i<ySortedOps.size(); i++)
    {
        eIOperator *op = ySortedOps[i];
        eASSERT(op != eNULL);

        // Store old input operators, so we can
        // see if input has changed and therefore
        // operator has to be updated.
        const eIOperatorPtrArray oldInputOps(op->m_inputOps);

        // Clear input and output operator lists.
        op->m_inputOps.clear();
        op->m_outputOps.clear();

        // Variables used to make sure, that each
        // input/output ID list is unique (every
        // (ID is just added once to list).
        eIOperator *oldInOp = eNULL;
        eIOperator *oldOutOp = eNULL;

        for (eU32 j=op->getPosition().x; j<op->getPosition().x+op->getWidth(); j++)
        {
            // Get input operators.
            if (op->getPosition().y > 0)
            {
                eASSERT(j < WIDTH);
                eASSERT(op->getPosition().y-1 < HEIGHT);

                eIOperator *curInOp = opField[j][op->getPosition().y-1];

                if (curInOp && curInOp != oldInOp)
                {
                    if (curInOp->getBypassed())
                    {
                        op->m_inputOps.append(curInOp->m_inputOps);
                    }
                    else
                    {
                        eASSERT(op->m_inputOps.exists(curInOp) == -1);
                        op->m_inputOps.append(curInOp);
                    }

                    oldInOp = curInOp;
                }
            }

            // Get output operators.
            if (op->getPosition().y < HEIGHT-1)
            {
                eASSERT(j < WIDTH);
                eASSERT(op->getPosition().y+1 < HEIGHT);

                eIOperator *curOutOp = opField[j][op->getPosition().y+1];

                if (curOutOp && oldOutOp != curOutOp)
                {
                    op->m_outputOps.append(curOutOp);
                    oldOutOp = curOutOp;
                }
            }

            // Add us to the list of linking operators
            // of the operators we're linking.
            for (eU32 j=0; j<op->getParameterCount(); j++)
            {
                const eParameter &p = op->getParameter(j);

                if (p.getType() == eParameter::TYPE_LINK)
                {
                    eIOperator *linkedOp = eDemoData::findOperator(p.getValue().linkedOpId);

                    if (linkedOp)
                    {
                        if (linkedOp->m_linkingOps.exists(op->getId()) == -1)
                        {
                            linkedOp->m_linkingOps.append(op->getId());
                        }
                    }
                }
                else if (p.isAnimated())
                {
                    eIOperator *pathOp = eDemoData::findOperator(p.getAnimationPathOpId());

                    if (pathOp)
                    {
                        if (pathOp->m_linkingOps.exists(op->getId()) == -1)
                        {
                            pathOp->m_linkingOps.append(op->getId());
                        }
                    }
                }
            }
        }

        // Check if input operators have changed.
        // If they have, set operator to changed.
        if (oldInputOps.size() != op->m_inputOps.size())
        {
            // Count unequal => changed.
            changedOps.append(op);
        }
        else
        {
            // Count is the same, so check for changes.
            for (eU32 i=0; i<oldInputOps.size(); i++)
            {
                if (oldInputOps[i] != op->m_inputOps[i])
                {
                    changedOps.append(op);
                    break;
                }
            }
        }
    }

    // Set all changed operators to changed.
    for (eU32 i=0; i<changedOps.size(); i++)
    {
        changedOps[i]->setChanged();
    }
}
#else
void eOperatorPage::updateLinks()
{
    for (eU32 i=0; i<m_ops.size(); i++)
    {
        eIOperator *op = m_ops[i];

        for (eU32 j=0; j<op->m_inputOps.size(); j++)
        {
            op->m_inputOps[j]->m_outputOps.append(op);
        }

        for (eU32 j=0; j<op->m_params.size(); j++)
        {
            const eParameter &p = *op->m_params[j];

            if (p.getType() == eParameter::TYPE_LINK)
            {
                eIOperator *linkedOp = eDemoData::findOperator(p.getValue().linkedOpId);

                if (linkedOp)
                {
                    linkedOp->m_linkingOps.append(op->getId());
                }
            }

            if (p.isAnimated())
            {
                eIOperator *animOp = eDemoData::findOperator(p.getAnimationPathOpId());

                if (animOp)
                {
                    animOp->m_linkingOps.append(op->getId());
                }
            }
        }
    }
}
#endif

eID eOperatorPage::getId() const
{
    return m_id;
}

eU32 eOperatorPage::getOperatorCount() const
{
    return m_ops.size();
}

eIOperator * eOperatorPage::getOperatorById(eID opId) const
{
    eInt index = _findOperatorIndex(opId, 0, m_ops.size()-1);
    return (index >= 0 ? m_ops[index] : eNULL);
}

eIOperator * eOperatorPage::getOperatorByPos(const ePoint &pos, eU32 width) const
{
    eASSERT(width > 0);

    for (eU32 i=0; i<m_ops.size(); i++)
    {
        const ePoint &opPos = m_ops[i]->getPosition();

        if (opPos.y == pos.y)
        {
            if ((opPos.x <= pos.x && opPos.x+(eInt)m_ops[i]->getWidth() > pos.x) ||
                (opPos.x >= pos.x && opPos.x < pos.x+(eInt)width))
            {
                return m_ops[i];
            }
        }
    }

    return eNULL;
}

eIOperator * eOperatorPage::getOperatorByIndex(eU32 index) const
{
    eASSERT(index < m_ops.size());
    return m_ops[index];
}

#ifdef eEDITOR
eBool eOperatorPage::getFreeInputPosition(const eIOperator *op, ePoint &pos) const
{
    static eIOperator *opField[WIDTH][HEIGHT];
    _buildOpField(opField, eFALSE);

    const ePoint &opPos = op->getPosition();

    if (opPos.y > 0) // First row is impossible.
    {
        const eU32 maxX = opPos.x+op->getWidth();

        for (eU32 x=opPos.x; x<maxX; x++)
        {
            // Try finding a free position above which
            // has a minimum width of three.
            eBool freePosFound = eTRUE;

            for (eU32 i=0; (i<3 && x+i<maxX); i++)
            {
                const eIOperator *aboveOp = opField[x+i][opPos.y-1];

                if (aboveOp != eNULL)
                {
                    // Not found so advance x-position.
                    freePosFound = eFALSE;
                    x += aboveOp->getWidth()-1;
                    break;
                }
            }

            // Position found?
            if (freePosFound)
            {
                pos.set(x, op->getPosition().y-1);
                return eTRUE;
            }
        }
    }

    return eFALSE;
}

// Checks all operators in given array can be moved
// by the given distance (if there are no other operators
// lying around, or the would be out of the page).
eBool eOperatorPage::areOpsMovable(const eIOperatorPtrArray &ops, const ePoint &dist) const
{
    static eIOperator *opField[WIDTH][HEIGHT];
    _buildOpField(opField, eFALSE);

    for (eU32 i=0; i<ops.size(); i++)
    {
        const eIOperator *op = ops[i];
        eASSERT(op != eNULL);
        eASSERT(op->getOwnerPage() == this);

        const ePoint pos = op->getPosition()+dist;

        for (eU32 j=pos.x; j<pos.x+op->getWidth(); j++)
        {
            // Would operator be moved out of page?
            if (j < 0 || j > WIDTH-1 || pos.y < 0 || pos.y > HEIGHT-1)
            {
                return eFALSE;
            }

            // Is at the target position already another
            // operator lying?
            eBool free = (opField[j][pos.y] == eNULL);

            for (eU32 k=0; k<ops.size() && !free; k++)
            {
                if (opField[j][pos.y] == ops[k])
                {
                    free = eTRUE;
                }
            }

            if (!free)
            {
                return eFALSE;
            }
        }
    }

    return eTRUE;
}

void eOperatorPage::setUserName(const eString &userName)
{
    m_userName = userName;
}

const eString & eOperatorPage::getUserName() const
{
    return m_userName;
}
#endif

// Does a binary search for an operator. The index
// of the operator with given ID is returned, or
// -(insertat+1), if operator couldn't be found
// on this page. This value can easily be
// transformed into the position to insert the
// operator into the array.
eInt eOperatorPage::_findOperatorIndex(eID opId, eInt startIndex, eInt stopIndex) const
{
    while (startIndex <= stopIndex)
    {
        // Compute midpoint.
        const eInt mid = (startIndex+stopIndex)/2; 
        eASSERT(mid >= 0 && mid < (eInt)m_ops.size());

        if (opId > m_ops[mid]->getId())
        {
            // Repeat search in top half.
            startIndex = mid+1;
        }
        else if (opId < m_ops[mid]->getId()) 
        {
            // Repeat search in bottom half.
            stopIndex = mid-1;
        }
        else
        {
            // Found it, so return position.
            return mid;
        }
    }

    // Failed to find key.
    return -(startIndex+1);
}

// Builds a two-dimensional array which represents
// the page with its operators. At positions where
// no operator lies, NULL is put into the array.
void eOperatorPage::_buildOpField(eIOperator *opField[WIDTH][HEIGHT], eBool excludeHiddenOps) const
{
    eMemSet(opField, eNULL, WIDTH*HEIGHT*sizeof(eIOperator *));

    for (eU32 i=0; i<m_ops.size(); i++)
    {
        eIOperator *op = m_ops[i];
        eASSERT(op != eNULL);

        // Only add operator to field, if operator is visible.
        if (!op->getHidden() || !excludeHiddenOps)
        {
            const ePoint &pos = op->getPosition();

            for (eU32 j=pos.x; j<pos.x+op->getWidth(); j++)
            {
                eASSERT(opField[j][pos.y] == eNULL); // Should be empty.
                opField[j][pos.y] = op;
            }
        }
    }
}

eBool eOperatorPage::_sortByOpCoordY(eIOperator * const &a, eIOperator * const &b)
{
    eASSERT(a != eNULL);
    eASSERT(b != eNULL);

    return (b->getPosition().y < a->getPosition().y);
}

// Generates a randomized ID. It is checked
// if the newly generated ID already exists.
eID eOperatorPage::_generateNewId() const
{
    eRandomize(eTimer::getTimeMs());

    eID id;

    do
    {
        id = eRandom();
    }
    while (eDemoData::existsPage(id) == eTRUE);

    return id;
}