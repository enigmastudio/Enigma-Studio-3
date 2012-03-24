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

#include "../system/system.hpp"
#include "../math/math.hpp"
#include "engine.hpp"

eU32 eRenderJob::_instanceCnt = 0;
eArray<eLinkedInstanceVertex>  eRenderJob::_instances;

eU32 eRenderJob::_jobCnt = 0;
eArray<eRenderJob*>            eRenderJob::_jobs;

eU32 eRenderJob::_jobInstancingClearStart = 0;

eRenderJob::eRenderJob() {
}

eRenderJob* eRenderJob::newRenderJob() {
    if(_jobCnt < _jobs.size())
        return _jobs[_jobCnt++];

    _jobs.append(new eRenderJob());
    return _jobs[_jobCnt++];
}

eLinkedInstanceVertex& eRenderJob::newInstance() {
    if(_instanceCnt < _instances.size())
        return _instances[_instanceCnt++];

    _instances.append(eLinkedInstanceVertex());
    return _instances[_instanceCnt++];
}


void eRenderJob::set(eGeometry *geometry, const eMaterial *material, eU32 passID, const eMatrix4x4 &modelMtx, const eMatrix4x4 &normalMtx, eBool useInstancing, eInt type) {
    eASSERT(geometry != eNULL);
    eASSERT(material != eNULL);
    m_geometry = geometry;
    m_material = material;
    m_useInstancing = useInstancing;
    m_type = type;
    m_materialIndex = 0;
    m_castsShadows = eTRUE;
    m_numInstances = 0;
    m_firstInstanceIdx = -1;
    m_passID = passID;

    geometry->addInstanciation((void*)material, this, passID);
    addInstance(modelMtx, normalMtx);
}

eRenderJob::~eRenderJob() {
//    m_geometry->removeInstanciation((void*)m_material);
}

eBool eRenderJob::canBeInstanced(const eGeometry *geo, const eMaterial *mat) const
{
    eASSERT(geo != eNULL);
    eASSERT(mat != eNULL);

    return (m_geometry == geo && m_material == mat);
}

void eRenderJob::render(eGraphicsApiDx9 *gfx, const eCamera &cam, eInt renderWhat, eInt renderFlags) const
{
    eASSERT(gfx != eNULL);
//    eASSERT(m_instances.size() > 0);

    // Check weather or not the render job has
    // to be rendered.
    if (m_material->getUseBlending() && !(renderWhat & ALPHA_ON))
    {
        return;
    }
    else if (!m_material->getUseBlending() && !(renderWhat & ALPHA_OFF))
    {
        return;
    }

    if (m_material->getUseRefraction() && !(renderWhat & REFRACTED_ON))
    {
        return;
    }
    else if (!m_material->getUseRefraction() && !(renderWhat & REFRACTED_OFF))
    {
        return;
    }

    if (m_material->getLighted() && !(renderWhat & LIGHTED_ON))
    {
        return;
    }
    else if (!m_material->getLighted() && !(renderWhat & LIGHTED_OFF))
    {
        return;
    }

    if (m_castsShadows && !(renderWhat & CASTSHADOW_ON))
    {
        return;
    }
    else if (!m_castsShadows && !(renderWhat & CASTSHADOW_OFF))
    {
        return;
    }

    // Render the geometry.
    const eBool useMats = !(renderFlags & MATERIALS_OFF);
    const eBool allowAlpha = !(renderFlags & BLENDING_OFF);

    if (useMats)
    {
        m_material->activate(gfx, allowAlpha);
    }

    gfx->setPsConst(ePSCONST_MAT_INDEX, (eF32)m_materialIndex/256.0f);
    cam.activate(gfx, _instances[this->m_firstInstanceIdx].vtx.modelMtx);

    // FIXME: instancing currently may remove the geometry reference
    if(m_geometry) {
        if (m_useInstancing)
        {
            m_geometry->render(&_instances, this->m_firstInstanceIdx, this->m_numInstances);
        }
        else
        {
            m_geometry->render();
        }
    }
}

void eRenderJob::addInstance(const eMatrix4x4 &modelMtx, const eMatrix4x4 &normalMtx)
{
    eASSERT(m_useInstancing == eTRUE);

    this->m_numInstances++;

    eLinkedInstanceVertex& vtx = newInstance();
    vtx.vtx.modelMtx = modelMtx;
    vtx.vtx.normalMtx = normalMtx;
    vtx.next = this->m_firstInstanceIdx;

    this->m_firstInstanceIdx = eRenderJob::_instanceCnt - 1;
}

void eRenderJob::resetInstancing() {
    for(eU32 i = _jobInstancingClearStart; i < _jobCnt; i++)
        if(_jobs[i]->m_geometry != eNULL)
            _jobs[i]->m_geometry->removeInstanciation(_jobs[i]->getMaterial(), _jobs[i]->getPassID());
    _jobInstancingClearStart = _jobCnt;
}

void eRenderJob::reset()
{
    eRenderJob::resetInstancing();
    _jobInstancingClearStart = 0;
    _instanceCnt = 0;
    _jobCnt = 0;
}

void eRenderJob::setMaterialIndex(eU32 index)
{
    m_materialIndex = index;
}

void eRenderJob::setCastsShadows(eBool castsShadows)
{
    if (!castsShadows)
    {
        m_castsShadows = castsShadows;
    }
}

eGeometry * eRenderJob::getGeometry() const
{
    return m_geometry;
}

const eMaterial * eRenderJob::getMaterial() const
{
    return m_material;
}
/*
const eInstanceVtxArray & eRenderJob::getInstances() const
{
    return m_instances;
}
*/
eInt eRenderJob::getType() const
{
    return m_type;
}

eU32 eRenderJob::getMaterialIndex() const
{
    return m_materialIndex;
}

eBool eRenderJob::getCastsShadows() const
{
    return m_castsShadows;
}

void eRenderJob::sortJobs(eRenderJobPtrArray &jobs)
{
    ePROFILER_ZONE("Sort render jobs");

    static eRenderJobPtrArray partition[256];

    for (eU32 i=0; i<32; i+=8)
    {
        // Do partition phase.
        for (eU32 j=0; j<jobs.size(); j++)
        {
            eRenderJob *job = jobs[j];
            eASSERT(job != eNULL);

            const eU32 key = job->getMaterial()->getSortKey();
            const eU32 slot = (key>>i)&0x000000ff;

            partition[slot].append(job);
        }

        // Do collection phase.
        for (eU32 j=0, l=0; j<256; j++)
        {
            for (eU32 k=0; k<partition[j].size(); k++)
            {
                jobs[l++] = partition[j][k];
            }

            partition[j].clear();
        }
    }
}