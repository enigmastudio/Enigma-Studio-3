/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *   This file is part of
 *       _______   ______________  ______     _____
 *      / ____/ | / /  _/ ____/  |/  /   |   |__  /
 *     / __/ /  |/ // // / __/ /|_/ / /| |    /_ <
 *    / /___/ /|  // // /_/ / /  / / ___ |  ___/ /
 *   /_____/_/ |_/___/\____/_/  /_/_/  |_| /____/.
 *
 *   Copyright  2003-2010 Brain Control, all rights reserved.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef PARAMETER_HPP
#define PARAMETER_HPP

class eIOperator;
class eDemoScript;
class eAllowedOpInput;

class eParameter
{
public:
    enum Type
    {
        TYPE_INT,
        TYPE_ENUM,
        TYPE_FLAGS,
        TYPE_BOOL,
        TYPE_STRING,
        TYPE_TEXT,
        TYPE_FLOAT,
        TYPE_FXY,
        TYPE_FXYZ,
        TYPE_FXYZW,
        TYPE_RGB,
        TYPE_RGBA,
        TYPE_IXY,
        TYPE_IXYZ,
        TYPE_IXYXY,
        TYPE_LABEL,
        TYPE_LINK,
        TYPE_TSHADERCODE,
        TYPE_FILE,
        TYPE_SYNTH,
        TYPE_COUNT // Number of parameter types.
    };

    enum AnimationChannel
    {
        CHANNEL_ROT,
        CHANNEL_TRANS,
        CHANNEL_SCALE
    };

public:
    static const eU32       MAX_TEXT_LENGTH = 512;

public:
    struct Value
    {
        union
        {
            eInt            integer;
            eF32            flt;
            eInt            enumSel;
            eU8             flags;
            eBool           boolean;
            eChar           string[MAX_TEXT_LENGTH];
            eChar           text[MAX_TEXT_LENGTH];
            eChar           fileName[MAX_TEXT_LENGTH];
            eChar           songName[MAX_TEXT_LENGTH];
            eChar           label[MAX_TEXT_LENGTH];
            eChar           sourceCode[MAX_TEXT_LENGTH];
            eFXY            fxy;
            eFXYZ           fxyz;
            eFXYZW          fxyzw;
            eFloatColor     color;
            eIXY            ixy;
            eIXYZ           ixyz;
            eIXYXY          ixyxy;
            eID             linkedOpId;
                        
            // Bitwise access to flags.
            struct
            {
                eU8         flag0 : 1;
                eU8         flag1 : 1;
                eU8         flag2 : 1;
                eU8         flag3 : 1;
                eU8         flag4 : 1;
                eU8         flag5 : 1;
                eU8         flag6 : 1;
                eU8         flag7 : 1;
            };
        };

        eByteArray          byteCode;
    };

public:
    eParameter(Type type, eIOperator *ownerOp);
    ~eParameter();

    eBool                   animate(eF32 time);

    void                    setChanged(eBool changed);
    void                    setMinMax(eF32 min, eF32 max);
    void                    setDefault(const Value &value);

    eID                     getAnimationPathOpId() const;
    eBool                   isAnimated() const;
    eBool                   isAffectedByAnimation() const;
    AnimationChannel        getAnimationChannel() const;
    eF32                    getAnimationTimeOffset() const;
    eF32                    getAnimationTimeScale() const;
    eBool                   getChanged() const;
    eIOperator *            getOwnerOp() const;
    Type                    getType() const;
    Value &                 getValue();
    const Value &           getValue() const;
    eBool                   isFloatType() const;
    eBool                   isIntType() const;
    eBool                   isTextType() const;
    eBool                   isAnimatableType() const;
	const Value &			getDefault() const;

#ifdef ePLAYER
    void                    load(eDemoScript &script, eU32 storePass);
#else
    eParameter(Type type, const eString &name, eIOperator *ownerOp);

    void                    store(eDemoScript &script, eU32 storePass) const;

    void                    setDescriptionItems(const eString &items);
    void                    setAllowedLinks(const eString &config);
    void                    setAnimationPathOpId(eID opId);
    void                    setAnimatable(eBool animatable);
    void                    setAnimated(eBool animated);
    void                    setAnimationChannel(AnimationChannel ac);
    void                    setAnimationTimeOffset(eF32 offset);
    void                    setAnimationTimeScale(eF32 scale);

    const eStringPtrArray & getDescriptionItems() const;
    const eAllowedOpInput & getAllowedLinks() const;
    eBool                   isAnimatable() const;
    const eString &         getName() const;
    eU32                    getComponentCount() const;
    void                    getMinMax(eF32 &min, eF32 &max) const;
#endif

public:
    Type                    m_type;
    eU32                    m_animated;
    eID                     m_animPathOpId;
    AnimationChannel        m_animChannel;
    eF32                    m_animTimeOffset;
    eF32                    m_animTimeScale;
    Value                   m_value;
    eIOperator *            m_ownerOp;
    eBool                   m_changed;
    eF32                    m_min;
    eF32                    m_max;
    eF32                    m_lastTime;

#ifdef eEDITOR
    Value                   m_default;
    eStringPtrArray         m_descrItems;
    eAllowedOpInput         m_allowedLinks;
    eBool                   m_animatable;
    eString                 m_name;
#endif

#if defined(eEDITOR) || defined(eDEBUG)
    static const eU32       M_RELEVANT_PARAMETER_LENGTH[];
#endif
};

typedef eArray<eParameter *> eParameterPtrArray;

#endif // PARAMETER_HPP