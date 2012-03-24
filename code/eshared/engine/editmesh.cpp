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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/glu.h>

#include "../system/system.hpp"
#include "../math/math.hpp"
#include "engine.hpp"
/*
#define ALLOCATE_OPT(type, target, preallocList)			type* target = new type
#define DEALLOCATE_OPT(target)								eSAFE_DELETE(target)
/**/

// optimized allocation
#define ALLOCATE_OPT(type, target, preallocList)			\
	type* target = eNULL;									\
	if(preallocList.size() < preallocList.capacity()) {		\
		preallocList.resize(preallocList.size() + 1);		\
		target = &preallocList[preallocList.size() - 1];	\
		target->isPreallocated = true;						\
	} else {												\
		target = new type;									\
		target->isPreallocated = false;						\
	}														

// optimized allocation, assumes that target is type**
#define ALLOCATE_OPT_MULTIPLE(type, target, preallocList, count)		\
	if(preallocList.size() + count <= preallocList.capacity()) {		\
		preallocList.resize(preallocList.size() + count);				\
		for(int _i = 0; _i < count; _i++) {								\
			target[_i] = &preallocList[preallocList.size() - count + _i];\
			target[_i]->isPreallocated = true;							\
		}																\
	} else {															\
		for(int _i = 0; _i < count; _i++) {								\
			target[_i] = new type;										\
			target[_i]->isPreallocated = false;							\
		}																\
	}														

#define DEALLOCATE_OPT(target)			\
{										\
	if((target != eNULL) && (!(target)->isPreallocated))		\
	    eSAFE_DELETE(target);			\
}
/**/
// Implementation of triangulator.

eTriangulator::eTriangulator() : m_totalVtxCount(0)
{
}

eTriangulator::~eTriangulator()
{
    clearContours();
}

eBool eTriangulator::triangulate()
{
    m_triIndices.clear();
    m_triIndices.reserve(m_totalVtxCount*3);

    GLUtesselator *tess = gluNewTess();
    eASSERT(tess != eNULL);

    gluTessCallback(tess, GLU_TESS_EDGE_FLAG_DATA, (void (eCALLBACK *)())_edgeFlagCallback);
    gluTessCallback(tess, GLU_TESS_VERTEX_DATA, (void (eCALLBACK *)())_vertexCallback);
    gluTessCallback(tess, GLU_TESS_ERROR_DATA, (void (eCALLBACK *)())_errorCallback);
    gluTessCallback(tess, GLU_TESS_COMBINE_DATA, (void (eCALLBACK *)())_combineCallback);

    gluTessBeginPolygon(tess, this);

    for (eU32 i=0, vtxNum=0; i<m_contours.size(); i++)
    {
        const eVector3Array &contour = *m_contours[i];
		if(contour.isEmpty())
			continue;

        gluTessBeginContour(tess);

        for (eU32 j=0; j<contour.size(); j++)
        {
            eF64 coords[] =
            {
                contour[j].x,
                contour[j].y,
                contour[j].z,
            };

            gluTessVertex(tess, coords, (ePtr)vtxNum);
            vtxNum++;
        }

        gluTessEndContour(tess);
    }

    gluTessEndPolygon(tess);

    eASSERT(m_triIndices.size()%3 == 0);
    return eTRUE;
}

void eTriangulator::addContour(const eVector3Array &contour)
{
    //eASSERT(contour.size() >= 3);

    m_vertices.append(contour);
    m_contours.append(new eVector3Array(contour));
    m_totalVtxCount += contour.size();
}

void eTriangulator::clearContours()
{
    for (eU32 i=0; i<m_contours.size(); i++)
    {
        eSAFE_DELETE(m_contours[i]);
    }

    m_contours.clear();
    m_totalVtxCount = 0;
}

const eArray<eU32> & eTriangulator::getIndices() const
{
    return m_triIndices;
}

const eVector3Array & eTriangulator::getVertices() const
{
    return m_vertices;
}

void eCALLBACK eTriangulator::_vertexCallback(eU32 vtxIndex, eTriangulator *trg)
{
    eASSERT(trg != eNULL);
    trg->m_triIndices.append(vtxIndex);
}

void eCALLBACK eTriangulator::_errorCallback(eU32 errNo, eTriangulator *trg)
{
    eASSERT(eFALSE);
}

// Has to be defined even that it doesn't do anything,
// in order to make the GLU tesseleator outputting
// only simple triangles (and no triangle fans or strips).
void eCALLBACK eTriangulator::_edgeFlagCallback(eU8 flag, eTriangulator *trg)
{
}

void eCALLBACK eTriangulator::_combineCallback(double newVert[3], eU32 neighbourVert[4], eF32 neighborWeight[4], eU32 *index, eTriangulator *trg)
{
    eASSERT(trg != eNULL);
    eASSERT(index != eNULL);

    trg->m_vertices.append(eVector3((eF32)newVert[0], (eF32)newVert[1], (eF32)newVert[2]));
    *index = trg->m_vertices.size()-1;
}

// Implementation of topology and helper classes.

eBool eEditMesh::Vertex::isFree() const
{
    if (he == eNULL)
    {
        return eTRUE;
    }

    HalfEdge *curHe = he;

    do
    {
        if (curHe->face == eNULL)
        {
            return eTRUE;
        }

        curHe = curHe->twin->next;
    }
    while (curHe != he);

    return eFALSE;
}

void				
eEditMesh::Vertex::lerp(const Vertex& v0, const Vertex& v1, eF32 t) {
	this->position = v0.position.lerp(t, v1.position);
	this->normal = v0.normal.lerp(t, v1.normal);
	this->texCoord = v0.texCoord.lerp(v1.texCoord, t);
}

void				
eEditMesh::Vertex::add(const eEditMesh::Vertex& other) {
	this->position += other.position;
	this->normal += other.normal;
	this->texCoord += other.texCoord;
}

void				
eEditMesh::Vertex::mul(eF32 val) {
	this->position *= val;
	this->normal *= val;
	this->texCoord *= val;
}


eBool eEditMesh::Vertex::isBoundary() const
{
    if (he == eNULL)
    {
        return eTRUE;
    }

    HalfEdge *curHe = he;

    do
    {
        if (curHe->isBoundary())
        {
            return eTRUE;
        }

        curHe = curHe->twin->next;
    }
    while (curHe != he);

    return eFALSE;
}

eBool eEditMesh::Edge::isBoundary() const
{
    return (!he0->face || !he1->face);
}

void eEditMesh::Face::updateNormal()
{
    eASSERT(he != eNULL);

    const eVector3 &pos0 = he->prev->origin->position;
    const eVector3 &pos1 = he->origin->position;
    const eVector3 &pos2 = he->next->origin->position;

    normal = (pos2-pos1)^(pos0-pos1);
    normal.normalize();
}

eBool eEditMesh::Face::isBoundary() const
{
    if (he == eNULL)
    {
        return eTRUE;
    }

    HalfEdge *curHe = he;

    do
    {
        if (curHe->twin->face == eNULL)
        {
            return eTRUE;
        }

        curHe = curHe->next;
    }
    while (curHe != he);

    return eFALSE;
}

// Returns area of polygon using Gaussian trapezoid formula.
eF32 eEditMesh::Face::getArea() const
{
	const eVector3& pos0 = he->origin->position;
	const eEditMesh::HalfEdge* he1 = he->next;
	const eEditMesh::HalfEdge* he2 = he1->next;
	eF32 area = 0.0f;
	do {
		area += 0.5f * ((he1->origin->position - pos0)^(he2->origin->position - pos0)).length();
		he1 = he2;
		he2 = he2->next;
	} while(he2 != he);
	return area;
}

void
eEditMesh::Face::getRandomSurfacePoint(eVector3 &resultPos, eVector3 &resultNormal) const {
	// assuming dumb equally sized triangulation, pick a random triangle
	int triNr = eRandom(0, this->getEdgeCount() - 2);
	const eEditMesh::Vertex& v0 = *this->he->origin;
	const eEditMesh::Vertex& v1 = *this->getVertex(triNr + 1);
	const eEditMesh::Vertex& v2 = *this->getVertex(triNr + 2);

	eF32 w0 = eRandomF();
    eF32 w1 = eRandomF();
    if (w0+w1 > 1.0f) {
        w0 = 1.0f - w0;
        w1 = 1.0f - w1;
    }
    eF32 w2 = 1.0f - w0 - w1;

	resultPos = (v0.position * w0) + (v1.position * w1) + (v2.position * w2);
    resultNormal = ((v0.normal * w0) + (v1.normal * w1) + (v2.normal * w2));
}


eEditMesh::Vertex eEditMesh::Face::getCenter() const
{
    HalfEdge *curHe = he;
    eASSERT(curHe != eNULL);

    eU32 edgeCount = 0;
    Vertex center;
	center.position = eVector3(0);
	center.normal = eVector3(0);
	center.texCoord = eVector2(0);

    do
    {
		center.add(*curHe->origin);
        edgeCount++;
        curHe = curHe->next;
    }
    while (curHe != he);

    eASSERT(edgeCount >= 3);
	center.mul(1.0f / (eF32)edgeCount);

    return center;
}

eU32 eEditMesh::Face::getEdgeCount() const
{
    eU32 edgeCount = 0;

    HalfEdge *curHe = he;
    eASSERT(curHe != eNULL);

    do
    {
        edgeCount++;
        curHe = curHe->next;
    }
    while (curHe != he);

    eASSERT(edgeCount >= 3);
    return edgeCount;
}

eEditMesh::HalfEdge * eEditMesh::Face::getHalfEdge(eU32 index) const {
    HalfEdge *curHe = he;
    while (index--)
        curHe = curHe->next;
    return curHe;
}

const eEditMesh::Vertex *
eEditMesh::Face::getVertex(eU32 index) const {
	return this->getHalfEdge(index)->origin;
}


eBool eEditMesh::HalfEdge::isFree() const
{
    return (face == eNULL);
}

eBool eEditMesh::HalfEdge::isBoundary() const
{
    return (face == eNULL);
}

eEditMesh::Vertex * eEditMesh::HalfEdge::destination() const
{
    return twin->origin;
}

// Implementation of edit mesh.

eEditMesh::eEditMesh(eU32 vertexCount, eU32 faceCount) :
    m_triangulated(eTRUE)
{
    reserveSpace(vertexCount, faceCount);
}

eEditMesh::eEditMesh(const eEditMesh &em) :
    m_triangulated(eTRUE)
{
    merge(em);
}

eEditMesh::~eEditMesh()
{
    clear();
}

// Reserve space for mesh data. The number of
// edges is calculated using the famous Euler
// formula: HE=4*F <=> 2*E=4*F <=> E=2*F
// (4x, because a quad mesh is assumed).
void eEditMesh::reserveSpace(eU32 vertexCount, eU32 faceCount)
{
    m_vertices.reserve(vertexCount);
    m_edges.reserve(4*faceCount);
    m_faces.reserve(faceCount);
}

void eEditMesh::merge(eEditMesh &em)
{
    ePROFILER_ZONE("Merge mesh Fast");

    reserveSpace(m_vertices.size()+em.getVertexCount(), m_faces.size()+em.getFaceCount());

    // Add vertices of new mesh.
    for (eU32 i=0; i<em.getVertexCount(); i++)
    {
        Vertex *vtx = em.getVertex(i);
        eASSERT(vtx != eNULL);

        Vertex *newVtx = addVertex(vtx->position, vtx->texCoord, vtx->normal);
        eASSERT(vtx != eNULL);
        newVtx->selected = vtx->selected;
        newVtx->color = vtx->color;
        newVtx->tag = vtx->tag;
		vtx->_volatile_attribute0 = newVtx;
    }

    // Add faces of new mesh.
    VertexPtrArray vtxLoop;
    eVector2Array texCoords;

    for (eU32 i=0; i<em.getFaceCount(); i++)
    {
        const Face *face = em.getFace(i);
        eASSERT(face != eNULL);

        HalfEdge *he = face->he;
        eASSERT(he != eNULL);


        Face *newFace = eNULL;
		if(em.isTriangulated()) {
			Vertex* v3[] = { (eEditMesh::Vertex*)he->origin->_volatile_attribute0,
				             (eEditMesh::Vertex*)he->next->origin->_volatile_attribute0,
						     (eEditMesh::Vertex*)he->next->next->origin->_volatile_attribute0};
			eVector2	tex3[] = {he->texCoord, he->next->texCoord, he->next->next->texCoord};
			newFace = addTriangleFast(&v3[0], &tex3[0]);
		} else {
			vtxLoop.clear();
			texCoords.clear();

			do
			{
				vtxLoop.append((eEditMesh::Vertex*)he->origin->_volatile_attribute0);
				texCoords.append(he->texCoord);

				he = he->next;
			}
			while (he != face->he);
			newFace = addFace(&vtxLoop[0], &texCoords[0], vtxLoop.size());
		}

		eASSERT(newFace != eNULL);
        newFace->normal = face->normal;
        newFace->selected = face->selected;
        newFace->material = face->material;
        newFace->tag = face->tag;

        // Copy over tags and selected state from edges.
        HalfEdge *heOrg = face->he;
        he = newFace->he;

        do
        {
            he->edge->tag = heOrg->edge->tag;
            he->edge->selected = heOrg->edge->selected;
            heOrg = heOrg->next;
            he = he->next;
        }
        while (he != newFace->he);
    }
}

void eEditMesh::merge(const eEditMesh &em)
{
    ePROFILER_ZONE("Merge mesh");

    reserveSpace(m_vertices.size()+em.getVertexCount(), m_faces.size()+em.getFaceCount());

    // Add vertices of new mesh.

    for (eU32 i=0; i<em.getVertexCount(); i++)
    {
        const Vertex *vtx = em.getVertex(i);
        eASSERT(vtx != eNULL);

        Vertex *newVtx = addVertex(vtx->position, vtx->texCoord, vtx->normal);
        eASSERT(vtx != eNULL);
        newVtx->selected = vtx->selected;
        newVtx->color = vtx->color;
        newVtx->tag = vtx->tag;
		// we need to break const here
        ((Vertex*)vtx)->_volatile_attribute0 = newVtx;
    }

    // Add faces of new mesh.
    VertexPtrArray vtxLoop;
    eVector2Array texCoords;

    for (eU32 i=0; i<em.getFaceCount(); i++)
    {
        const Face *face = em.getFace(i);
        eASSERT(face != eNULL);

        HalfEdge *he = face->he;
        eASSERT(he != eNULL);

        vtxLoop.clear();
        texCoords.clear();

        do
        {
            vtxLoop.append((Vertex*)he->origin->_volatile_attribute0);
            texCoords.append(he->texCoord);

            he = he->next;
        }
        while (he != face->he);

        Face *newFace = addFace(&vtxLoop[0], &texCoords[0], vtxLoop.size());
        eASSERT(newFace != eNULL);
        newFace->normal = face->normal;
        newFace->selected = face->selected;
        newFace->material = face->material;
        newFace->tag = face->tag;

        // Copy over tags and selected state from edges.
        HalfEdge *heOrg = face->he;
        he = newFace->he;

        do
        {
            he->edge->tag = heOrg->edge->tag;
            he->edge->selected = heOrg->edge->selected;
            heOrg = heOrg->next;
            he = he->next;
        }
        while (he != newFace->he);
    }
}

void eEditMesh::clear()
{
    for (eU32 i=0; i<m_faces.size(); i++)
        DEALLOCATE_OPT(m_faces[i]);

    for (eU32 i=0; i<m_edges.size(); i++)
    {
        Edge *edge = m_edges[i];
        eASSERT(edge != eNULL);

        DEALLOCATE_OPT(edge->he0);
        DEALLOCATE_OPT(edge->he1);
        DEALLOCATE_OPT(edge);
    }

    for (eU32 i=0; i<m_vertices.size(); i++)
	    DEALLOCATE_OPT(m_vertices[i]);

    m_bbox.clear();

    m_faces.clear();
    m_edges.clear();
    m_vertices.clear();

    m_triangulated = eTRUE;
}

void                    
eEditMesh::clearAndPreallocate(eU32 numVertices, eU32 numFaces, eU32 numEdges) {
	this->clear();
	this->m_preallocatedVertices.reserve(numVertices);
	this->m_preallocatedVertices.resize(0);
	this->m_preallocatedFaces.reserve(numFaces);
	this->m_preallocatedFaces.resize(0);
	this->m_preallocatedEdges.reserve(numEdges);
	this->m_preallocatedEdges.resize(0);
	this->m_preallocatedHalfEdges.reserve(numEdges * 2);
	this->m_preallocatedHalfEdges.resize(0);
}

// Be careful with faces with more than four vertices.
// They might be un triangulatable after some mesh
// modifiers, because the faces got degenerated.
eBool eEditMesh::triangulate()
{
    ePROFILER_ZONE("Triangulate");

    if (m_triangulated)
    {
        return eTRUE;
    }

    const eU32 oldFaceCount = m_faces.size();

    for (eInt i=(eInt)oldFaceCount-1; i>=0; i--)
    {
        Face *face = m_faces[i];
        eASSERT(face != eNULL);

        switch (face->getEdgeCount())
        {
            case 3:
            {
                // Triangle => do nothing.
                break;
            }

            case 4:
            {
                // Vertices and texture coordinates
                // for the two new triangles.
                HalfEdge *faceHe = face->he;
                eASSERT(faceHe != eNULL);

                Vertex *vertsTri0[3] =
                {
                    faceHe->origin,
                    faceHe->next->origin,
                    faceHe->next->next->origin,
                };

                const eVector2 texCoordsTri0[3] =
                {
                    faceHe->texCoord,
                    faceHe->next->texCoord,
                    faceHe->next->next->texCoord,
                };

                Vertex *vertsTri1[3] =
                {
                    faceHe->origin,
                    faceHe->next->next->origin,
                    faceHe->next->next->next->origin,
                };

                const eVector2 texCoordsTri1[3] =
                {
                    faceHe->texCoord,
                    faceHe->next->next->texCoord,
                    faceHe->next->next->next->texCoord
                };
                
                // Unlink face in order to be able to
                // add new faces to the old half-edges.
                HalfEdge *he = faceHe;

                do
                {
                    he->face = eNULL;
                    he = he->next;
                }
                while (he != face->he);

                // Add first triangle. Make a trick so that
                // we don't have to delete the face out of
                // the middle, but from the end of the faces
                // array (=> much faster).
                Face *f = addFace(vertsTri0, texCoordsTri0, 3);
                f->selected = face->selected;
                f->material = face->material;
                f->normal = face->normal;
                f->tag = face->tag;

                m_faces[i] = f;
                m_faces.removeAt(m_faces.size()-1);

                // Add second triangle.
                f = addFace(vertsTri1, texCoordsTri1, 3);
                f->selected = face->selected;
                f->material = face->material;
                f->normal = face->normal;
                f->tag = face->tag;

                // Free memory of old quad face.
                DEALLOCATE_OPT(face);
                break;
            }

            default:
            {
                if (!_triangulateFace(face))
                {
                    return eFALSE;
                }

                break;
            }
        }
    }

    m_triangulated = eTRUE;
    return eTRUE;
}

void eEditMesh::optimizeIndices()
{
}

void eEditMesh::updateBoundingBox()
{
    m_bbox.clear();

    for (eU32 i=0; i<m_vertices.size(); i++)
    {
        m_bbox.updateExtent(m_vertices[i]->position);
    }
}

void eEditMesh::updateOutlineNormals()
{
    for (eU32 i=0; i<m_vertices.size(); i++)
        if(m_vertices[i]->isBoundary() && (m_vertices[i]->he))
        {
            eVector3 sum(0,0,0);
            int cnt = 0;
            // scan through edges
            eEditMesh::HalfEdge* startHE = m_vertices[i]->he;
            eEditMesh::HalfEdge* he = startHE;
            do {
                if(he->isBoundary()) {
                    eVector3 conorm = (he->twin->face->normal^(he->destination()->position - he->origin->position)).normalized();
                    sum += conorm;
                    cnt++;
                }
                if(he->twin->isBoundary()) {
                    eVector3 conorm = ((he->destination()->position - he->origin->position)^he->face->normal).normalized();
                    sum += conorm;
                    cnt++;
                }
                he = he->twin->next;
            } while(he != startHE);
            if(cnt != 0) {
                m_vertices[i]->normal = sum * (1.0f / (eF32)cnt);
            }
        }
}

void eEditMesh::updateNormals()
{
    for (eU32 i=0; i<m_vertices.size(); i++)
    {
        m_vertices[i]->normal.null();
    }

    for (eU32 i=0; i<m_faces.size(); i++)
    {
        Face *face = m_faces[i];
        eASSERT(face != eNULL);
        face->updateNormal();

        HalfEdge *he = face->he;
        eASSERT(he != eNULL);

        do
        {
            he->origin->normal += face->normal;
            he = he->next;
        }
        while (he != face->he);
    }

    for (eU32 i=0; i<m_vertices.size(); i++)
    {
        m_vertices[i]->normal.normalize();
    }
}

void eEditMesh::setMaterial(const eMaterial *mat)
{
    eASSERT(mat != eNULL);

    for (eU32 i=0; i<m_faces.size(); i++)
    {
        m_faces[i]->material = mat;
    }
}

eEditMesh::Vertex * eEditMesh::addVertex(const Vertex& vertex)
{
	return addVertex(vertex.position, vertex.texCoord, vertex.normal);
}

eEditMesh::Vertex * eEditMesh::addVertex(const eVector3 &pos)
{
	return addVertex(pos, eVector2());
}

eEditMesh::Vertex * eEditMesh::addVertex(const eVector3 &pos, const eVector2 &texCoord)
{
	return addVertex(pos, texCoord, eVector3());
}

eEditMesh::Vertex * eEditMesh::addVertex(const eVector3 &pos, const eVector2 &texCoord, const eVector3 &normal)
{
	ALLOCATE_OPT(Vertex, vtx, this->m_preallocatedVertices);
    eASSERT(vtx != eNULL);

	vtx->position = pos;
    vtx->texCoord = texCoord;
    vtx->normal = normal;
    vtx->he = eNULL;
    vtx->selected = eFALSE;
    vtx->color = eColor::WHITE;
    vtx->tag = 0;

    m_bbox.updateExtent(pos);
    m_vertices.append(vtx);

    return vtx;
}

void                    
eEditMesh::setSize(eU32 vertexCount, eU32 faceCount) {
	this->reserveSpace(vertexCount, faceCount);
	this->m_faces.resize(faceCount);
	this->m_vertices.resize(vertexCount);
	// initialize vertices
	for(eU32 i = 0; i < vertexCount; i++) {
		ALLOCATE_OPT(Vertex, vtx, this->m_preallocatedVertices);
		m_vertices[i] = vtx;
	}
}

eEditMesh::HalfEdge * eEditMesh::_findHalfEdge(const Vertex *fromVtx, const Vertex *toVtx)
{
    eASSERT(fromVtx != eNULL);
    eASSERT(toVtx != eNULL);

    if (fromVtx->he == eNULL || toVtx->he == eNULL)
        return eNULL;

    eEditMesh::HalfEdge *he = fromVtx->he;
    do {
        if (he->destination() == toVtx)
            return he;
        he = he->twin->next;
    } while (he != fromVtx->he);

    return eNULL;
}

eEditMesh::Face * eEditMesh::addTriangleFast(Vertex** vertices, const eVector2 *texCoords)
{
	ALLOCATE_OPT(Face, newFace, this->m_preallocatedFaces);
	eASSERT(newFace != eNULL);

	Edge *nedges[3];
	HalfEdge *nhalfEdges[6];

	ALLOCATE_OPT_MULTIPLE(Edge, nedges, this->m_preallocatedEdges, 3);
	ALLOCATE_OPT_MULTIPLE(HalfEdge, nhalfEdges, this->m_preallocatedHalfEdges, 6);

	// Create half-edges that are not yet existing.
	for (eU32 i=0; i<3; i++)
    {
		const eU32 nextIdx = (i + 1) % 3;

		Edge *edge = nedges[i];
		m_edges.append(edge);

		HalfEdge* newHe = nhalfEdges[i];
		HalfEdge* newHe2 = nhalfEdges[i + 3];

		// Initialize edge and half-edges.
		edge->he0 = newHe;
		edge->he1 = newHe2;
		edge->selected = eFALSE;
		edge->tag = 0;

		newHe->origin = vertices[i];
		newHe->edge = edge;
		newHe->face = newFace;
        newHe->texCoord = texCoords[i];
		newHe->next = nhalfEdges[nextIdx];
		newHe->twin = newHe2;

		newFace->he = newHe;

		vertices[i]->he = newHe;

		newHe2->twin = newHe;
		newHe2->face = eNULL;
		newHe2->edge = edge;
		newHe2->origin = vertices[nextIdx];

    }

    newFace->selected = eFALSE;
    newFace->material = eMaterial::getDefault();
    newFace->tag = 0;

    m_faces.append(newFace);
    return newFace;
}


eEditMesh::Face * eEditMesh::addFace(Vertex **vertices, const eVector2 *texCoords, eU32 vertexCount)
{
    eASSERT(vertexCount >= 3);

    if (vertexCount > 3)
    {
        m_triangulated = eFALSE;
    }

    // Check that the vertices are free.
    for (eU32 i=0; i<vertexCount; i++)
    {
        eASSERT(vertices[i] != eNULL);
        eASSERT(vertices[i]->isFree() == eTRUE);
    }

    // Find the existing half-edges. Check that they are free.
    for (eU32 i=0; i<vertexCount; i++)
    {
        HalfEdge *he = _findHalfEdge(vertices[i], vertices[(i+1)%vertexCount]);

        if (he != eNULL)
        {
            eASSERT(he->isFree() == eTRUE);
        }

		vertices[i]->_volatile_attribute0 = he;
    }

    // Create half-edges that are not yet existing.
    for (eU32 i=0; i<vertexCount; i++)
    {
        const eU32 nextIndex = (i+1)%vertexCount;

		if (vertices[i]->_volatile_attribute0 == eNULL)
        {
            Edge *newEdge = _addEdge(vertices[i], vertices[nextIndex]);
            eASSERT(newEdge != eNULL);

            HalfEdge *newHe = newEdge->he0;
            eASSERT(newHe != eNULL);

            if (newHe->origin != vertices[i])
            {
                newHe = newHe->twin;
                eASSERT(newHe->origin == vertices[i]);
            }

            vertices[i]->_volatile_attribute0 = newHe;
        }

        // Copy texture coordinates.
		HalfEdge* curHe = ((HalfEdge*)vertices[i]->_volatile_attribute0);
        eASSERT(curHe->origin == vertices[i]);
        curHe->texCoord = (texCoords ? texCoords[i] : curHe->origin->texCoord);
    }

    // Check that the half-edges are free and form a chain.
    for (eU32 i=0; i<vertexCount; i++)
    {
        const eU32 ip1 = (i+1)%vertexCount;
		const HalfEdge* curHe = ((HalfEdge*)vertices[i]->_volatile_attribute0);
		const HalfEdge* ip1He = ((HalfEdge*)vertices[ip1]->_volatile_attribute0);
        eASSERT(curHe->destination() == ip1He->origin);
        eASSERT(curHe->isFree() == eTRUE);
    }

    // Try to reorder the links to get a proper orientation.
    for (eU32 i=0; i<vertexCount; i++)
    {
        const eU32 ip1 = (i+1)%vertexCount;

        // Would face introduce a non-manifold condition?
        const eBool nonManifold = !_makeAdjacent(((HalfEdge*)vertices[i]->_volatile_attribute0), 
			                                     ((HalfEdge*)vertices[ip1]->_volatile_attribute0));
        eASSERT(nonManifold == eFALSE);
    }

	ALLOCATE_OPT(Face, newFace, this->m_preallocatedFaces);
    eASSERT(newFace != eNULL);
    newFace->selected = eFALSE;
    newFace->material = eMaterial::getDefault();
    newFace->tag = 0;
    newFace->he = ((HalfEdge*)vertices[0]->_volatile_attribute0);

    // Link half-edges to the polygon.
    for (eU32 i=0; i<vertexCount; i++)
        ((HalfEdge*)vertices[i]->_volatile_attribute0)->face = newFace;

    m_faces.append(newFace);
    return newFace;
}

eEditMesh::Face * eEditMesh::addFace(const eU32 *indices, const eVector2 *texCoords, eU32 indexCount)
{
    eASSERT(indices != eNULL);
    eASSERT(indexCount >= 3);
    eASSERT(indexCount <= m_vertices.size());

    VertexPtrArray vertices(indexCount);

    for (eU32 i=0; i<indexCount; i++)
    {
        const eU32 index = indices[i];
        eASSERT(index < m_vertices.size());
        vertices[i] = m_vertices[index];
    }

    return addFace(&vertices[0], texCoords, indexCount);
}

eEditMesh::Face * eEditMesh::addTriangle(Vertex *v0, Vertex *v1, Vertex *v2)
{
    Vertex *vertices[3] =
    {
        v0, v1, v2
    };

    return addFace(vertices, eNULL, 3);
}

eEditMesh::Face * eEditMesh::addTriangle(eU32 i0, eU32 i1, eU32 i2)
{
    const eU32 indices[3] =
    {
        i0, i1, i2
    };

    return addFace(indices, eNULL, 3);
}

eEditMesh::Face * eEditMesh::addQuad(Vertex *v0, Vertex *v1, Vertex *v2, Vertex *v3)
{
    Vertex *vertices[4] =
    {
        v0, v1, v2, v3
    };

    return addFace(vertices, eNULL, 4);
}

eEditMesh::Face * eEditMesh::addQuad(eU32 i0, eU32 i1, eU32 i2, eU32 i3)
{
    const eU32 indices[4] =
    {
        i0, i1, i2, i3
    };

    return addFace(indices, eNULL, 4);
}

eBool eEditMesh::removeVertex(const Vertex *vtx)
{
    for (eU32 i=0; i<m_vertices.size(); i++)
    {
        if (m_vertices[i] == vtx)
        {
            return removeVertex(i);
        }
    }

    return eFALSE;
}

eBool eEditMesh::removeVertex(eU32 index)
{
    eASSERT(index < m_vertices.size());

    Vertex *vtx = m_vertices[index];
    eASSERT(vtx != eNULL);

    // Can vertex be deleted?
    if (vtx->he)
    {
        return eFALSE;
    }

	DEALLOCATE_OPT(vtx);
    m_vertices.removeAt(index);

    return eTRUE;
}

eBool eEditMesh::removeEdge(const Edge *edge)
{
    for (eU32 i=0; i<m_edges.size(); i++)
    {
        if (m_edges[i] == edge)
        {
            return removeEdge(i);
        }
    }

    return eFALSE;
}

eBool eEditMesh::removeEdge(eU32 index)
{
    eASSERT(index < m_edges.size());

    // Can edge be deleted?
    Edge *edge = m_edges[index];
    eASSERT(edge != eNULL);

    if (edge->he0->face || edge->he1->face)
    {
        return eFALSE;
    }

    // Link the from-side of the edge off the mesh.
    HalfEdge *fromToHe = edge->he0;
    HalfEdge *toFromHe = edge->he1;
    Vertex *fromVtx = fromToHe->origin;
    HalfEdge *fromIn = fromToHe->prev;
    HalfEdge *fromOut = fromToHe->twin->next;

    if (fromVtx->he == fromToHe)
    {
        fromVtx->he = (fromOut == fromToHe ? eNULL : fromOut);
    }

    fromIn->next = fromOut;
    fromOut->prev = fromIn;

    // Link the to-side of the edge off the mesh.
    Vertex *toVtx = toFromHe->origin;
    HalfEdge *toIn = toFromHe->prev;
    HalfEdge *toOut = toFromHe->twin->next;

    if (toVtx->he == toFromHe)
    {
        toVtx->he = (toOut == toFromHe ? eNULL : toOut);
    }

    toIn->next = toOut;
    toOut->prev = toIn;

    // Can vertices be removed?
    if (toVtx->he == eNULL)
    {
        removeVertex(toVtx);
    }

    if (fromVtx->he == eNULL)
    {
        removeVertex(fromVtx);
    }

    // Free the memory.
    DEALLOCATE_OPT(edge->he0);
    DEALLOCATE_OPT(edge->he1);
    DEALLOCATE_OPT(edge);
    m_edges.removeAt(index);

    return eTRUE;
}

void eEditMesh::removeFace(const Face *face, eBool remFreeGeo)
{
    for (eU32 i=0; i<m_faces.size(); i++)
    {
        if (m_faces[i] == face)
        {
            removeFace(i, remFreeGeo);
            break;
        }
    }
}

// Removes the face with the given index from the
// mesh. Geometry (edges, vertices) that gets "free"
// after removing the face is removed depending on
// the second parameter flag.
void eEditMesh::removeFace(eU32 index, eBool remFreeGeo)
{
    eASSERT(index < m_faces.size());

    Face *face = m_faces[index];
    eASSERT(face != eNULL);

    HalfEdge *he = face->he;
    EdgePtrArray edgesToRemove;

    if (he != eNULL)
    {
        do
        {
            he->face = eNULL;

            // Check if edge can be removed.
            if (he->twin->face == eNULL)
            {
                edgesToRemove.append(he->edge);
            }

            he = he->next;
        }
        while (he != face->he);

        face->he = eNULL;
    }

    // Free memory of face.
    DEALLOCATE_OPT(face);
    m_faces.removeAt(index);

    // Remove "free" edges of face if requested.
    if (remFreeGeo)
    {
        for (eU32 i=0; i<edgesToRemove.size(); i++)
        {
            removeEdge(edgesToRemove[i]);
        }
    }
}

eBool eEditMesh::checkConsistency() const
{
    for (eU32 i=0; i<m_vertices.size(); i++)
    {
    }

    return eTRUE;
}

eBool eEditMesh::isTriangulated() const
{
    return m_triangulated;
}

eBool eEditMesh::isEmpty() const
{
    return (m_vertices.size() == 0 && m_faces.size() == 0);
}

eU32 eEditMesh::getVertexCount() const
{
    return m_vertices.size();
}

eU32 eEditMesh::getMaxFaceValence() const
{
    eU32 maxValence = 0;

    for (eU32 i=0; i<m_faces.size(); i++)
    {
        const eU32 count = m_faces[i]->getEdgeCount();

        if (count > maxValence)
        {
            maxValence = count;
        }
    }

    return maxValence;
}

eU32 eEditMesh::getEdgeCount() const
{
    return m_edges.size();
}

eU32 eEditMesh::getFaceCount() const
{
    return m_faces.size();
}

const eAABB & eEditMesh::getBoundingBox() const
{
    return m_bbox;
}

eAABB & eEditMesh::getBoundingBox()
{
    return m_bbox;
}

void eEditMesh::setBoundingBox(const eAABB & bbox)
{
    m_bbox = bbox;
}

void eEditMesh::updateBoundingBox(const eVector3& pos) {
	m_bbox.updateExtent(pos);
}


const eEditMesh::Vertex * eEditMesh::getVertex(eU32 index) const
{
    eASSERT(index < m_vertices.size());
    return m_vertices[index];
}

eEditMesh::Vertex * eEditMesh::getVertex(eU32 index)
{
    eASSERT(index < m_vertices.size());
    return m_vertices[index];
}

const eEditMesh::Edge * eEditMesh::getEdge(eU32 index) const
{
    eASSERT(index < m_edges.size());
    return m_edges[index];
}

eEditMesh::Edge * eEditMesh::getEdge(eU32 index)
{
    eASSERT(index < m_edges.size());
    return m_edges[index];
}

const eEditMesh::Face * eEditMesh::getFace(eU32 index) const
{
    eASSERT(index < m_faces.size());
    return m_faces[index];
}

eEditMesh::Face * eEditMesh::getFace(eU32 index)
{
    eASSERT(index < m_faces.size());
    return m_faces[index];
}

eEditMesh & eEditMesh::operator = (eEditMesh &em) {
    if (&em != this) {
		this->clearAndPreallocate(em.getVertexCount(), em.getFaceCount(), em.getEdgeCount());
		this->reserveSpace(em.getVertexCount(), em.getFaceCount());
        merge(em);
    }

    return *this;
}

eEditMesh::HalfEdge * eEditMesh::_findFreeIncident(Vertex *vtx) const
{
    eASSERT(vtx != eNULL);
    eASSERT(vtx->he != eNULL);

    eEditMesh::HalfEdge *he = vtx->he->twin;

    do
    {
        if (he->face == eNULL)
        {
            return he;
        }

        he = he->next->twin;
    }
    while (he != vtx->he->twin);

    return eNULL;
}

eEditMesh::HalfEdge * eEditMesh::_findFreeIncident(Vertex *vtx, HalfEdge *startHe, HalfEdge *endBeforeHe) const
{
    eASSERT(vtx != eNULL);
    eASSERT(vtx->he != eNULL);
    eASSERT(startHe != eNULL);
    eASSERT(startHe->destination() == vtx);
    eASSERT(endBeforeHe != eNULL);
    eASSERT(endBeforeHe->destination() == vtx);

    if (endBeforeHe == startHe)
    {
        return eNULL;
    }

    eEditMesh::HalfEdge *he = startHe;

    do
    {
        if (he->face == eNULL)
        {
            return he;
        }

        he = he->next->twin;
    }
    while (he != endBeforeHe);

    return eNULL;
}

eBool eEditMesh::_makeAdjacent(HalfEdge *inHe, HalfEdge *outHe)
{
    eASSERT(inHe != eNULL);
    eASSERT(outHe != eNULL);
    eASSERT(inHe->destination() == outHe->origin);
    eASSERT(inHe->face == eNULL);
    eASSERT(outHe->face == eNULL);

    // Is adjacency already correct?
    if (inHe->next == outHe)
    {
        return eTRUE;
    }

    eEditMesh::HalfEdge *b = inHe->next;
    eEditMesh::HalfEdge *d = outHe->prev;

    // Find a free incident half-edge after
    // d and before the in half-edge.
    eEditMesh::HalfEdge *g = _findFreeIncident(outHe->origin, outHe->twin, inHe);

    if (g == eNULL)
    {
        // There is no such half-edge.
        return eFALSE;
    }

    eEditMesh::HalfEdge *h = g->next;

    inHe->next = outHe;
    outHe->prev = inHe;
    g->next = b;
    b->prev = g;
    d->next = h;
    h->prev = d;

    return eTRUE;
}

eEditMesh::Edge * eEditMesh::_addEdge(Vertex *fromVtx, Vertex *toVtx)
{
    eASSERT(fromVtx != eNULL);
    eASSERT(toVtx != eNULL);
    eASSERT(fromVtx != toVtx);

    // Does given half-edge already exists?
    HalfEdge *existingHe = _findHalfEdge(fromVtx, toVtx);

    if (existingHe != eNULL)
    {
        return existingHe->edge;
    }

    // Check that the vertices are free. If
    // not, the edge addition is not possible.
    HalfEdge *fromFreeHe = eNULL;

    if (fromVtx->he != eNULL)
    {
        fromFreeHe = _findFreeIncident(fromVtx);

        if (fromFreeHe == eNULL)
        {
            return eNULL;
        }
    }

    if (toVtx->he != eNULL)
    {
        const HalfEdge *toFreeHe = _findFreeIncident(toVtx);

        if (toFreeHe == eNULL)
        {
            return eNULL;
        }
    }

    // Allocate edge and half-edges.
	ALLOCATE_OPT(Edge, edge, this->m_preallocatedEdges);
    eASSERT(edge != eNULL);
    m_edges.append(edge);

	ALLOCATE_OPT(HalfEdge, fromToHe, this->m_preallocatedHalfEdges);
    eASSERT(fromToHe != eNULL);
	ALLOCATE_OPT(HalfEdge, toFromHe, this->m_preallocatedHalfEdges);
    eASSERT(toFromHe != eNULL);

    // Initialize edge and half-edges.
    edge->he0 = fromToHe;
    edge->he1 = toFromHe;
    edge->selected = eFALSE;
    edge->tag = 0;

    fromToHe->next = toFromHe;
    fromToHe->prev = toFromHe;
    fromToHe->twin = toFromHe;
    fromToHe->origin = fromVtx;
    fromToHe->edge = edge;
    fromToHe->face = eNULL;

    toFromHe->next = fromToHe;
    toFromHe->prev = fromToHe;
    toFromHe->twin = fromToHe;
    toFromHe->origin = toVtx;
    toFromHe->edge = edge;
    toFromHe->face = eNULL;

    // Link the from-side of the edge.
    if (fromVtx->he == eNULL)
    {
        fromVtx->he = fromToHe;
    }
    else
    {
        HalfEdge *fromInHe = fromFreeHe;
        eASSERT(fromInHe != eNULL);

        HalfEdge *fromOutHe = fromInHe->next;

        fromInHe->next = fromToHe;
        fromToHe->prev = fromInHe;
        toFromHe->next = fromOutHe;
        fromOutHe->prev = toFromHe;
    }

    // Link the to-side of the edge.
    if (toVtx->he == eNULL)
    {
        toVtx->he = toFromHe;
    }
    else
    {
        // 'toFreeHe' can't be reused here, because
        // the perimeter has changed due to the linking
        // of the from-side.
        HalfEdge *toInHe = _findFreeIncident(toVtx);
        eASSERT(toInHe != eNULL);

        HalfEdge *toOutHe = toInHe->next;

        toInHe->next = toFromHe;
        toFromHe->prev = toInHe;
        fromToHe->next = toOutHe;
        toOutHe->prev = fromToHe;
    }

    return edge;
}

// Returns wether or not the face could be triangulated.
eBool eEditMesh::_triangulateFace(Face *face)
{
    eASSERT(face != eNULL);

    // Get contour of given face.
    eVector3Array contourPos;
    VertexPtrArray contourVerts;
    eVector2Array contourUvs;

    HalfEdge *he = face->he;

    do
    {
        contourPos.append(he->origin->position);
        contourVerts.append(he->origin);
        contourUvs.append(he->texCoord);

        he = he->next;
    }
    while (he != face->he);

    // Triangulate given face.
    eTriangulator trg;

    trg.addContour(contourPos);
    
    if (!trg.triangulate())
    {
        return eFALSE;
    }

    // Remove triangulated face from mesh.
    for (eU32 i=0; i<m_faces.size(); i++)
    {
        if (m_faces[i] == face)
        {
            Face *face = m_faces[i];
            eASSERT(face != eNULL);

            HalfEdge *he = face->he;

            do
            {
                he->face = eNULL;
                he = he->next;
            }
            while (he != face->he);

            m_faces.removeAt(i);
            break;
        }
    }

    // Add newly generated vertices to mesh.
    for (eU32 i=contourPos.size(); i<trg.getVertices().size(); i++)
    {
        Vertex *newVtx = addVertex(trg.getVertices()[i]);
        eASSERT(newVtx != eNULL);
        contourVerts.append(newVtx);
    }

    // Add new triangles to mesh.
    const eArray<eU32> &indices = trg.getIndices();

    for (eU32 i=0; i<indices.size(); i+=3)
    {
        const eU32 idx0 = indices[i];
        const eU32 idx1 = indices[i+1];
        const eU32 idx2 = indices[i+2];

        const eVector2 texCoords[3] =
        {
            contourUvs[idx0],
            contourUvs[idx1],
            contourUvs[idx2],
        };

        Vertex *vertices[3] =
        {
            contourVerts[idx0],
            contourVerts[idx1],
            contourVerts[idx2]
        };

        Face *newFace = addFace(vertices, texCoords, 3);
        eASSERT(newFace != eNULL);
        newFace->material = face->material;
        newFace->selected = face->selected;
        newFace->normal = face->normal;
        newFace->tag = face->tag;
    }

    DEALLOCATE_OPT(face);
    return eTRUE;
}

void					
eEditMesh::centerMesh() {
    const eVector3 &center = this->getBoundingBox().getCenter();

    for (eU32 i=0; i<this->getVertexCount(); i++)
        this->getVertex(i)->position -= center;

    this->getBoundingBox().translate(-center);
}

void					
eEditMesh::mapUVs(eU32 method, const eVector3	up, const eVector3	projectAxis, const eVector3	center, const eVector2	uvscale) {
//	this->triangulate();
	this->updateNormals();

	eMatrix4x4 mat;
	mat.lookAt(projectAxis, eVector3(0,0,0), up);
	eMatrix4x4 invMat = mat.inverse();

    typedef struct vRecord {
        eVector3    pos;
        eVector3    normal;
        eF32        lat;
    } vRecord;

    eArray<vRecord>    vList;
	for(eU32 f = 0; f < this->getFaceCount(); f++) {
		eEditMesh::Face* face = this->getFace(f);
/*
        eVector3	pos[3];
		eVector3	normal[3];
		eF32		lat[3] = {-1.0f, -1.0f, -1.0f};
*/
        vList.clear();
		eU32		defCnt = 0;
		eF32		lastLat;
        for(eU32 e = 0; e < face->getEdgeCount(); e++) {
            vRecord rec;
            rec.lat = 1.0;
			rec.pos = invMat * (face->getVertex(e)->position - center);
			rec.normal = rec.pos.normalized();
			if((rec.normal.x != 0.0f) || (rec.normal.y != 0.0f)) {
				rec.lat = eATan2(rec.normal.y, rec.normal.x) + ePI;
				lastLat = rec.lat;
				defCnt++;
			}
            vList.append(rec);
		}

		for(eU32 e = 0; e < vList.size(); e++) {
            vRecord& rec = vList[e];
			if(rec.lat < 0.0f) {
				if(defCnt == 1)
					rec.lat = lastLat;
				else {
                    eF32 lat0 = vList[(e+1)%vList.size()].lat;
					eF32 lat1 = vList[(e+2)%vList.size()].lat;
					rec.lat = 0.5f * (lat0 + lat1);
					if(eAbs(lat1 - lat0 >= ePI))
						rec.lat += ePI;
				}
			}

			eEditMesh::HalfEdge* edge = face->getHalfEdge(e);
			eF32 tu, tv;
			switch(method) {
				case SPHERICAL: {
							tu = rec.lat;
							tv = eACos(rec.normal.z) * 2.0f;
							tu = eClamp(0.0f, tu, 2.0f * ePI);
							tv = eClamp(0.0f, tv, 2.0f * ePI);
						} break;
				case SPHERICAL_MERCATOR: {
							tu = rec.lat;
							eF32 s = rec.normal.z;
							if(s != 1.0f)
								tv = eATanh(s) + ePI;
							else 
								tv = ePI;
							tu = eClamp(0.0f, tu, 2.0f * ePI);
							tv = eClamp(0.0f, tv, 2.0f * ePI);
						} break;
				case CYLINDER: {
							tu = rec.lat;
							tv = rec.pos.z;
							tu = eClamp(0.0f, tu, 2.0f * ePI);
						} break;
				case PLANE: {
							tu = rec.pos.x;
							tv = rec.pos.y;
						} break;
				case CUBE: {
							eVector3 n = invMat * face->normal;
							eU32 normAxis = 0;
							if(eAbs(n[1]) > eAbs(n[normAxis]))
								normAxis = 1;
							if(eAbs(n[2]) > eAbs(n[normAxis]))
								normAxis = 2;
							eU32 side0 = 0;
							while(side0 == normAxis)
								side0++;
							eU32 side1 = 0;
							while((side1 == normAxis) || (side1 == side0))
								side1++;

							tu = rec.pos[side0];
							tv = rec.pos[side1];
						} break;
			}
			tu *= 0.5f * uvscale.x / ePI;
			tv *= 0.5f * uvscale.y / ePI;
			edge->texCoord = eVector2(tu, tv);
			edge->origin->texCoord = edge->texCoord;
		}

		// fix mapping errors
		switch(method) {
			case SPHERICAL:
			case SPHERICAL_MERCATOR:
			case CYLINDER:
				for(eU32 e = 0; e < 3; e++) {
					eEditMesh::HalfEdge* cur = face->getHalfEdge(e);
					if(       (eAbs(cur->next->texCoord.x - cur->texCoord.x) >= uvscale.x * 0.5f) &&
						(eAbs(cur->next->next->texCoord.x - cur->texCoord.x) >= uvscale.x * 0.5f)) {
							if(cur->texCoord.x >= uvscale.x * 0.5f) {
								cur->next->texCoord.x += uvscale.x;
								cur->next->next->texCoord.x += uvscale.x;
							}
							else
								cur->texCoord.x += uvscale.x;
							break;
					}
				}
				break;
		}
	}
}


void					
eEditMesh::transform(const eMatrix4x4 &mtx) {
    eMatrix4x4 mtxNrm = mtx;
    mtxNrm.invert();
    mtxNrm.transpose();

	for (eU32 i=0; i<this->getVertexCount(); i++)
    {
        eEditMesh::Vertex *vtx = this->getVertex(i);
        eASSERT(vtx != eNULL);
        vtx->position *= mtx;
        vtx->normal *= mtxNrm;
	}

    // Update face normals.
    for (eU32 i=0; i<this->getFaceCount(); i++)
    {
        this->getFace(i)->updateNormal();
    }

    // Finally update bounding box and normals.
    this->updateBoundingBox();

}
