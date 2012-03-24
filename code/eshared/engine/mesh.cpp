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

eMesh::Instance::Instance(eMesh &mesh, eBool castsShadows) : eIRenderable(),
    m_mesh(mesh),
    m_castsShadows(castsShadows)
{
}

void eMesh::Instance::update(eF32 time)
{
}

void eMesh::Instance::getRenderJobs(const eMatrix4x4 &mtx, const eMatrix4x4 &normalMtx, eRenderJobPtrArray &jobs, eU32 passID) const
{
    ePROFILER_ZONE("Mesh get render jobs");

    jobs.reserve(jobs.size()+m_mesh.getDrawSectionCount());

    for (eU32 i=0; i<m_mesh.getDrawSectionCount(); i++)
    {
        eMesh::DrawSection &ds = m_mesh.getDrawSection(i);

        if (ds.vertices.size() != 0)
        {
            eRenderJob *job = (eRenderJob *)ds.geometry->getJobInstanciation((ePtr)ds.material, passID);

            if (job)
            {
                job->addInstance(mtx, normalMtx);
            }
            else
            {
                eRenderJob *job = eRenderJob::newRenderJob();
                eASSERT(job != eNULL);

                job->set(ds.geometry, ds.material, passID, mtx, normalMtx, eTRUE, getType());
                job->setCastsShadows(m_castsShadows);

                jobs.append(job);
            }
        }
    }
}

const eAABB & eMesh::Instance::getBoundingBox() const
{
    return m_mesh.getBoundingBox();
}

eIRenderable::Type eMesh::Instance::getType() const
{
    return TYPE_MESH;
}

eBool eMesh::Instance::getCastsShadows() const
{
    return m_castsShadows;
}

eMesh::eMesh(Type type, eU32 primitiveCount, eU32 vertexCount) :
    m_type(type)
{
    reserveSpace(primitiveCount, vertexCount);
}

eMesh::eMesh(Type type, eEditMesh &em) :
    m_type(type)
{
    fromEditMesh(em);
}

eMesh::~eMesh()
{
    clear();
}

eBool eMesh::upload()
{
    if (m_type == TYPE_STATIC)
    {
        for (eU32 i=0; i<m_drawSections.size(); i++)
        {
            DrawSection &ds = m_drawSections[i];

            if (ds.primitives.size() != 0)
            {
                _fillDynamicBuffers((ePtr)&ds, ds.geometry);
            }
        }
    }

    return eTRUE;
}

eBool eMesh::unload()
{
    return eTRUE;
}

void eMesh::reserveSpace(eU32 primitiveCount, eU32 vertexCount)
{
    m_vertices.reserve(vertexCount);
    m_primitives.reserve(primitiveCount);
}

// Has to be called after last vertex/triangle was added
// to the mesh, because creating resources in upload/unload
// functions isn't possible. Further the generation of the
// indexed triangles is too slow for dynamic meshes.
void eMesh::finishLoading(Type type)
{
    ePROFILER_ZONE("Mesh finish loading");

	m_type = type;

    if(m_vertices.size() == 0) {
        m_drawSections.clear();
        return;
    }

    m_bbox.merge(m_bbox); // calculate center and size

    eArray<eU32> posMap(m_vertices.size());

    for (eU32 i=0; i<m_drawSections.size(); i++)
    {
        DrawSection &ds = m_drawSections[i];
        if(ds.primitives.size() == 0)
            continue;
        eMemSet(&posMap[0], -1, posMap.size()*sizeof(eU32));

        const eU32 primSizes[3] = { 3, 2, 1 };
        ds.indices.reserve(ds.primitives.size()*primSizes[ds.type]);

        for (eU32 j=0, vertexCount=0; j<ds.primitives.size(); j++)
        {
            const Primitive &prim = m_primitives[ds.primitives[j]];

            for (eU32 k=0; k<primSizes[ds.type]; k++)
            {
                const eU32 vtxIndex = prim.indices[k];

                if (posMap[vtxIndex] == -1)
                {
                    posMap[vtxIndex] = vertexCount++;
                    ds.vertices.append(&m_vertices[vtxIndex]);
                }

                ds.indices.append(posMap[vtxIndex]);
            }
        }

        // Create geometry object.
        eASSERT(ds.indices.size() == ds.primitives.size()*primSizes[ds.type]);

        const ePrimitiveType primTypes[3] =
        {
            ePRIMTYPE_TRIANGLELIST,
            ePRIMTYPE_LINELIST,
            ePRIMTYPE_TRIANGLELIST
        };

        const eGeometry::Type geoType = (m_type == TYPE_DYNAMIC || ds.type == DrawSection::TYPE_POINTS ?
                                         eGeometry::TYPE_DYNAMIC_INDEXED : eGeometry::TYPE_STATIC_INDEXED);

        const eU32 indexCount = (ds.type == DrawSection::TYPE_POINTS ? ds.vertices.size()*6 : ds.indices.size());
        const eU32 vertexCount = (ds.type == DrawSection::TYPE_POINTS ? ds.vertices.size()*4 : ds.vertices.size());
        const eU32 primCount = (ds.type == DrawSection::TYPE_POINTS ? ds.primitives.size()*2 : ds.primitives.size());

        eSAFE_DELETE(ds.geometry);
        ds.geometry = new eGeometry(vertexCount, indexCount, primCount, eVTXTYPE_DEFAULT, geoType, primTypes[ds.type], _fillDynamicBuffers, &ds);
        eASSERT(ds.geometry != eNULL);
    }

    upload();
}

void eMesh::fromEditMesh(eEditMesh &em)
{
    if (em.isTriangulated())
    {
        _fromTriangleEditMesh(em);
    }
    else
    {
        eEditMesh triMesh(em);

        triMesh.triangulate();
        _fromTriangleEditMesh(triMesh);
    }
}

void eMesh::clear()
{
    m_bbox.clear();

    m_vertices.clear();
    m_primitives.clear();

    // Free all arrays (destructor isn't called,
    // because of 64k-implementation of array class).
    for (eU32 i=0; i<m_drawSections.size(); i++)
    {
        DrawSection &ds = m_drawSections[i];

        ds.primitives.free();
        ds.indices.free();
        ds.vertices.free();

        eSAFE_DELETE(ds.geometry);
    }
    
    m_drawSections.clear();
}

// NOTE: Assumes that this mesh has enough capacity
void eMesh::merge(const eMesh &other, const eMatrix4x4& mtx, const eMatrix4x4& mtxRotOnly) {
    eU32 vtxCntBefore = m_vertices.size();
    for(eU32 i = 0; i < other.getVertexCount(); i++) {
        m_vertices.append(other.getVertex(i));
        eVertex& v = m_vertices.lastElement();
        v.position = eVector3(v.position) * mtx;
        v.normal = eVector3(v.normal) * mtxRotOnly;
        m_bbox.updateExtentFast(v.position);
    }

    for(eU32 d = 0; d < other.getDrawSectionCount(); d++) {
        const DrawSection& ods = other.getDrawSection(d);
        DrawSection &ds = _findDrawSection(ods.material, ods.type);
        ds.primitives.reserve(ods.primitives.size());

        for(eU32 i = 0; i < ods.primitives.size(); i++) {
            const Primitive& oprim = other.getPrimitive(ods.primitives[i]);
            m_primitives.append(oprim);
            Primitive& prim = m_primitives.lastElement();
            prim.material = ods.material;
            for(eU32 k = 0; k < 3; k++)
                prim.indices[k] = oprim.indices[k] + vtxCntBefore;

            ds.primitives.append(m_primitives.size()-1);
        }
    }
}

eU32 eMesh::addVertexFast(const eVector3 &pos, const eVector3 &normal, const eVector2 &texCoord) {
    eVertex& v = m_vertices.pushFast();
    v.position = pos;
    v.normal = normal;
    v.texCoord = texCoord;
    v.color = eColor::WHITE.toArgb();
    m_bbox.updateExtentFast(pos);

    return m_vertices.size()-1;
}

eU32 eMesh::addVertex(const eVector3 &pos, const eVector3 &normal, const eVector2 &texCoord, const eColor &color)
{
    m_vertices.append(eVertex(pos, normal, texCoord, color));
    m_bbox.updateExtentFast(pos);

    return m_vertices.size()-1;
}

eU32 eMesh::addVertex(const eVector3 &pos, const eColor &color)
{
    return addVertex(pos, eVector3(), eVector2(), color);
}

eU32 eMesh::addTriangle(eU32 vtx0, eU32 vtx1, eU32 vtx2, const eMaterial *mat)
{
    const eU32 vertices[3] = 
    {
        vtx0, vtx1, vtx2
    };

    return _addPrimitive(vertices, 3, mat, DrawSection::TYPE_TRIANGLES);
}

eU32 eMesh::addLine(eU32 startVtx, eU32 endVtx, const eMaterial *mat)
{
    const eU32 vertices[2] = 
    {
        startVtx, endVtx
    };

    return _addPrimitive(vertices, 2, mat, DrawSection::TYPE_LINES);
}

eU32 eMesh::addLine(const eVector3 &pos0, const eVector3 &pos1, const eColor &color, const eMaterial *mat)
{
    const eU32 vtx0 = addVertex(pos0, color);
    const eU32 vtx1 = addVertex(pos1, color);

    return addLine(vtx0, vtx1, mat);
}

eU32 eMesh::addPoint(eU32 vertex, const eMaterial *mat)
{
    return _addPrimitive(&vertex, 1, mat, DrawSection::TYPE_POINTS);
}

eU32 eMesh::addPoint(const eVector3 &pos, const eColor &col, const eMaterial *mat)
{
    return addPoint(addVertex(pos, col), mat);
}

const eVertex & eMesh::getVertex(eU32 index) const
{
    eASSERT(index < m_vertices.size());
    return m_vertices[index];
}

const eMesh::Primitive & eMesh::getPrimitive(eU32 index) const
{
    eASSERT(index < m_primitives.size());
    return m_primitives[index];
}

const eMesh::DrawSection & eMesh::getDrawSection(eU32 index) const
{
    return m_drawSections[index];
}

eMesh::DrawSection & eMesh::getDrawSection(eU32 index)
{
    return m_drawSections[index];
}

eU32 eMesh::getVertexCount() const
{
    return m_vertices.size();
}

eU32 eMesh::getPrimitiveCount() const
{
    return m_primitives.size();
}

eU32 eMesh::getDrawSectionCount() const
{
    return m_drawSections.size();
}

const eAABB & eMesh::getBoundingBox() const
{
    return m_bbox;
}

void eMesh::createWireCube(eMesh &mesh, const eVector3 &size, const eMaterial *mat)
{
    eASSERT(mat != eNULL);

    const eVector3 vertices[8] =
    {
        eVector3(-0.5f, -0.5f, -0.5f), // ulh
        eVector3(-0.5f, -0.5f,  0.5f), // ulv
        eVector3( 0.5f, -0.5f,  0.5f), // urv
        eVector3( 0.5f, -0.5f, -0.5f), // urh
        eVector3(-0.5f,  0.5f, -0.5f), // olh
        eVector3(-0.5f,  0.5f,  0.5f), // olv
        eVector3( 0.5f,  0.5f,  0.5f), // orv
        eVector3( 0.5f,  0.5f, -0.5f), // orh
    };

    mesh.clear();

    for (eU32 i=0; i<8; i++)
    {
        eVector3 pos = vertices[i];
        pos.scale(size);

        mesh.addVertex(pos);
    }

    // bottom quad
    mesh.addLine(0, 1, mat);
    mesh.addLine(1, 2, mat);
    mesh.addLine(2, 3, mat);
    mesh.addLine(3, 0, mat);

    // top quad
    mesh.addLine(4, 5, mat);
    mesh.addLine(5, 6, mat);
    mesh.addLine(6, 7, mat);
    mesh.addLine(7, 4, mat);

    // four lines
    mesh.addLine(0, 4, mat);
    mesh.addLine(1, 5, mat);
    mesh.addLine(2, 6, mat);
    mesh.addLine(3, 7, mat);

    mesh.finishLoading(TYPE_DYNAMIC);
}

eU32 eMesh::addDrawSection(const eMaterial *mat, DrawSection::Type dsType)
{
    eU32 idx = _findDrawSectionIdx(mat, dsType);
    m_drawSections[idx].primitives.reserve(m_primitives.capacity());
    return idx;
}

eU32 eMesh::_findDrawSectionIdx(const eMaterial *mat, DrawSection::Type dsType)
{
    for (eU32 i=0; i<m_drawSections.size(); i++)
    {
        DrawSection &ds = m_drawSections[i];

        if (ds.material == mat && ds.type == dsType)
        {
            return i;
        }
    }

    m_drawSections.append(DrawSection());
    DrawSection &ds = m_drawSections[m_drawSections.size()-1];
    ds.type = dsType;
    ds.material = mat;
    ds.geometry = eNULL;

    return m_drawSections.size() - 1;
}

eU32 eMesh::addTriangleFast(eU32 vtx0, eU32 vtx1, eU32 vtx2, eU32 drawSectionIdx)
{
    eASSERT(drawSectionIdx < m_drawSections.size());

    DrawSection &ds = m_drawSections[drawSectionIdx];
    
    eASSERT(m_primitives.size() < m_primitives.capacity());
    Primitive& prim = m_primitives.pushFast();
    prim.material = ds.material;
    prim.indices[0] = vtx0;
    prim.indices[1] = vtx1;
    prim.indices[2] = vtx2;

    const eU32 index = m_primitives.size()-1;
    ds.primitives.append(index);
    return index;

}

eU32 eMesh::_addPrimitive(const eU32 *vertices, eU32 vertexCount, const eMaterial *mat, DrawSection::Type dsType)
{
    eASSERT(vertexCount <= 3);

    Primitive prim;
    prim.material = mat;

    for (eU32 i=0; i<vertexCount; i++)
    {
        eASSERT(vertices[i] < m_vertices.size());
        prim.indices[i] = vertices[i];
    }

    m_primitives.append(prim);
    const eU32 index = m_primitives.size()-1;
    DrawSection &ds = _findDrawSection(mat, dsType);
    ds.primitives.append(index);

    return index;
}

void eMesh::_fromTriangleEditMesh(eEditMesh &em)
{
    ePROFILER_ZONE("To renderable mesh");
	eASSERT(em.isTriangulated() == eTRUE);

    clear();
    reserveSpace(em.getFaceCount(), em.getFaceCount()*3); 
	m_bbox = em.getBoundingBox();

	for(eU32 i = 0; i < em.getVertexCount(); i++)
    {
		em.getVertex(i)->_volatile_attribute1 = (ePtr)-1;
    }

    eArray<eU32> indices;

    for (eU32 i=0; i<em.getFaceCount(); i++)
    {
        const eEditMesh::Face *face = em.getFace(i);
        eASSERT(face != eNULL);

        const eMaterial *mat = face->material;
        eASSERT(mat != eNULL);

        indices.clear();
        eEditMesh::HalfEdge *he = face->he;

        do
        {
            eEditMesh::Vertex *vtx = he->origin;
            eASSERT(vtx != eNULL);

            const eVector3 &normal = (mat->getFlatShaded() ? face->normal : vtx->normal);

            // Check if vertices have to be duplicated.
            if (!he->texCoord.equals(vtx->texCoord))
            {
                // Yes, so add new vertex.
                addVertex(vtx->position, normal, he->texCoord, vtx->color);
                indices.append(m_vertices.size()-1);
            }
            else if (mat->getFlatShaded())
            {
                addVertex(vtx->position, normal, vtx->texCoord, vtx->color);
                indices.append(m_vertices.size()-1);
            }
            // With flat shading every vertex has to be duplicated,
            // because vertices can't share normals anymore.
            else if (vtx->_volatile_attribute1 == (ePtr)-1)
            {
                // No and vertex was not added yet.
                addVertex(vtx->position, normal, vtx->texCoord, vtx->color);
                indices.append(m_vertices.size()-1);
                vtx->_volatile_attribute1 = (ePtr)(m_vertices.size()-1);
            }
            else
            {
                // Yes and vertex was already added before.
                indices.append((eU32)vtx->_volatile_attribute1);
            }
            he = he->next;
        }
        while (he != face->he);

        // Add triangle to mesh.
        addTriangle(indices[0], indices[1], indices[2], mat);
    }
}

// Forbid usage of assignment operator by making it private.
eMesh & eMesh::operator = (const eMesh &mesh)
{
    eASSERT(eFALSE);
    return *this;
}

void eMesh::_fillDynamicBuffers(ePtr param, eGeometry *geo)
{
	ePROFILER_ZONE("Upload mesh");

    DrawSection *ds = (DrawSection *)param;
    eASSERT(ds != eNULL);
    eASSERT(geo != eNULL);
    
    eVertex *vertices = eNULL;
    eU32 *indices = eNULL;

    geo->startFilling((ePtr *)&vertices, &indices);
    {
        if (ds->type == DrawSection::TYPE_POINTS)
        {
            eVector3 s, t;
    
            geo->getGraphics()->getBillboardVectors(s, t);
            const eVector3 normal = s^t;

            for (eU32 i=0, idxCount=0, vtxCount=0; i<ds->vertices.size(); i++)
            {
                const eVector3 r = s*ds->material->getPointSize();
                const eVector3 u = t*ds->material->getPointSize();

                const eVector3 pos = ds->vertices[i]->position;
                const eColor col = ds->vertices[i]->color;

                indices[idxCount++] = vtxCount+0;
                indices[idxCount++] = vtxCount+1;
                indices[idxCount++] = vtxCount+2;
                indices[idxCount++] = vtxCount+0;
                indices[idxCount++] = vtxCount+2;
                indices[idxCount++] = vtxCount+3;

                vertices[vtxCount++].set(pos-r-u, normal, eVector2(0.0f, 0.0f), col);
                vertices[vtxCount++].set(pos-r+u, normal, eVector2(0.0f, 1.0f), col);
                vertices[vtxCount++].set(pos+r+u, normal, eVector2(1.0f, 1.0f), col);
                vertices[vtxCount++].set(pos+r-u, normal, eVector2(1.0f, 0.0f), col);
            }
        }
        else
        {
            for (eU32 j=0; j<ds->vertices.size(); j++)
            {
                vertices[j] = *ds->vertices[j];
            }

            eMemCopy(indices, &ds->indices[0], ds->indices.size()*sizeof(eU32));
        }
    }
    geo->stopFilling();
}