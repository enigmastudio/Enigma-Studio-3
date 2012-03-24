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

#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

class eGeometry
{
public:
    enum Type
    {
        TYPE_STATIC,
        TYPE_DYNAMIC,
        TYPE_STATIC_INDEXED,
        TYPE_DYNAMIC_INDEXED
    };

public:
    typedef void (* FillCallback)(ePtr param, eGeometry *geo);

public:
    eGeometry(eU32 vertexCount, eU32 indexCount, eU32 primCount, eVertexType vertexType,
              Type type, ePrimitiveType primType, FillCallback fillCb=eNULL, ePtr fillCbParam=eNULL);
    ~eGeometry();

    static void             initialize(eGraphicsApiDx9 *gfx);
    static void             shutdown();

public:
    void                    startFilling(ePtr *vertices, eU32 **indices);
    void                    stopFilling(eBool countChanged=eFALSE, eU32 vertexCount=0, eU32 indexCount=0, eU32 primCount=0);
    void                    render(const eArray<eLinkedInstanceVertex>* instBuf = eNULL, eInt firstInstanceIdx = 0, eInt instCount = 0);

    eGraphicsApiDx9 *         getGraphics() const;

private:

    static void             _createInstanceBuffer();
    static void             _createDynamicBuffers();

private:
    void                    _lockInstanceBuffer(eInstanceVertex **instances, eU32 instCount);
    void                    _unlockInstanceBuffer();
    void                    _lockDynamicBuffers(ePtr *vertices, eU32 **indices);
    void                    _unlockDynamicBuffers();

    void                    _createStaticBuffers();
    void                    _freeStaticBuffers();
    void                    _lockStaticBuffers(ePtr *vertices, eU32 **indices);
    void                    _unlockStaticBuffers();

private:
    static const eU32       INSTANCE_VB_ELEMENTS = 100000;
    static const eU32       DYNAMIC_VB_ELEMENTS  = 100000;
    static const eU32       DYNAMIC_IB_ELEMENTS  = 1000000;

private:
    static eU32             m_instVbPos;
    static eU32             m_dynVbPos;
    static eU32             m_dynIbIndex;

    static eIVertexBuffer * m_instanceVb;
    static eIVertexBuffer * m_dynamicVb;
    static eIIndexBuffer *  m_dynamicIb;

    static eGraphicsApiDx9 *  m_gfx;

private:
    eBool                   m_loading;

    Type                    m_type;
    eBool                   m_indexed;
    eBool                   m_dynamic;
    ePrimitiveType          m_primType;
    eVertexType             m_vertexType;
    eU32                    m_vertexSize;

    eU32                    m_vertexCount;
    eU32                    m_indexCount;
    eU32                    m_primCount;

    eU32                    m_usedVertexCount;
    eU32                    m_usedIndexCount;
    eU32                    m_usedPrimCount;

    eU32                    m_lastDynVbFill;
    eU32                    m_lastDynIbFill;

    eIIndexBuffer *         m_staticIb;
    eIVertexBuffer *        m_staticVb;

    FillCallback            m_fillCb;
    ePtr                    m_cbParam;

public:
    void addInstanciation(void* mat, void* job, eU32 passID) {
        tRenderInstance inst;
        inst.job = job;
        inst.mat = mat;
        inst.passID = passID;
        this->m_renderInstances.append(inst);
    }
    
    eInt findInstanciation(const void* mat, eU32 passID) {
        for(eU32 i = 0; i < this->m_renderInstances.size(); i++)
            if((this->m_renderInstances[i].mat == mat) && (this->m_renderInstances[i].passID == passID))
               return i;
        return -1;
    }

    void removeInstanciation(const void* mat, eU32 passID);

    void* getJobInstanciation(void* mat, eU32 passID) {
        eInt idx = this->findInstanciation(mat, passID);
        if(idx != -1) 
            return this->m_renderInstances[idx].job;
        else
            return eNULL;
    }
    void removeInstanciations() {
        while(this->m_renderInstances.size() > 0)
            this->removeInstanciation(this->m_renderInstances[0].mat, this->m_renderInstances[0].passID);
    }
private:
    struct tRenderInstance {
        void* mat;
        void* job;
        eU32  passID;
    };
    eArray<tRenderInstance> m_renderInstances;
};

#endif // GEOMETRY_HPP