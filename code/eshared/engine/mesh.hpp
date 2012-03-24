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

#ifndef MESH_HPP
#define MESH_HPP

// Encapsulates a non editable mesh which can hold
// different sorts of geometry and provides fast rendering.
class eMesh : public eINonVolatileResource
{
public:
    // Instance of a mesh for scene graph.
    class Instance : public eIRenderable
    {
    public:
        Instance(eMesh &mesh, eBool castsShadows=eFALSE);

        virtual void            update(eF32 time);
        virtual void            getRenderJobs(const eMatrix4x4 &mtx, const eMatrix4x4 &normalMtx, eRenderJobPtrArray &jobs, eU32 passID) const;
        virtual const eAABB &   getBoundingBox() const;
        virtual Type            getType() const;

        const eMesh&            getMesh() const { return this->m_mesh; };
    public:
        eBool                   getCastsShadows() const;

    private:
        eMesh &                 m_mesh;
        const eBool             m_castsShadows;
    };

    enum Type
    {
        TYPE_STATIC,
        TYPE_DYNAMIC
    };

    struct DrawSection
    {
        enum Type
        {
            TYPE_TRIANGLES,
            TYPE_LINES,
            TYPE_POINTS
        };

        Type                    type;
        eArray<eU32>            primitives;
        eArray<eVertex *>       vertices;
        eArray<eU32>            indices;
        const eMaterial *       material;
        eGeometry *             geometry;
    };

    struct Primitive
    {
        const eMaterial *       material;
        eU32                    indices[3];
    };

public:
    eMesh(Type type=eMesh::TYPE_DYNAMIC, eU32 primitiveCount=0, eU32 vertexCount=0);
    eMesh(Type type, eEditMesh &em);
    virtual ~eMesh();

    virtual eBool               upload();
    virtual eBool               unload();

public:
    void                        reserveSpace(eU32 primitiveCount, eU32 vertexCount);
    void                        finishLoading(Type type);
    void                        fromEditMesh(eEditMesh &em);
    void                        clear();

    eU32                        addVertex(const eVector3 &pos, const eVector3 &normal, const eVector2 &texCoord, const eColor &color=eColor::WHITE);
    eU32                        addVertex(const eVector3 &pos, const eColor &color=eColor::WHITE);
    eU32                        addVertexFast(const eVector3 &pos, const eVector3 &normal, const eVector2 &texCoord);
    eU32                        addTriangle(eU32 vtx0, eU32 vtx1, eU32 vtx2, const eMaterial *mat);
    eU32                        addTriangleFast(eU32 vtx0, eU32 vtx1, eU32 vtx2, eU32 drawSectionIdx);
    eU32                        addLine(eU32 startVtx, eU32 endVtx, const eMaterial *mat);
    eU32                        addLine(const eVector3 &pos0, const eVector3 &pos1, const eColor &color, const eMaterial *mat);
    eU32                        addPoint(eU32 vertex, const eMaterial *mat);
    eU32                        addPoint(const eVector3 &pos, const eColor &col, const eMaterial *mat);
    eU32                        addDrawSection(const eMaterial *mat, DrawSection::Type dsType);

    const eVertex &             getVertex(eU32 index) const;
    const Primitive &           getPrimitive(eU32 index) const;
    const DrawSection &         getDrawSection(eU32 index) const;
    DrawSection &               getDrawSection(eU32 index);

    eU32                        getVertexCount() const;
    eU32                        getPrimitiveCount() const;
    eU32                        getDrawSectionCount() const;

    const eAABB &               getBoundingBox() const;

    void                        merge(const eMesh &other, const eMatrix4x4& mtx = eMatrix4x4(), const eMatrix4x4& mtxRotOnly = eMatrix4x4());
public:
    static void                 createWireCube(eMesh &mesh, const eVector3 &size, const eMaterial *mat);

private:
    eU32                        _findDrawSectionIdx(const eMaterial *mat, DrawSection::Type dsType);
    eFORCEINLINE DrawSection&  _findDrawSection(const eMaterial *mat, DrawSection::Type dsType) {
        return m_drawSections[_findDrawSectionIdx(mat, dsType)];
    }
    eU32                        _addPrimitive(const eU32 *vertices, eU32 vertexCount, const eMaterial *mat, DrawSection::Type dsType);
    void                        _fromTriangleEditMesh(eEditMesh &em);

    eMesh &                     operator = (const eMesh &mesh);

private:
    static void                 _fillDynamicBuffers(ePtr param, eGeometry *geo);

private:
    typedef eArray<eVertex> eVertexArray;
    typedef eArray<Primitive> PrimitiveArray;
    typedef eArray<DrawSection> DrawSectionArray;

private:
    eVertexArray                m_vertices;
    PrimitiveArray              m_primitives;
    DrawSectionArray            m_drawSections;
    
    Type			            m_type;
    eAABB                       m_bbox;
};

#endif // MESH_HPP