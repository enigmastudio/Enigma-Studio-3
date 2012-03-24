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

#ifndef IOPERATOR_HPP
#define IOPERATOR_HPP

// Base class for all operator categories.
typedef eArray<class eIOperator *> eIOperatorPtrArray;
typedef eArray<const class eIOperator *> eIOpConstPtrArray;

class eIOperator
{
    friend class eOperatorPage;
    friend class eDemoData;
    friend class eDemoOp;

public:
    // Base class of operator results. Base class
    // is needed for results, because getResult()
    // is virtual and so covariant return types
    // are needed.
    struct Result
    {
    };

#ifdef eEDITOR
    struct MetaInfos
    {
        eString                 category;
        eString                 name;
        eColor                  color;
        eChar                   shortcut;
        eU32                    minInput;
        eU32                    maxInput;
        eAllowedOpInput         allowedInput;
        eString                 classNameString;
        eString                 sourceFileName;
        eString                 type;
        ePtr                    execFunc;
    };
#endif

public:
    eIOperator();
#ifdef eEDITOR
    virtual ~eIOperator();
#endif

    virtual eBool               process(eIRenderer *renderer, eF32 time, eBool (*callback)(eIRenderer *renderer, eU32 processed, eU32 total, ePtr param)=eNULL, ePtr param=eNULL);

#ifdef ePLAYER
    void                        load(eDemoScript &script);
#else
    void                        store(eDemoScript &script) const;
#endif

    void                        setChanged();
    void                        setPosition(const ePoint &pos);
    void                        setWidth(eU32 width);
    void                        setBypassed(eBool bypass);
    void                        setHidden(eBool hidden);

    eU32                        getParameterCount() const;
    eParameter &                getParameter(eU32 index);
    const eParameter &          getParameter(eU32 index) const;
    eU32                        getInputCount() const;
    eIOperator *                getInputOperator(eU32 index) const;
    eU32                        getOutputCount() const;
    eIOperator *                getOutputOperator(eU32 index) const;
    eU32                        getLinkingCount() const;
    eID                         getLinkingOperator(eU32 index) const;
    eID                         getId() const;
    eOperatorPage *             getOwnerPage() const;
    eBool                       getChanged() const;
    ePoint                      getPosition() const;
    eU32                        getWidth() const;
    eBool                       getBypassed() const;
    eBool                       getHidden() const;

    eBool                       isAffectedByAnimation() const;
    void                        getOpsInStack(eIOperatorPtrArray &ops);

#ifdef eEDITOR
    virtual const MetaInfos &   getMetaInfos() const;

public:
    eParameter &                addParameter(eParameter::Type type, const eString &name, const eString &items, eU32 index=0);
    eParameter &                addParameter(eParameter::Type type, const eString &name, eF32 min, eF32 max, const eVector4 &v);

    void                        setUserName(const eString &userName);

    eBool                       getFreeInputPosition(ePoint &pos) const;
    void                        getMinMaxInput(eU32 &minInput, eU32 &maxInput) const;
    eChar                       getShortcut() const;
    eColor                      getColor() const;
    eBool                       getValid() const;
    eParameter &                getParameter(eString str);

public:
    virtual void                doEditorInteraction(eGraphicsApiDx9 *gfx, eSceneData &sg);
    virtual eBool               checkValidity() const;
    virtual const eString &     getUserName() const;
    virtual void                optimizeForExport();
#else
    eParameter &                addParameter(eParameter::Type type);
#endif

    virtual const Result &      getResult() const = 0;
    virtual Result &            getResult() = 0;
#ifdef eEDITOR
    virtual const eString &     getCategory() const;
    virtual const eString &     getName() const;
    virtual const eString &     getType() const;
    virtual const eString &     getRealType() const;
#else
    eU32                        m_metaOpID;
    eU32                        m_metaCategoryID;
    ePtr                        m_metaExecFunc;
#endif

protected:
    void                        _initialize();
    void                        _deinitialize();

private:
    void                        _callExecute(eGraphicsApiDx9 *gfx);
    void                        _animateParameters(eF32 time);
    void                        _clearParameters();
    void                        _getOpsInStackInternal(eIOperatorPtrArray &ops);
    eID                         _generateNewId() const;

private:
    virtual void                _preExecute(eGraphicsApiDx9 *gfx);

private:
    eID                         m_id;
    eOperatorPage *             m_ownerPage;
    ePoint                      m_position;
    eU32                        m_width;
    eIOperatorPtrArray          m_inputOps;
    eIOperatorPtrArray          m_outputOps;
    eIdArray                    m_linkingOps; // Operators which are linking me.
    eBool                       m_changed;
    eBool                       m_bypassed;
    eBool                       m_hidden;
    eParameterPtrArray          m_params;
    eBool                       m_visited;

#ifdef eEDITOR    
    mutable eBool               m_valid;
    mutable eBool               m_checkValidity;

    eString                     m_userName;

protected:
    MetaInfos *                 m_metaInfos;
#endif
};

#ifdef eEDITOR    
// Class holding all operator meta information.
class eOpMetaInfoManager
{
public:
    typedef eArray<const eIOperator::MetaInfos *> MetaInfosPtrArray;

public:
    static void addMetaInfos(const eIOperator::MetaInfos *mi)
    {
        eASSERT(mi != eNULL);
        getMetaInfos().append(mi);
    }

    static const eIOperator::MetaInfos & getInfos(eU32 index)
    {
        return *getMetaInfos()[index];
    }

    static eU32 getInfosCount()
    {
        return getMetaInfos().size();
    }

private:
    static MetaInfosPtrArray & getMetaInfos()
    {
        static MetaInfosPtrArray allMetaInfos;
        return allMetaInfos;
    }
};

// Declare operator factory, used to create
// instances of operators based on their IDs.
typedef eFactory<eIOperator, eString> eOpFactory;
#endif

#endif // IOPERATOR_HPP