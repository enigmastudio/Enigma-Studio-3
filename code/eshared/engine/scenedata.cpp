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

eSceneData::eSceneData()
    : m_renderableTotal(0)
{
}

void eSceneData::merge(eSceneData &sg, const eMatrix4x4 &mtx)
{
    eASSERT(!eIsNAN(0.0f * sg.getBoundingBox().getSize().sqrLength())); // SANITY CHECK for valid bound boxes
	ePROFILER_ZONE("Merge Scene List");

    for (eU32 i=0; i<sg.getLightCount(); i++)
    {
        m_lights.append(sg.m_lights[i]);
    }


    this->m_renderableTotal += sg.m_renderableTotal;

    Entry e;

    e.renderableList = &sg;
    e.matrix = mtx;
    e.aabb = sg.m_aabb;
    e.aabb.transform(mtx);
    e.renderableCount = sg.m_renderableTotal;

    m_aabb.merge(e.aabb);
    m_entries.append(e);
}

void eSceneData::transform(const eMatrix4x4 &mtx)
{
    this->m_aabb.clear();
    for (eU32 i=0; i<m_entries.size(); i++)
    {
        m_entries[i].matrix *= mtx;
        m_entries[i].aabb.transform(mtx);
        this->m_aabb.mergeFast(m_entries[i].aabb);
    }
    this->m_aabb.merge(this->m_aabb);
}

void eSceneData::clear()
{
    this->m_renderableTotal = 0;
    m_aabb.clear();
    m_entries.clear();
    m_lights.clear();
}

void eSceneData::addLight(const eLight *light)
{
    m_lights.append(light);
}

void eSceneData::addRenderable(const eIRenderable *ra, const eMatrix4x4 &mtx)
{
    eASSERT(!eIsNAN(0.0f * ra->getBoundingBox().getSize().sqrLength())); // SANITY CHECK for valid bound boxes
    eASSERT(ra != eNULL);

    m_renderableTotal++;

    Entry ne;
    
    ne.aabb = ra->getBoundingBox();
    eASSERT((ne.aabb.getMaximum() - ne.aabb.getMinimum()).length() > 0.0f); // empty bound boxes should not occur
    ne.matrix = mtx;
    ne.renderableObject = ra;
    ne.renderableList = eNULL;
    ne.renderableCount = 1;

    ne.aabb.transform(mtx);
    m_aabb.merge(ne.aabb);

    m_entries.append(ne);
}

eU32 eSceneData::getEntryCount() const
{
    return m_entries.size();
}

const eSceneData::Entry & eSceneData::getEntry(eU32 index) const
{
    eASSERT(index < m_entries.size());
    return m_entries[index];
}

const eAABB & eSceneData::getBoundingBox() const
{
    return m_aabb;
}

eU32 eSceneData::getLightCount() const
{
    return m_lights.size();
}

const eLight & eSceneData::getLight(eU32 index) const
{
    eASSERT(index < m_lights.size());
    return *m_lights[index];
}

eU32 eSceneData::getRenderableTotal() const 
{
	return this->m_renderableTotal;
}

void eSceneData::convertToMeshOrCount(eU32& verticeCount, eU32& faceCount, eMatrix4x4& mat, eMesh* targetMesh) const {
    for(eU32 i = 0; i < getEntryCount(); i++) {
        const eSceneData::Entry& e = getEntry(i);
        if(e.renderableList != eNULL) {
            e.renderableList->convertToMeshOrCount(verticeCount, faceCount, mat * e.matrix, targetMesh);
        } else if(e.renderableObject->getType() == eIRenderable::TYPE_MESH) {
            const eMesh::Instance& mi = (const eMesh::Instance&)*e.renderableObject;
            const eMesh& m = mi.getMesh();
            if(targetMesh)
                targetMesh->merge(m, mat, mat.toRotationMatrix());

            verticeCount += m.getVertexCount();
            faceCount += m.getPrimitiveCount();
        }
    }

}
