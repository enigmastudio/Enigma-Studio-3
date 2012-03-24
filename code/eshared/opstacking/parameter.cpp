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

#if defined(eEDITOR) || defined(eDEBUG)
const eU32 eParameter::M_RELEVANT_PARAMETER_LENGTH[] =
{
    4, 1, 1, sizeof(eBool), 0, 0, 4, 8, 12, 16, 12, 16, 8, 12, 16, 0, 4, 0, 0, 0
};
#endif


eParameter::eParameter(Type type, eIOperator *ownerOp) :
#ifdef eEDITOR
    m_animatable(isAnimatableType()),
#endif
    m_type(type),
    m_animated(eFALSE),
    m_animPathOpId(eNOID),
    m_animChannel(CHANNEL_TRANS),
    m_animTimeOffset(0.0f),
    m_animTimeScale(1.0f),
    m_ownerOp(ownerOp),
    m_changed(eTRUE),
    m_min(-eF32_MAX),
    m_max(eF32_MAX),
    m_lastTime(-1.0f)
{
    eASSERT(ownerOp != eNULL);
    eMemSet(&m_value, 0, sizeof(m_value));
}

eParameter::~eParameter()
{
#ifdef eEDITOR
    for (eU32 i=0; i<m_descrItems.size(); i++)
    {
        eSAFE_DELETE(m_descrItems[i]);
    }
#endif
}

// Returns wether or not the parameter was changed
// by this animation call. This helps to decide if
// the owning operator has to be set to changed or not.
eBool eParameter::animate(eF32 time)
{
    eASSERT(time >= 0.0f);

    // Is parameter animated or not?
    if (!m_animated || (eAreFloatsEqual(time, m_lastTime) && !m_changed))
    {
        return eFALSE;
    }

#ifdef eEDITOR
    eASSERT(isAnimatableType() == eTRUE);
    eASSERT(isAnimatable() == eTRUE);
#endif

    // Find path operator.
    eIOperator *op = eDemoData::findOperator(m_animPathOpId);
    eBool changed = eFALSE;

    if (op)
    {
        eASSERT(TEST_CATEGORY(op, "Path", Path_CID));

        // Get and evaluate path.
        const ePath &path = ((eIPathOp *)op)->getResult().path;

        // Transform time by scale and offset and loop it.
        const eF32 transTime =  eMod(eMax(0.0f, time*m_animTimeScale+m_animTimeOffset)-path.getStartTime(), path.getRunningTime());

        eVector4 av = path.process(transTime, this);

        const Value oldVal = m_value;

        // Update parameter depending on type.
        if (isFloatType())
        {
            av.clamp(m_min, m_max);
            m_value.fxyzw = av;

            changed = (av != eVector4(oldVal.fxyzw));
        }
        else if (isIntType())
        {
            av.clamp(m_min, m_max);

            const eInt nx0 = eFtoL(av.x);
            const eInt ny0 = eFtoL(av.y);
            const eInt nx1 = eFtoL(av.z);
            const eInt ny1 = eFtoL(av.w);

            eIXYXY &v = m_value.ixyxy;
            changed = (nx0 != v.x0 || ny0 != v.y0 || nx1 != v.x1 || ny1 != v.y1);

            v.x0 = nx0;
            v.y0 = ny0;
            v.x1 = nx1;
            v.y1 = ny1;
        }
    }

    if (changed)
    {
        m_changed = changed;
    }

    m_lastTime = time;
    return changed;
}

void eParameter::setChanged(eBool changed)
{
    m_changed = changed;
}

void eParameter::setMinMax(eF32 min, eF32 max)
{
    m_min = min;
    m_max = max;

    if (m_min > m_max)
    {
        eSwap(m_min, m_max);
    }
}

#ifdef eEDITOR
void eParameter::setDefault(const Value &value)
{
    m_default = value;
    m_value = value;
}
const eParameter::Value & eParameter::getDefault() const
{
	return m_default;
}
#endif

eID eParameter::getAnimationPathOpId() const
{
    return m_animPathOpId;
}

eBool eParameter::isAnimated() const
{
    return m_animated;
}

eBool eParameter::isAffectedByAnimation() const
{
    if (m_animated)
    {
        return eTRUE;
    }
    else if (m_type == TYPE_LINK)
    {
        const eIOperator *op = eDemoData::findOperator(m_value.linkedOpId);

        if (op)
        {
            return op->isAffectedByAnimation();
        }
    }
    
    return eFALSE;
}

eParameter::AnimationChannel eParameter::getAnimationChannel() const
{
    return m_animChannel;
}

eF32 eParameter::getAnimationTimeOffset() const
{
    return m_animTimeOffset;
}

eF32 eParameter::getAnimationTimeScale() const
{
    return m_animTimeScale;
}

eBool eParameter::getChanged() const
{
    return m_changed;
}

eIOperator * eParameter::getOwnerOp() const
{
    return m_ownerOp;
}

eParameter::Type eParameter::getType() const
{
    return m_type;
}

eParameter::Value & eParameter::getValue()
{
    return m_value;
}

const eParameter::Value & eParameter::getValue() const
{
    return m_value;
}

// Returns whether or not this parameter
// is a floating point type.
eBool eParameter::isFloatType() const
{
    switch (m_type)
    {
        case TYPE_FLOAT:
        case TYPE_FXY:
        case TYPE_FXYZ:
        case TYPE_FXYZW:
        case TYPE_RGB:
        case TYPE_RGBA:
        {
            return eTRUE;
        }
    }

    return eFALSE;
}

// Returns whether or not this parameter
// is a integral type.
eBool eParameter::isIntType() const
{
    switch (m_type)
    {
        case TYPE_ENUM:
        case TYPE_INT:
        case TYPE_IXY:
        case TYPE_IXYZ:
        case TYPE_IXYXY:
        {
            return eTRUE;
        }
    }

    return eFALSE;
}

eBool eParameter::isTextType() const
{
    switch (m_type)
    {
        case TYPE_STRING:
        case TYPE_TEXT:
        case TYPE_FILE:
        case TYPE_SYNTH:
        case TYPE_LABEL:
        case TYPE_TSHADERCODE:
        {
            return eTRUE;
        }
    }

    return eFALSE;
}

eBool eParameter::isAnimatableType() const
{
    return ((isIntType() || isFloatType()) && m_type != TYPE_ENUM);
}

#ifdef ePLAYER
void eParameter::load(eDemoScript &script, eU32 storePass)
{
	if (m_animated)
    {
        if(storePass == 0) {
            eU32 ac;

            script >> ac;
            if(ac & 7) { // else use default values
                m_animChannel = (AnimationChannel)((ac & 7) - 1);
                script >> m_animTimeOffset;
                script >> m_animTimeScale;
            }
            if(ac & 8) {
                script >> m_min;
                script >> m_max;
            }
        }
        return;
	}
    if(storePass == 0)
        return;

    if(m_type == TYPE_LINK)
        return;

    switch (m_type)
    {
        case TYPE_ENUM:
        case TYPE_FLAGS:
        {
            script >> m_value.flags;
            break;
        }

        case TYPE_INT:
        {
            script >> m_value.integer;
            break;
        }

        case TYPE_BOOL:
        {
            script >> m_value.boolean;
            break;
        }

        case TYPE_TSHADERCODE:
        {
            script >> m_value.byteCode;
            break;
        }

        case TYPE_STRING:
        case TYPE_TEXT:
        {
            eString str;

            script >> str;
            eStrNCopy(m_value.string, str, MAX_TEXT_LENGTH);
            break;
        }
/*
        case TYPE_FILE:
        {
            eByteArray data;
            eString fileName;

            script >> fileName;
            script >> data;

            eStrNCopy(m_value.fileName, fileName, MAX_TEXT_LENGTH);
            eDemoData::addVirtualFile(m_value.fileName, data);
            break;
        }
*/
        case TYPE_SYNTH:
        {
            eU32 isRealDemoOp;
            script >> isRealDemoOp;
            if(isRealDemoOp == 0)
                return;

            eBool songStored;
            eString songName;

            script >> songName;
            script >> songStored;

            eStrNCopy(m_value.songName, songName, MAX_TEXT_LENGTH);

            if (songStored)
            {
                eByteArray data;
                script >> data;

                eDataStream stream(&data[0], data.size());
                eDemo::getSynth().loadInstruments(stream);
                tfSong *song = eDemoData::newSong();
                song->load(stream);
            }

            break;
        }

        case TYPE_FLOAT:
        case TYPE_FXY:
        case TYPE_FXYZ:
        case TYPE_FXYZW:
        {
            eF32 *v = (eF32 *)&m_value.fxyzw;
            eASSERT(v != eNULL);

			for (eU32 i=0; i<=(eU32)m_type-TYPE_FLOAT; i++)
            {
				script >> v[i];
            }

            break;
        }

        case TYPE_IXY:
        case TYPE_IXYZ:
        case TYPE_IXYXY:
        {
            eInt *r = (eInt *)&m_value.ixyxy;
            eASSERT(r != eNULL);

            for (eU32 i=0; i<=(eU32)m_type-TYPE_IXY+1; i++)
            {
                script >> r[i];
            }

            break;
        }

        case TYPE_RGBA:
        case TYPE_RGB:
        {
            eU8 r, g, b, a;

            script >> r;
            script >> g;
            script >> b;
            a = 255;

            if (m_type == TYPE_RGBA)
            {
                script >> a;
            }

            const eColor c(r, g, b, a);
            eFloatColor &fc = m_value.color;

            fc.r = c.redF();
            fc.g = c.greenF();
            fc.b = c.blueF();
            fc.a = c.alphaF();
            break;
        }
    }
}
#else
eParameter::eParameter(Type type, const eString &name, eIOperator *ownerOp) :
    m_type(type),
    m_animated(eFALSE),
    m_animPathOpId(eNOID),
    m_animChannel(CHANNEL_TRANS),
    m_animTimeOffset(0.0f),
    m_animTimeScale(1.0f),
    m_ownerOp(ownerOp),
    m_changed(eTRUE),
    m_min(0.0f),
    m_max(255.0f),
    m_lastTime(-1.0f),
    m_animatable(isAnimatableType() != eFALSE),
    m_name(name)
{
    eASSERT(ownerOp != eNULL);
    eMemSet(&m_value, 0, sizeof(m_value));
}

void eParameter::store(eDemoScript &script, eU32 storePass) const
{
    // If parameter is animated its value doesn't
    // have to be stored.
    if (m_animated && eDemoData::findOperator(m_animPathOpId))
    {
        if(storePass == 0) {
            eU32 noDefaultMinMax = ((m_min == -eF32_MAX) && (m_max == eF32_MAX)) ? 0 : 8; 
            if((m_animChannel == CHANNEL_TRANS) && (m_animTimeOffset == 0.0f) && (m_animTimeScale == 1.0f))
                script << (eU32)(0 | noDefaultMinMax);
            else {
                script << (eU32)((m_animChannel + 1) | noDefaultMinMax);
                script << m_animTimeOffset;
                script << m_animTimeScale;
            }
            if(noDefaultMinMax) {
                script << m_min;
                script << m_max;
            }
        }
		return;
    }
    if(storePass == 0) 
        return;

    if(m_type == TYPE_LINK)
        return;

    switch (m_type)
    {
        case TYPE_ENUM:
        case TYPE_FLAGS:
        {
            script << m_value.flags;
            break;
        }

        case TYPE_INT:
        {
            script << m_value.integer;
            break;
        }

        case TYPE_BOOL:
        {
            script << m_value.boolean;
            break;
        }

        case TYPE_TSHADERCODE:
        {
            eTShaderCompiler tsc;

            if (!tsc.compile(m_value.sourceCode))
            {
                eShowError(tsc.getErrors());
            }

            eByteArray code;
            tsc.getCode(code);
            script << code;
            break;
        }

        case TYPE_STRING:
        case TYPE_TEXT:
        {
            script << m_value.string;
            break;
        }

        case TYPE_FILE:
        {
            eShowError("Warning: Script uses a file. This feature is currently disabled in loader.");
            script << m_value.fileName;

            eFile f(m_value.fileName);
            eByteArray data;

            if (f.open())
            {
                data.resize(f.getSize());
                f.read(&data[0], f.getSize());
            }

            script << data;
            break;
        }

        case TYPE_SYNTH:
        {
            if((eStrCompare(m_value.string, "") == 0)) {
                script << (eU32)0; // loader demo op
                return;
            }
            script << (eU32)1; // normal demo op

            script << m_value.songName;

            eBool songStored = eFALSE;

            for (eU32 i=0; i<eDemoData::getSongCount(); i++)
            {
                tfSong *song = eDemoData::getSongByIndex(i);
                eASSERT(song != eNULL);

                if (eStrCompare(m_value.songName, song->getUserName()) == 0)
                {
                    eDataStream stream;

                    eDemo::getSynth().storeInstruments(stream);
                    song->store(stream);

                    script << eTRUE;
                    script << stream.getData();

                    songStored = eTRUE;
                    break;
                }
            }

            if (!songStored)
            {
                script << eFALSE;
            }

            break;
        }

        case TYPE_FLOAT:
        case TYPE_FXY:
        case TYPE_FXYZ:
        case TYPE_FXYZW:
        {
            const eVector4 &v = m_value.fxyzw;

			for (eU32 i=0; i<=(eU32)m_type-TYPE_FLOAT; i++)
            {
				script << v[i];
            }

            break;
        }

        case TYPE_IXY:
        case TYPE_IXYZ:
        case TYPE_IXYXY:
        {
            const eRect &r = m_value.ixyxy;

            for (eU32 i=0; i<= (eU32)m_type-TYPE_IXY+1; i++)
            {
                script << r[i];
            }

            break;
        }

        case TYPE_RGBA:
        case TYPE_RGB:
        {
            const eColor c(m_value.color);

            script << c.red();
            script << c.green();
            script << c.blue();

            if (m_type == TYPE_RGBA)
            {
                script << c.alpha();
            }

            break;
        }
    }
}

void eParameter::setDescriptionItems(const eString &items)
{
    const eU32 itemsLen = items.length();
    eString buffer;

    for (eU32 i=0; i<itemsLen; i++)
    {
        buffer = "";

        while (i < itemsLen && items[i] != '|')
        {
            buffer += items[i++];
        }

        m_descrItems.append(new eString(buffer));
    }
}

void eParameter::setAllowedLinks(const eString &config)
{
    m_allowedLinks.parseConfig(config);
}

void eParameter::setAnimationPathOpId(eID opId)
{
    m_animPathOpId = opId;
}

void eParameter::setAnimatable(eBool animatable)
{
    eASSERT(isAnimatableType() != eFALSE);
    m_animatable = animatable;
}

void eParameter::setAnimated(eBool animated)
{
    m_animated = animated;
}

void eParameter::setAnimationChannel(AnimationChannel ac)
{
    m_animChannel = ac;
}

void eParameter::setAnimationTimeOffset(eF32 offset)
{
    m_animTimeOffset = offset;
}

void eParameter::setAnimationTimeScale(eF32 scale)
{
    m_animTimeScale = scale;
}

const eStringPtrArray & eParameter::getDescriptionItems() const
{
    return m_descrItems;
}

const eAllowedOpInput & eParameter::getAllowedLinks() const
{
    return m_allowedLinks;
}

eBool eParameter::isAnimatable() const
{
    return m_animatable;
}

const eString & eParameter::getName() const
{
    return m_name;
}

eU32 eParameter::getComponentCount() const
{
    eASSERT(m_type < TYPE_COUNT);

    // The i-th field gives the number of
    // components for the parameter type
    // with constant i.
    const eU32 COMPONENT_COUNT[TYPE_COUNT] =
    {
        1, 1, 1, 1, 1, 1, 1, 2, 3, 4, 3, 4, 2, 3, 4, 1, 1, 1, 1, 1,
    };

    return COMPONENT_COUNT[m_type];
}

void eParameter::getMinMax(eF32 &min, eF32 &max) const
{
    min = m_min;
    max = m_max;
}
#endif