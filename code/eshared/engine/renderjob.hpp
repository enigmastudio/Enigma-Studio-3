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

#ifndef RENDER_JOB_HPP
#define RENDER_JOB_HPP

typedef eArray<class eRenderJob *> eRenderJobPtrArray;

class eRenderJob
{
public:
    enum RenderFlags
    {
        MATERIALS_OFF   = 1,
        BLENDING_OFF    = 2
    };

    enum RenderWhat
    {
        ALPHA_ON        = 1,
        ALPHA_OFF       = 2,
        ALPHA_BOTH      = ALPHA_ON | ALPHA_OFF,
        
        REFRACTED_ON    = 4,
        REFRACTED_OFF   = 8,
        REFRACTED_BOTH  = REFRACTED_ON | REFRACTED_OFF,
        
        LIGHTED_ON      = 16,
        LIGHTED_OFF     = 32,
        LIGHTED_BOTH    = LIGHTED_ON | LIGHTED_OFF,
        
        CASTSHADOW_ON   = 64,
        CASTSHADOW_OFF  = 128,
        CASTSHADOW_BOTH = CASTSHADOW_ON | CASTSHADOW_OFF,

        RENDER_ALL      = ALPHA_BOTH | REFRACTED_BOTH | LIGHTED_BOTH | CASTSHADOW_BOTH,
    };

public:
    eRenderJob();
    ~eRenderJob();

    static eRenderJob*          newRenderJob();
    static eLinkedInstanceVertex& newInstance();
    void                        set(eGeometry *geo, const eMaterial *mat, eU32 passID, const eMatrix4x4 &modelMtx, const eMatrix4x4 &normalMtx, eBool useInstancing, eInt type);
    eBool                       canBeInstanced(const eGeometry *geo, const eMaterial *mat) const;

    void                        render(eGraphicsApiDx9 *gfx, const eCamera &cam, eInt renderWhat, eInt renderFlags=0) const;
    void                        addInstance(const eMatrix4x4 &modelMtx, const eMatrix4x4 &normalMtx);
    static void                 reset();
    static void                 resetInstancing();
    void                        setMaterialIndex(eU32 index);
    void                        setCastsShadows(eBool castsShadows);

    eGeometry *                 getGeometry() const;
    const eMaterial *           getMaterial() const;
//    const eInstanceVtxArray &   getInstances() const;
    eInt                        getType() const;
    eU32                        getMaterialIndex() const;
    eBool                       getCastsShadows() const;
    eU32                        getPassID() const { return this->m_passID; };
    
    void                        setGeometry(eGeometry* geometry) { this->m_geometry = geometry; };

public:
    static void                 sortJobs(eRenderJobPtrArray &jobs);

private:
    eGeometry *                 m_geometry;
    const eMaterial *           m_material;
    eBool                       m_useInstancing;

    eInt                        m_type;
    eU32                        m_materialIndex;
    eBool                       m_castsShadows;

    eInt                        m_firstInstanceIdx;
    eInt                        m_numInstances;
    eU32                        m_passID;
    static eArray<eLinkedInstanceVertex>    _instances;
    static eU32                             _instanceCnt;
    static eArray<eRenderJob*>              _jobs;
    static eU32                             _jobCnt;
    static eU32                             _jobInstancingClearStart;
};

#endif // RENDER_JOB_HPP