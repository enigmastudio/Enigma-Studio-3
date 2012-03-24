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

#ifndef EDIT_MESH_HPP
#define EDIT_MESH_HPP

// Triangulator for polygons (with holes).
class eTriangulator
{
public:
    eTriangulator();
    ~eTriangulator();

    eBool                   triangulate();

    void                    addContour(const eVector3Array &contour);
    void                    clearContours();

    const eArray<eU32> &    getIndices() const;
    const eVector3Array &   getVertices() const;

private:
    static void eCALLBACK   _vertexCallback(eU32 vtxIndex, eTriangulator *trg);
    static void eCALLBACK   _errorCallback(eU32 errNo, eTriangulator *trg);
    static void eCALLBACK   _edgeFlagCallback(eU8 flag, eTriangulator *trg);
    static void eCALLBACK   _combineCallback(double newVert[3], eU32 neighbourVert[4], eF32 neighborWeight[4], eU32 *index, eTriangulator *trg);

private:
    eArray<eVector3>        m_vertices;
    eArray<eVector3Array *> m_contours;
    eArray<eU32>            m_triIndices;
    eU32                    m_totalVtxCount;
};

// Half-edge based mesh editing data-structure.
class eEditMesh
{
public:
    class HalfEdge;
    class Face;
    class Edge;

    class Vertex
    {
    public:
        eBool               isFree() const;
        eBool               isBoundary() const;

		void				lerp(const Vertex& v0, const Vertex& v1, eF32 t);
		void				add(const Vertex& other);
		void				mul(eF32 val);

    public:
        HalfEdge *          he;

        eVector3            position;
        eVector3            normal;
        eVector2            texCoord;
        eBool               selected;
        eColor              color;
        eID                 tag;

		void*				_volatile_attribute0; // for emesh-internal operations
		void*				_volatile_attribute1; // for      -external operations
		eBool				isPreallocated;
    };

    class Edge
    {
    public:
        eBool               isBoundary() const;

    public:
        HalfEdge *          he0;
        HalfEdge *          he1;
        eBool               selected;
        eID                 tag;

		void*				_volatile_attribute0;
		eBool				isPreallocated;
    };

    class Face
    {
    public:
        void                updateNormal();

        eBool               isBoundary() const;
        eF32                getArea() const;
		Vertex				getCenter() const;
        eU32                getEdgeCount() const;
        HalfEdge *          getHalfEdge(eU32 index) const;
		const Vertex *		getVertex(eU32 index) const;
		void				getRandomSurfacePoint(eVector3 &resultPos, eVector3 &resultNormal) const;
    public:
        HalfEdge *          he;

        const eMaterial *   material;
        eVector3            normal;
        eBool               selected;
        eID                 tag;

		void*				_volatile_attribute0;
		eBool				isPreallocated;
    };

    class HalfEdge
    {
    public:
        eBool               isFree() const;
        eBool               isBoundary() const;

        Vertex *            destination() const;

    public:
        Vertex *            origin;
        HalfEdge *          prev;
        HalfEdge *          next;
        HalfEdge *          twin;
        Edge *              edge;
        Face *              face;

        eVector2            texCoord;
		void*				_volatile_attribute0;
		eBool				isPreallocated;
    };

public:
    typedef eArray<HalfEdge *> HalfEdgePtrArray;
    typedef eArray<Vertex *> VertexPtrArray;
    typedef eArray<Edge *> EdgePtrArray;
    typedef eArray<Face *> FacePtrArray;

public:
    eEditMesh(eU32 vertexCount=0, eU32 faceCount=0);
    eEditMesh(const eEditMesh &em);
    ~eEditMesh();

	enum MAPPINGS {
		SPHERICAL = 0,
		SPHERICAL_MERCATOR,
		CYLINDER,
		PLANE,
		CUBE
	};
	void					mapUVs(eU32 method, const eVector3	up, const eVector3	projectAxis, const eVector3	center, const eVector2	uvscale);

	void                    reserveSpace(eU32 vertexCount, eU32 faceCount);
    void                    setSize(eU32 vertexCount, eU32 faceCount);
	void					merge(eEditMesh &em);
    void                    merge(const eEditMesh &em);
    void                    clear();
    void                    clearAndPreallocate(eU32 numVertices, eU32 numFaces, eU32 numEdges);
    eBool                   triangulate();
    void                    optimizeIndices();
    void                    updateBoundingBox();
    void                    updateBoundingBox(const eVector3& pos);
    void                    updateNormals();
    void                    updateOutlineNormals();
    void                    setMaterial(const eMaterial *mat);
	void					centerMesh();
	void					transform(const eMatrix4x4 &mtx);

    Vertex *                addVertex(const eVector3 &pos, const eVector2 &texCoord, const eVector3 &normal);
    Vertex *                addVertex(const eVector3 &pos, const eVector2 &texCoord);
    Vertex *                addVertex(const eVector3 &pos);
    Vertex *                addVertex(const Vertex& vertex);
    Face *                  addTriangleFast(Vertex **vertices, const eVector2 *texCoords);
    Face *                  addFace(Vertex **vertices, const eVector2 *texCoords, eU32 vertexCount);
    Face *                  addFace(const eU32 *indices, const eVector2 *texCoords, eU32 indexCount);
    Face *                  addTriangle(Vertex *v0, Vertex *v1, Vertex *v2);
    Face *                  addTriangle(eU32 i0, eU32 i1, eU32 i2);
    Face *                  addQuad(Vertex *v0, Vertex *v1, Vertex *v2, Vertex *v3);
    Face *                  addQuad(eU32 i0, eU32 i1, eU32 i2, eU32 i3);

    eBool                   removeVertex(const Vertex *vtx);
    eBool                   removeVertex(eU32 index);
    eBool                   removeEdge(const Edge *edge);
    eBool                   removeEdge(eU32 index);
    void                    removeFace(const Face *face, eBool remFreeGeo=eTRUE);
    void                    removeFace(eU32 index, eBool remFreeGeo=eTRUE);

    eBool                   checkConsistency() const;
    eBool                   isTriangulated() const;
    eBool                   isEmpty() const;
    eU32                    getMaxFaceValence() const;
    eU32                    getVertexCount() const;
    eU32                    getEdgeCount() const;
    eU32                    getFaceCount() const;
    const eAABB &           getBoundingBox() const;
    eAABB &                 getBoundingBox();
	void					setBoundingBox(const eAABB & bbox);
	const Vertex *          getVertex(eU32 index) const;
    Vertex *                getVertex(eU32 index);
    const Edge *            getEdge(eU32 index) const;
    Edge *                  getEdge(eU32 index);
    const Face *            getFace(eU32 index) const;
    Face *                  getFace(eU32 index);

    eEditMesh &             operator = (eEditMesh &em);

private:
    HalfEdge *              _findFreeIncident(Vertex *vtx) const;
    HalfEdge *              _findFreeIncident(Vertex *vtx, HalfEdge *startHe, HalfEdge *endBeforeHe) const;
    HalfEdge *              _findHalfEdge(const Vertex *fromVtx, const Vertex *toVtx);
    eBool                   _makeAdjacent(HalfEdge *inHe, HalfEdge *outHe);
    Edge *                  _addEdge(Vertex *fromVtx, Vertex *toVtx);
    eBool                   _triangulateFace(Face *face);

private:
public:
	VertexPtrArray          m_vertices;
private:
    EdgePtrArray            m_edges;
    FacePtrArray            m_faces;
    eAABB                   m_bbox;
    eBool                   m_triangulated;

	eArray<Vertex>			m_preallocatedVertices;
	eArray<HalfEdge>		m_preallocatedHalfEdges;
	eArray<Edge>			m_preallocatedEdges;
	eArray<Face>			m_preallocatedFaces;
};

#endif // EDIT_MESH_HPP