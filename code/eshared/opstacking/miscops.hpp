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

#ifndef MISC_OPS_HPP
#define MISC_OPS_HPP

// Collects sequencer operators and makes
// a demo out of them.
class eIDemoOp : public eIOperator
{
public:
    struct Result : public eIOperator::Result
    {
        Result(eDemo &nd) : demo(nd)
        {
        }

        eDemo & demo;
    };

public:
    eIDemoOp() :
        m_result(m_demo),
        m_processAll(eFALSE)
    {
    }

    void setProcessAll(eBool processAll)
    {
        m_processAll = processAll;
    }

    eBool getProcessAll() const
    {
        return m_processAll;
    }

public:
    virtual const Result & getResult() const
    {
        return m_result;
    }

    virtual Result & getResult()
    {
        return m_result;
    }

protected:
    eDemo   m_demo;
    Result  m_result;
    eBool   m_processAll;
};

// Renders a scene into a texture which can
// then be used inside the material operator.
class eIRenderToTextureOp : public eIOperator
{
public:
    struct Result : public eIOperator::Result
    {
        Result(eITexture2d *&rt, eITexture2d *&dt) :
            renderTarget(rt),
            depthTarget(dt)
        {
        }

        eITexture2d *&  renderTarget;
        eITexture2d *&  depthTarget;
    };

public:
    eIRenderToTextureOp() :
        m_renderTarget(eNULL),
        m_depthTarget(eNULL),
        m_result(m_renderTarget, m_depthTarget)
    {
    }

    virtual const Result & getResult() const
    {
        return m_result;
    }

    virtual Result & getResult()
    {
        return m_result;
    }

protected:
    Result          m_result;
    eITexture2d *   m_renderTarget;
    eITexture2d *   m_depthTarget;
};

// Material for meshes.
class eIMaterialOp : public eIOperator
{
public:
    struct Result : public eIOperator::Result
    {
        Result(eMaterial &mat) : material(mat)
        {
        }

        eMaterial & material;
    };

public:
    eIMaterialOp() : m_result(m_mat)
    {
    }

    virtual const Result & getResult() const
    {
        return m_result;
    }

    virtual Result & getResult()
    {
        return m_result;
    }

protected:
    eMaterial   m_mat;
    Result      m_result;
};

// Scene operator between model and effect operators.
// Holds a scene with a spatial subdivision data structure.
class eISceneOp : public eIOperator
{
public:
    struct Result : public eIOperator::Result
    {
        Result(eScene &sc) : scene(sc)
        {
        }

        eScene & scene; 
    };

public:
    eISceneOp() : m_result(m_scene)
    {
    }

    virtual const Result & getResult() const
    {
        return m_result;
    }

    virtual Result & getResult()
    {
        return m_result;
    }

protected:
    eScene  m_scene;
    Result  m_result;
};

// Base class for all structure operators
// (load, store, nop).
class eIStructureOp : public eIOperator
{
public:
    virtual const Result & getResult() const
    {
        return _getRealOp()->getResult();
    }

    virtual Result & getResult()
    {
        return _getRealOp()->getResult();
    }


#ifdef eEDITOR
    virtual const eString & getCategory() const
    {
        const eIOperator *op = _getRealOp();
        return (op ? op->getCategory() : eIOperator::getCategory());
    }

    virtual const eString & getType() const
    {
        const eIOperator *op = _getRealOp();
        return (op ? op->getType() : eIOperator::getType());
    }

    virtual const eString & getRealType() const
    {
        return eIOperator::getType();
    }

    virtual const MetaInfos &   getMetaInfos() const 
    {
        const eIOperator *op = _getRealOp();
        return (op ? op->getMetaInfos() : eIOperator::getMetaInfos());
    }
#endif

protected:
    virtual eIOperator * _getRealOp() const
    {
        return (getInputCount() > 0 ? getInputOperator(0) : eNULL);
    }
};

// Generic operator.
class eIGenericOp : public eIOperator
{
public:
    struct Result : public eIOperator::Result
    {
        Result(void* &gd) : genericDataPtr(gd)
        {
        }

        (void*) & genericDataPtr;
    };

public:
    eIGenericOp() : m_result(m_genData)
    {
    }

    virtual const Result & getResult() const
    {
        return m_result;
    }

    virtual Result & getResult()
    {
        return m_result;
    }

protected:
    void*   m_genData;
    Result  m_result;
};


#endif // MISC_OPS_HPP