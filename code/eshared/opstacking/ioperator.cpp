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

#ifdef eDEBUG
#include <windows.h>
#include <stdio.h>
#endif

eIOperator::eIOperator() :
#ifdef eEDITOR
    m_valid(eTRUE),
    m_checkValidity(eTRUE),
#endif
    m_id(_generateNewId()),
    m_changed(eTRUE),
    m_bypassed(eFALSE),
    m_hidden(eFALSE),
    m_ownerPage(eNULL),
    m_width(4),
    m_visited(eFALSE)
{
}

#ifdef eEDITOR
eIOperator::~eIOperator()
{
    _clearParameters();
}
#endif

#ifdef ePLAYER
void eIOperator::load(eDemoScript &script)
{
    for (eU32 i=0; i<m_params.size(); i++)
    {
//        m_params[i]->load(script);
#ifdef eDEBUG
    if(m_params[i]->getType() != eParameter::TYPE_LINK) {
        char debbuf[1000];
        if(m_params[i]->isAnimated()) {
            sprintf(debbuf, " isanimated ->\n");
            OutputDebugString(debbuf);
        }
        sprintf(debbuf, "  Parameter %i type %i : ", i, m_params[i]->getType());
        OutputDebugString(debbuf);
        if(m_params[i]->isTextType()) {
            sprintf(debbuf, "String [%s]", m_params[i]->m_value.string);
            OutputDebugString(debbuf);
        } else {
            eU32 k = eParameter::M_RELEVANT_PARAMETER_LENGTH[m_params[i]->getType()];
            sprintf(debbuf, "Val %i : ", k);
            OutputDebugString(debbuf);
            for(eU32 n = 0; n < k; n++) {
                sprintf(debbuf, "%i, ", ((eU8*)&m_params[i]->m_value)[n]);
                OutputDebugString(debbuf);
            }
        }
        sprintf(debbuf, "\n");
        OutputDebugString(debbuf);
    }
#endif
    }
}
#else
void eIOperator::store(eDemoScript &script) const
{
    eU32 k = 0;
    for (eU32 i=0; i<m_params.size(); i++)
    {
        if(m_params[i]->getType() != eParameter::TYPE_LABEL)
        {
#ifdef eDEBUG
    if(m_params[i]->getType() != eParameter::TYPE_LINK) {
        char debbuf[1000];
        if(m_params[i]->isAnimated()) {
            sprintf(debbuf, " isanimated ->\n");
            OutputDebugString(debbuf);
        }
        sprintf(debbuf, "  Parameter %i type %i : ", k, m_params[i]->getType());
        OutputDebugString(debbuf);
        if(m_params[i]->isTextType()) {
            sprintf(debbuf, "String [%s]", m_params[i]->m_value.string);
            OutputDebugString(debbuf);
        } else {
            eU32 m = eParameter::M_RELEVANT_PARAMETER_LENGTH[m_params[i]->getType()];
            sprintf(debbuf, "Val %i : ", m);
            OutputDebugString(debbuf);
            for(eU32 n = 0; n < m; n++) {
                sprintf(debbuf, "%i, ", ((eU8*)&m_params[i]->m_value)[n]);
                OutputDebugString(debbuf);
            }
        }
        sprintf(debbuf, "\n");
        OutputDebugString(debbuf);
    }
#endif
//            m_params[i]->store(script);
            k++;
        }
    }

    // Add this operator to the list of used operators.
    script.addUsedOp(this);
}
#endif

// Processes (executes) all operators in input
// list of operator and operator it-self. Returns
// wether or not one or more operators were
// executed.
eBool eIOperator::process(eIRenderer *renderer, eF32 time, eBool (*callback)(eIRenderer *renderer, eU32 processed, eU32 total, ePtr param), ePtr param)
{
    ePROFILER_ZONE("Process operator");

    eASSERT(time >= 0.0f);

    // First animate parameters.
    eIOperatorPtrArray stackOps;
    getOpsInStack(stackOps);

    for (eU32 i=0; i<stackOps.size(); i++)
    {
        stackOps[i]->_animateParameters(time);
        stackOps[i]->m_visited = eFALSE;
    }

    // Fetch all operators to process (changed
    // input and changed linked operators).
    eIOperatorPtrArray changedOps;
    
    for (eU32 i=0; i<stackOps.size(); i++)
    {
        eASSERT(changedOps.exists(stackOps[i]) == -1);

        if (stackOps[i]->getChanged())
        {
            changedOps.append(stackOps[i]);
        }
    }

    if (changedOps.isEmpty())
    {
        return eFALSE;
    }

    // Iterate over list of changed operators
    // and execute them.
    for (eU32 i=0; i<changedOps.size(); i++)
    {
        eIOperator *op = changedOps[i];
        eASSERT(op != eNULL);

#ifdef eEDITOR
        if (op->getValid())
#endif
        {
            eGraphicsApiDx9 *gfx = (renderer ? renderer->getGraphicsApi() : eNULL);

            op->_preExecute(gfx);
            op->_callExecute(gfx);
        }

        // Set operator and all its parameters to unchanged.
        op->m_changed = eFALSE;

        for (eU32 j=0; j<op->m_params.size(); j++)
        {
            op->m_params[j]->setChanged(eFALSE);
        }

        // Call callback procedure (used for
        // progress information in calling code).
        if (callback)
        {
            if (!callback(renderer, i+1, changedOps.size(), param))
            {
                // Interrupt generation of demo content.
                return eFALSE;
            }
        }
    }

    return eTRUE;
}

void eIOperator::setChanged()
{
    ePROFILER_ZONE("Operator set changed");

    // If operator was already set to changed, avoid
    // traversing the graph again and just return.
    if (m_changed)
    {
        return;
    }

#ifdef eEDITOR
    // Update validity status.
    m_valid = checkValidity();
    m_checkValidity = !m_valid;
#endif

    // Set all output operators (below us) to changed.
    for (eU32 i=0; i<m_outputOps.size(); i++)
    {
        m_outputOps[i]->setChanged();
    }

    // Set operators which are linking us to changed.
    for (eU32 i=0; i<m_linkingOps.size(); i++)
    {
        eIOperator *op = eDemoData::findOperator(m_linkingOps[i]);

        if (op)
        {
            // Set linking parameters explicitly to changed.
            for (eU32 j=0; j<op->m_params.size(); j++)
            {
                eParameter &param = *op->m_params[j];

                if (param.getType() == eParameter::TYPE_LINK)
                {
                    eIOperator *linkingOp = eDemoData::findOperator(param.getValue().linkedOpId);

                    if (linkingOp == this)
                    {
                        param.setChanged(eTRUE);
                    }
                }
                else if (param.isAnimated())
                {
                    eIOperator *animOp = eDemoData::findOperator(param.getAnimationPathOpId());

                    if (animOp == this)
                    {
                        param.setChanged(eTRUE);
                    }
                }
            }

            // Set linking operator to changed.
            op->setChanged();
        }
    }

    // Set us to changed, too.
    m_changed = eTRUE;
}

void eIOperator::setPosition(const ePoint &pos)
{
    eASSERT(pos.x >= 0 && pos.x+m_width <= eOperatorPage::WIDTH);
    eASSERT(pos.y >= 0 && pos.y < eOperatorPage::HEIGHT);

    m_position = pos;
}

void eIOperator::setWidth(eU32 width)
{
    eASSERT(width > 0);
    eASSERT(m_position.x+width <= eOperatorPage::WIDTH);

    m_width = width;
}

void eIOperator::setBypassed(eBool bypass)
{
    m_bypassed = bypass;
}

void eIOperator::setHidden(eBool hidden)
{
    m_hidden = hidden;
}

eU32 eIOperator::getParameterCount() const
{
    return m_params.size();
}

#ifdef eEDITOR
eParameter & eIOperator::getParameter(eString str)
{
    for(eU32 i = 0; i < this->getParameterCount(); i++) {
        eString parName = this->getParameter(i).m_name;
        if(eStrCompare(parName, str) == 0)
            return this->getParameter(i);
    }
    return this->getParameter(-1);
}
#endif

eParameter & eIOperator::getParameter(eU32 index)
{
    eASSERT(index < m_params.size());
    return *m_params[index];
}

const eParameter & eIOperator::getParameter(eU32 index) const
{
    eASSERT(index < m_params.size());
    return *m_params[index];
}

eU32 eIOperator::getInputCount() const
{
    return m_inputOps.size();
}

eIOperator * eIOperator::getInputOperator(eU32 index) const
{
    eASSERT(index < m_inputOps.size());
    return m_inputOps[index];
}

eU32 eIOperator::getOutputCount() const
{
    return m_outputOps.size();
}

eIOperator * eIOperator::getOutputOperator(eU32 index) const
{
    eASSERT(index < m_outputOps.size());
    return m_outputOps[index];
}

eU32 eIOperator::getLinkingCount() const
{
    return m_linkingOps.size();
}

eID eIOperator::getLinkingOperator(eU32 index) const
{
    eASSERT(index < m_linkingOps.size());
    return m_linkingOps[index];
}

eID eIOperator::getId() const
{
    return m_id;
}

eOperatorPage * eIOperator::getOwnerPage() const
{
    return m_ownerPage;
}

eBool eIOperator::getChanged() const
{
    return m_changed;
}

ePoint eIOperator::getPosition() const
{
    return m_position;
}

eU32 eIOperator::getWidth() const
{
    return m_width;
}

eBool eIOperator::getBypassed() const
{
    return m_bypassed;
}

eBool eIOperator::getHidden() const
{
    return m_hidden;
}

#ifdef eEDITOR
const eIOperator::MetaInfos & eIOperator::getMetaInfos() const
{
    return *m_metaInfos;
}
#endif

eBool eIOperator::isAffectedByAnimation() const
{
    // Checks if a parameter is animated or if a
    // parameter is a link if the linked operator
    // stack is animated.
    for (eU32 i=0; i<m_params.size(); i++)
    {
        const eParameter &param = *m_params[i];

        if (param.isAffectedByAnimation())
        {
            return eTRUE;
        }
    }

    // Checks if an input operator is animated.
    for (eU32 i=0; i<m_inputOps.size(); i++)
    {
        if (m_inputOps[i]->isAffectedByAnimation())
        {
            return eTRUE;
        }
    }

    return eFALSE;
}


#ifdef eEDITOR
const eString & eIOperator::getCategory() const
{
    return m_metaInfos->category;
}

const eString & eIOperator::getName() const
{
    return m_metaInfos->name;
}

const eString & eIOperator::getType() const
{
    return m_metaInfos->type;
}

const eString & eIOperator::getRealType() const
{
    return m_metaInfos->type;
}
#endif


void eIOperator::getOpsInStack(eIOperatorPtrArray &ops)
{
    ePROFILER_ZONE("Collect stack operators");

    _getOpsInStackInternal(ops);

    for (eU32 i=0; i<ops.size(); i++)
    {
        ops[i]->m_visited = eFALSE;
    }
}

#ifdef eEDITOR
eBool eIOperator::getFreeInputPosition(ePoint &pos) const
{
    eASSERT(m_ownerPage != eNULL);
    return m_ownerPage->getFreeInputPosition(this, pos);
}

void eIOperator::setUserName(const eString &userName)
{
    m_userName = userName;
}


// Function for convinience. Specialized for adding
// enumeration parameters. The enumeration items
// are given in just one single string and parsed
// in this function. The format is "item1|item2|...".
eParameter & eIOperator::addParameter(eParameter::Type type, const eString &name, const eString &items, eU32 index)
{
    m_params.append(new eParameter(type, name, this));

    eParameter &param = *m_params[m_params.size()-1];
    eParameter::Value value;

    // Depending on type initialize parameter.
    switch (type)
    {
        case eParameter::TYPE_ENUM:
        case eParameter::TYPE_FLAGS:
        {
            value.enumSel = index;
            value.flags = index;

            param.setDescriptionItems(items);
            break;
        }

        case eParameter::TYPE_TSHADERCODE:
        case eParameter::TYPE_STRING:
        case eParameter::TYPE_LABEL:
        case eParameter::TYPE_TEXT:
        case eParameter::TYPE_FILE:
        {
            eStrNCopy(value.text, items, eParameter::MAX_TEXT_LENGTH);
            break;
        }
    }

    param.setDefault(value);

    // Verify that index isn't out of range. If
    // parameter is of type enumeration, index
    // is used and therefore has to be in range.
    eASSERT(type != eParameter::TYPE_ENUM || index < param.getDescriptionItems().size());
    return param;
}

eParameter & eIOperator::addParameter(eParameter::Type type, const eString &name, eF32 min, eF32 max, const eVector4 &v)
{
    m_params.append(new eParameter(type, name, this));

    eParameter &param = *m_params[m_params.size()-1];
    param.setMinMax(min, max);

    eParameter::Value value;

    if (param.isIntType())
    {
        value.ixyxy = eRect((eInt)v.x, (eInt)v.y, (eInt)v.z, (eInt)v.w);
    }
    else if (type == eParameter::TYPE_BOOL)
    {
        value.boolean = (eBool)v.x;
    }
    else
    {
        value.fxyzw = v;
    }

    param.setDefault(value);
    return param;
}

void eIOperator::getMinMaxInput(eU32 &minInput, eU32 &maxInput) const
{
    minInput = m_metaInfos->minInput;
    maxInput = m_metaInfos->maxInput;
}

eChar eIOperator::getShortcut() const
{
    return m_metaInfos->shortcut;
}

eColor eIOperator::getColor() const
{
    return m_metaInfos->color;
}

eBool eIOperator::getValid() const
{
#ifdef eEDITOR
    if (m_checkValidity)
    {
        m_valid = checkValidity();
        m_checkValidity = eFALSE;
    }
#endif

    return m_valid;
}

const eString & eIOperator::getUserName() const
{
    return m_userName;
}

void eIOperator::doEditorInteraction(eGraphicsApiDx9 *gfx, eSceneData &sg)
{
}

eBool eIOperator::checkValidity() const
{
    for (eU32 i=0; i<m_inputOps.size(); i++)
    {
        // Is input operator it-self valid?
        const eIOperator *op = m_inputOps[i];
        eASSERT(op != eNULL);

        if (!op->getValid())
        {
            return eFALSE;
        }

        // Verify, that input operators are from
        // allowed category/type and are valid.
        if (!m_metaInfos->allowedInput.isAllowedAt(i, op))
        {
            return eFALSE;
        }
    }

    // All linked operators have to be valid (to
    // get linked operators we look at parameters).
    for (eU32 i=0; i<m_params.size(); i++)
    {
        const eParameter &p = *m_params[i];

        if (p.getType() == eParameter::TYPE_LINK)
        {
            const eID linkedOpId = p.getValue().linkedOpId;
            const eIOperator *op = eDemoData::findOperator(linkedOpId);

            if (op)
            {
                // Avoid direct recursive linking
                // and verify that operator is valid.
                if (op == this || !op->getValid())
                {
                    return eFALSE;
                }

                // Linked operator has to be
                // from allowed type.
                if (!p.getAllowedLinks().isAllowed(op))
                {
                    return eFALSE;
                }
            }
        }
    }

    // Number of input operators has to be in range.
    eU32 minInput, maxInput;

    getMinMaxInput(minInput, maxInput);
    return (m_inputOps.size() >= minInput && m_inputOps.size() <= maxInput);
}

    void eIOperator::optimizeForExport() {
    };


#elif ePLAYER
// Function for convinience. Specialized for adding
// enumeration parameters. The enumeration items
// are given in just one single string and parsed
// in this function. The format is "item1|item2|...".
eParameter & eIOperator::addParameter(eParameter::Type type)
{
    m_params.append(new eParameter(type, this));

    eParameter &param = *m_params[m_params.size()-1];
    param.m_value.byteCode = eByteArray();

    return param;
}
#endif

void eIOperator::_initialize()
{
}

void eIOperator::_deinitialize()
{
}

void eIOperator::_callExecute(eGraphicsApiDx9 *gfx)
{
    ePROFILER_ZONE("Call operator execute");

#ifdef eEDITOR
    ePtr func = m_metaInfos->execFunc;
#else
    ePtr func = m_metaExecFunc;
#endif
    eASSERT(func != eNULL);

    eU32 stackSize = 4;

    for (eInt i=(eInt)m_params.size()-1; i>=0; i--)
    {
        const eParameter &p = *m_params[i];
        const eParameter::Value &val = p.getValue();

        switch (p.getType())
        {
#ifdef ePLAYER
            case eParameter::TYPE_TSHADERCODE:
            {
                const eByteArray &bc = val.byteCode;

                __asm push dword ptr [bc]
                stackSize += 4;
                break;
            }
#endif
            case eParameter::TYPE_IXY:
            case eParameter::TYPE_IXYZ:
            case eParameter::TYPE_IXYXY:
            case eParameter::TYPE_FXY:
            case eParameter::TYPE_FXYZ:
            case eParameter::TYPE_FXYZW:
            case eParameter::TYPE_RGB:
            case eParameter::TYPE_RGBA:
            {
                const eFXYZ &v = val.fxyz;

                __asm push dword ptr [v]
                stackSize += 4;
                break;
            }

            case eParameter::TYPE_LINK:
            {
                eIOperator *linkedOp = eDemoData::findOperator(val.linkedOpId);
                __asm push dword ptr [linkedOp];
                stackSize += 4;
                break;
            }

#ifdef eEDITOR
            case eParameter::TYPE_TSHADERCODE:
#endif
            case eParameter::TYPE_SYNTH:
            case eParameter::TYPE_STRING:
            case eParameter::TYPE_TEXT:
            case eParameter::TYPE_FILE:
            {
                const eChar *string = val.string;
                __asm push dword ptr [string]
                stackSize += 4;
                break;
            }

            
            case eParameter::TYPE_BOOL:
            case eParameter::TYPE_ENUM:
            case eParameter::TYPE_FLAGS:
            case eParameter::TYPE_INT:
            {
                const int integer = val.integer;
                __asm push dword ptr [integer]
                stackSize += 4;
                break;
            }

            case eParameter::TYPE_FLOAT:
            {
                const float flt = val.flt;

                __asm
                {
                    fld     dword ptr [flt]
                    sub     esp, 4
                    fstp    dword ptr [esp]
                }

                stackSize += 4;
                break;
            }
        }
    }

    // Call execute function. The calling convention
    // used for non-static member functions is
    // this-call. Hence no stack clean up required here.
    __asm
    {
        push    dword ptr [gfx] // first parameter is always graphics API
        mov     ecx, dword ptr [this]
        call    dword ptr [func]
    }

}

void eIOperator::_animateParameters(eF32 time)
{
    ePROFILER_ZONE("Animate parameters");
    eASSERT(time >= 0.0f);

    eBool changed = eFALSE;

    for (eU32 i=0; i<m_params.size(); i++)
    {
        if (m_params[i]->animate(time))
        {
            changed = eTRUE;
        }
    }

    if (changed)
    {
        setChanged();
    }
}

void eIOperator::_clearParameters()
{
    // Explicitly call destructor of parameter,
    // because it is not called automatically
    // due to the 64k-ish implementation of the
    // array.
    for (eU32 i=0; i<m_params.size(); i++)
    {
        eSAFE_DELETE(m_params[i]);
    }

    m_params.clear();
}

// This function is called before virtual execute
// function is called. Do preparation of your
// operator here.
void eIOperator::_preExecute(eGraphicsApiDx9 *gfx)
{
}

// Returns a list of all changed operators to
// process when executing. The operators in the
// list are in the following order: the first one
// has to be executed first, because all other
// operators are needing its data. The last operator
// is the calling one.
void eIOperator::_getOpsInStackInternal(eIOperatorPtrArray &ops)
{
    if (m_visited)
    {
        return;
    }

    // Get linked operators.
    for (eU32 i=0; i<m_params.size(); i++)
    {
        const eParameter &param = *m_params[i];

        if (param.isAnimated())
        {
            eIOperator *op = eDemoData::findOperator(param.getAnimationPathOpId());

            if (op)
            {
                op->_getOpsInStackInternal(ops);
            }
        }
        else if (param.getType() == eParameter::TYPE_LINK)
        {
            eIOperator *op = eDemoData::findOperator(param.getValue().linkedOpId);

            if (op)
            {
                op->_getOpsInStackInternal(ops);
            }
        }
    }

    // Get changed input operators.
    for (eU32 i=0; i<m_inputOps.size(); i++)
    {
        m_inputOps[i]->_getOpsInStackInternal(ops);
    }

#ifdef eEDITOR
    // Do we have to update our validity status?
    if (m_checkValidity)
    {
        m_valid = checkValidity();
        m_checkValidity = eFALSE;
    }
#endif

    ops.append(this);
    m_visited = eTRUE;
}

// Generates a randomized ID. It is checked
// if the newly generated ID already exists.
eID eIOperator::_generateNewId() const
{
    eRandomize(eTimer::getTimeMs());

    eID id;

    do
    {
        id = eRandom();
    }
    while (eDemoData::existsOperator(id) == eTRUE);

    return id;
}