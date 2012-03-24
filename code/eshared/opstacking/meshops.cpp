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

#include "../eshared.hpp"
#include "../math/math.hpp"
#include "../engine/engine.hpp"
#include "../modules/lsystem/lsystem.hpp"

// Import ESVG (mesh) operator
// ---------------------------
// Imports a scalable vector graphic as a mesh

#if defined(HAVE_OP_MESH_IMPORT_ESVG) || defined(eEDITOR)
class eBitFileLoader
{
public:
    eBitFileLoader() {
        this->bits = eNULL;
        this->offset = 0;
    }

    ~eBitFileLoader() {
        if(this->bits) {
            delete [] this->bits;
        }
    }

    bool LoadFile(eChar* fileName)
    {
        eFile f(fileName);

        if (!f.open())
        {
            return false;
        }

        eInt size = f.getSize();
        this->bits = new char[size * 8];

        for(eInt i = 0; i < size; i++)
        {
            unsigned char c;

            f.read(&c, 1);

            for(eInt k = 0; k < 8; k++)
            {
                this->bits[i * 8 + k] = (c & (1 << (7-k))) >> (7-k);
            }
        }

        return true;
    }

    eU32 ReadUnsignedInt(eU32 bits) {
        eU32 res = 0;
        for(eU32 i = 0; i < bits; i++) 
            res = (res << 1) + this->bits[offset++];
        return res;
    }

    eS32 ReadSignedInt(eU32 bits) {
        if(this->bits[offset++] == 1)
            return -(eS32)this->ReadUnsignedInt(bits-1);
        else
            return this->ReadUnsignedInt(bits-1);
    }


    char*    bits;
    eInt        offset;
};

#define ESVG_COMMAND_PATHREL 0
#define ESVG_COMMAND_PATHEND 1
#define ESVG_COMMAND_BITS 1

char buf[1000];

void DrawBezier(const eF32* ctl, eArray<eVector3>** contour, eU32 segs) {
    float step = (1.0f / (float)(segs));
    for(eU32 i = 0; i <= segs; i++) {                              
        float t = (float)i * step;
        float tinv = 1.0f - t;
        float res[2];
        for(eInt d = 0; d < 2; d++) {
            res[d] = 0;
            res[d] += tinv*tinv*tinv * ctl[0 + d];
            res[d] += 3 * tinv * tinv * t * ctl[2 + d];
            res[d] += 3 * tinv * t * t * ctl[4 + d];
            res[d] += t * t * t * ctl[6 + d];
        }
        eVector3 p(-res[0] + 0.5f, -res[1] + 0.5f, 0);
        

/*
        if((i == 0) && ((p.x != lastPos.x) || (p.y != lastPos.y) || (p.z != lastPos.z))) {
            if((*contour)->size() > 0) {
                if((*contour)->size() > 2)
                    contours.append(*contour);
                *contour = new eArray<eVector3>();
            }
        }
*/
        (*contour)->append(p);

//        sprintf(buf,"   BezPoint %f  %f   (%d) \n", p.x, p.y, (*contour)->size());
//        OutputDebugString(buf);
    }
}

OP_DEFINE_MESH(eImportESVGOp, eImportESVGOp_ID, "Import ESVG", ' ', 0, 0, "")
    OP_INIT()
    {
        eOP_PARAM_ADD_FILE("Path", "");
        eOP_PARAM_ADD_INT("Detail", 1, 64, 1);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, const eChar *path, eU32 detail) 
    {
        eBitFileLoader loader;
        eBool success = loader.LoadFile((eChar *)path);
        if(success) {
            //OutputDebugString("Successfully loaded esvg file\n");
            eInt ESVG_MOVE_COORD_BITS = loader.ReadUnsignedInt(6);
            eInt ESVG_PATH_SEGCNT_BITS = loader.ReadUnsignedInt(4);
            eInt ESVG_MIN_MAX_BITS = loader.ReadUnsignedInt(6);
            eInt ESVG_MIN_PATH_BITS = loader.ReadUnsignedInt(4);
            eInt ESVG_PATH_BITS_BITS = loader.ReadUnsignedInt(4);
            eInt ESVG_CACHE_IDX_BITS = loader.ReadUnsignedInt(4);
            eInt ESVG_CACHE_LEN_BITS = loader.ReadUnsignedInt(4);
            eInt pathCnt = loader.ReadUnsignedInt(16);
            eInt numVert = 0;

            struct tCacheEntry {
                eF32 cp[8];
                eU32 prevInCache;
            };
            eArray<tCacheEntry> cache;

		    eU32 debCacheCount = 0;

            for(eInt p = 0; p < pathCnt; p++) {

                char col[3];
                for(eInt i = 0; i < 3; i++)
                    col[i] = loader.ReadUnsignedInt(8);
    /*
                sprintf(buf,"Path %d \n", p);
                OutputDebugString(buf);
    */
    //            float max = ((float)loader.ReadSignedInt(ESVG_MIN_MAX_BITS)) / (float)(1 << (ESVG_MIN_MAX_BITS - 1));
                //sprintf(buf,"Path %d  Min= %f Max= %f\n", p, min, max);
                //OutputDebugString(buf);
    //            float conv = (max-min) / (double)((1 << ESVG_PATHCOORD_BITS)-1);

                eF32    pen[2] = {0,0};

                eArray<eVector3>* contour = new eArray<eVector3>();
                eArray<eArray<eVector3>*> contours;

                eVector3 lastPos(0, 0, 0.0f);
    //            eU32 firstCacheNrInPath = cache.size();
                eU32 firstCacheNrInPath = -1;
                eInt numPathes = 0;
                for(;;) {
                    eInt cmd = loader.ReadUnsignedInt(ESVG_COMMAND_BITS);
                    if(cmd == ESVG_COMMAND_PATHEND) {
					    if(firstCacheNrInPath != -1)
						    cache[firstCacheNrInPath].prevInCache = cache.size() - 1;
                        break;
                    }
                    if(cmd == ESVG_COMMAND_PATHREL) {
    //					sprintf(buf," Starting Path at %d \n", cache.size());
                        eF32    lastLen = 0;
    //					OutputDebugString(buf);
					    if(firstCacheNrInPath != -1)
		                    cache[firstCacheNrInPath].prevInCache = cache.size() - 1;
					    firstCacheNrInPath = cache.size();
					    numPathes++;

					    eInt ESVG_PATHCOORD_ANGLE_BITS = ESVG_MIN_PATH_BITS + loader.ReadUnsignedInt(ESVG_PATH_BITS_BITS);
					    eInt ESVG_PATHCOORD_LEN_BITS = ESVG_MIN_PATH_BITS + loader.ReadUnsignedInt(ESVG_PATH_BITS_BITS);
					    float lenSize = ((float)loader.ReadSignedInt(ESVG_MIN_MAX_BITS)) / (float)(1 << (ESVG_MIN_MAX_BITS - 1));

                        float lastCtrl2[2] = {0,0};

    //                    sprintf(buf,"  Bit pos %d \n", loader.offset);
    //					OutputDebugString(buf);

                        eInt segNr = -1;
                        while(true) {
                            segNr++;
                            eF32 ctl[8];
                            eInt isCoord = loader.ReadUnsignedInt(1);
    //						sprintf(buf," IsCoord %d \n", isCoord);
    //						OutputDebugString(buf);
                            if(isCoord == 1) {
                                if(segNr == 0) {
                                    // first segment, read initial pen
                                    pen[0] = ((float)loader.ReadUnsignedInt(ESVG_MOVE_COORD_BITS)) / (float)(1 << (ESVG_MOVE_COORD_BITS - 1));
                                    pen[1] = ((float)loader.ReadUnsignedInt(ESVG_MOVE_COORD_BITS)) / (float)(1 << (ESVG_MOVE_COORD_BITS - 1));
                                    lastCtrl2[0] = pen[0];
                                    lastCtrl2[1] = pen[1];
                                }
                                ctl[0] = pen[0];
                                ctl[1] = pen[1];
                                ctl[2] = eClamp(0.0f, pen[0] + (pen[0] - lastCtrl2[0]), 1.0f);
                                ctl[3] = eClamp(0.0f, pen[1] + (pen[1] - lastCtrl2[1]), 1.0f);

                                eInt angleStepsHalf = 1 << (ESVG_PATHCOORD_ANGLE_BITS - 1);
                                eInt lenStepsHalf = 1 << (ESVG_PATHCOORD_LEN_BITS - 1);

                                for(eInt i = 1; i < 4; i++) {
                                    float angle = ((float)loader.ReadUnsignedInt(ESVG_PATHCOORD_ANGLE_BITS)) / (float)(1 << (ESVG_PATHCOORD_ANGLE_BITS));

                                    float lenCode = (float)loader.ReadUnsignedInt(ESVG_PATHCOORD_LEN_BITS);
                                    float lenDiff = 0;
                                    if(lenCode < lenStepsHalf) lenDiff = -lenSize + (1.0f - ePow(1.0f - lenCode / (float)lenStepsHalf, eEXPONE)) * lenSize;
                                    else lenDiff = ePow((lenCode - (float)lenStepsHalf) / (float)(lenStepsHalf - 1), eEXPONE) * lenSize;

                                    angle = eClamp(0.0f, angle, 1.0f);
                                    eF32 len = eClamp(0.0f, lastLen + lenDiff, 1.0f);
                                    lastLen = len;

                                    const eF32 EPSILON = 0.00000001f;
                                    angle = (angle - 0.5f) * (2.0f * ePI + EPSILON);
                                    len *= eSqrt(2.0f);

                                    const eU32 idxX = i * 2;
                                    const eU32 idxY = idxX + 1;



                                    ctl[idxX] += eCos(angle) * len;
                                    ctl[idxY] += eSin(angle) * len;

    //							    sprintf(buf," Point: %f   %f \n", ctl[idxX], ctl[idxY]);
    //							    OutputDebugString(buf);


                                    if(i < 3) {
                                        ctl[idxX + 2] = eClamp(0.0f, ctl[idxX] + (ctl[idxX] - ctl[idxX - 2]), 1.0f);
                                        ctl[idxY + 2] = eClamp(0.0f, ctl[idxY] + (ctl[idxY] - ctl[idxY - 2]), 1.0f);
                                    }
                                }
    //							sprintf(buf," Normal : %f2.4 : %f2.4 : %f2.4 : %f2.4 : %f2.4 : %f2.4 : %f2.4 : %f2.4 \n", ctl[0],ctl[1],ctl[2],ctl[3],ctl[4],ctl[5],ctl[6],ctl[7]);
    //							OutputDebugString(buf);

    //                            if(segNr == 2)
                                DrawBezier(ctl,&contour, detail);
                                tCacheEntry ce;
                                eMemCopy(ce.cp, ctl, 8 * sizeof(eF32));
                                ce.prevInCache = cache.size() - 1;
                                cache.append(ce);
                            } else {
                                eU32 index = loader.ReadUnsignedInt(ESVG_CACHE_IDX_BITS);
                                eU32 len = loader.ReadUnsignedInt(ESVG_CACHE_LEN_BITS);
    //							sprintf(buf,"  Cached idx = %d len = %d \n", index, len);
    //							OutputDebugString(buf);
							    if(len == 0)
                                    break;
                                for(eU32 i = 0; i < (eU32)len; i++) {
                                    const tCacheEntry& ce = cache[index];
                                    index = ce.prevInCache;
                                    ctl[0] = ce.cp[6];
                                    ctl[1] = ce.cp[7];
                                    ctl[2] = ce.cp[4];
                                    ctl[3] = ce.cp[5];
                                    ctl[4] = ce.cp[2];
                                    ctl[5] = ce.cp[3];
                                    ctl[6] = ce.cp[0];
                                    ctl[7] = ce.cp[1];
    //								sprintf(buf," DebCacheCnt %d : %f2.4 : %f2.4 : %f2.4 : %f2.4 : %f2.4 : %f2.4 : %f2.4 : %f2.4 \n", debCacheCount, ctl[0],ctl[1],ctl[2],ctl[3],ctl[4],ctl[5],ctl[6],ctl[7]);
    //								OutputDebugString(buf);
								    debCacheCount++;
    //                                contour->append(eVector3(-ctl[0], -ctl[1], 0));
                                    DrawBezier(ctl, &contour, detail);
                                    tCacheEntry nce;
                                    eMemCopy(nce.cp, ctl, 8 * sizeof(eF32));
                                    nce.prevInCache = cache.size() - 1;
                                    cache.append(nce);

                                }
    /*
							    // pen correction
							    ctl[6] = ((float)loader.ReadUnsignedInt(ESVG_MOVE_COORD_BITS)) / (float)(1 << (ESVG_MOVE_COORD_BITS - 1));
							    ctl[7] = ((float)loader.ReadUnsignedInt(ESVG_MOVE_COORD_BITS)) / (float)(1 << (ESVG_MOVE_COORD_BITS - 1));
    */
                            }
                            lastCtrl2[0] = ctl[4];
                            lastCtrl2[1] = ctl[5];
                            pen[0] = ctl[6];
                            pen[1] = ctl[7];
                        }
                        if(contour->size() > 0) {
                            if(contour->size() > 2)
                                contours.append(contour);
                            contour = new eArray<eVector3>();
                        }
                    }                
    //                cache[firstCacheNrInPath].prevInCache = cache.size() - 1;
				    if(firstCacheNrInPath != -1)
		                cache[firstCacheNrInPath].prevInCache = cache.size() - 1;
                }
                eTriangulator tri;


                for(eU32 i = 0; i < contours.size(); i++) {
                    contour = contours[i];
    /*
                    for(eInt k = 0; k < contour->size(); k++) {
                        eVector3 v = (*contour)[k];
                        v.z = (float)p / 40.0f;
                        eEditMesh::Vertex* vert = m_mesh.addVertex(v, eNULL, eNULL);
                        vert->selected = true;
                        vert->tag = p + 1;
                    }
    */
                    tri.addContour(*contours[i]);
                }

                tri.triangulate();
                const eArray<eU32> &triangles = tri.getIndices();

                eMaterial *mat = new eMaterial;
                mat->setDiffuseColor(eColor((eU8)col[0], (eU8)col[1], (eU8)col[2]));
                //mat->setDiffuseColor(eColor(eRandom(0, 255), eRandom(0, 255), eRandom(0, 255)));

                eInt initialVertCount = m_mesh.getVertexCount();
                const eArray<eVector3> &vertices = tri.getVertices();
                for(eU32 i = 0; i < vertices.size(); i++) {
                    eVector3 v = vertices[i];
    //                v.z = (float)p / 10.0f;
                    eEditMesh::Vertex* vert = m_mesh.addVertex(v);
                    vert->selected = eTRUE;
                    vert->tag = p + 1;
                }

                for(eU32 i = 0; i < triangles.size(); i += 3)
                {
                        eEditMesh::Face *face = m_mesh.addTriangle(initialVertCount + triangles[i],
                                                                   initialVertCount + triangles[i + 1],
                                                                   initialVertCount + triangles[i + 2]);
                       face->material = mat;
                       face->selected = eTRUE;
                       face->tag = p + 1;

                }

            }
            //m_mesh.triangulate();
            //OutputDebugString("Finished loading esvg\n");
        }
        else
        {
            //OutputDebugString("Loading esvg file failed\n");
        }
    }
OP_END(eImportESVGOp);
#endif

// UV-Mapping (mesh) operator
// ---------------------
// Maps UV coordinates

#if defined(HAVE_OP_MESH_MAP_UV) || defined(eEDITOR)
OP_DEFINE_MESH(eUvMapOp, eUvMapOp_ID, "Map UV", ' ', 1, 1, "-1,Mesh")
    OP_INIT()
    {
	    eOP_PARAM_ADD_LABEL("General", "General");
	    eOP_PARAM_ADD_ENUM("Method", "Sphere|Sphere Mercator|Cylinder|Plane|Cube", 0);
        eOP_PARAM_ADD_FXYZ("Up", -1, 1, 0, 0, 1);
        eOP_PARAM_ADD_FXYZ("ProjectAxis", -1, 1, 0, 1, 0);
        eOP_PARAM_ADD_FXYZ("Center", -eF32_MAX, eF32_MAX, 0, 0, 0);
        eOP_PARAM_ADD_FXY("UVScale", 0, eF32_MAX, 1, 1);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eInt method, const eVector3 &up, const eVector3 &projAxis,
            const eVector3 &center, const eVector2 &uvScale) 
    {
        _copyFirstInputMesh();
	    m_mesh.mapUVs(method, up, projAxis, center, uvScale);
    }
OP_END(eUvMapOp);
#endif

// Plane (mesh) operator
// ---------------------
// Creates a 2-dimensional, segmented plane.

#if defined(HAVE_OP_MESH_PLANE) || defined(eEDITOR)
OP_DEFINE_MESH(ePlaneOp, ePlaneOp_ID, "Plane", 'p', 0, 0, "")
    OP_INIT()
    {
        eOP_PARAM_ADD_IXY("Segments", 1, 64, 1, 1);
        eOP_PARAM_ADD_FXY("Size", eALMOST_ZERO, eF32_MAX, 1.0f, 1.0f);
        eOP_PARAM_ADD_LINK("Material", "Misc : Material");
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, const ePoint &segs, const eVector2 &size, eIMaterialOp *matOp)
    {
        m_mesh.reserveSpace((segs.x+1)*(segs.y+1), segs.x*segs.y);

        const eVector2 posStep(size.x/(eF32)segs.x, size.y/(eF32)segs.y);
        const eVector2 uvStep(1.0f/(eF32)segs.x, 1.0f/(eF32)segs.y);

        eVector3 pos(-size.x*0.5f, 0.0f, size.y*0.5f);
        eVector2 uv;

        for (eInt y=0; y<segs.y+1; y++)
        {
            pos.x = -size.x*0.5f;
            uv.u = 0.0f;

            for (eInt x=0; x<segs.x+1; x++)
            {
                m_mesh.addVertex(pos, uv, eVector3::YAXIS);

                pos.x += posStep.x;
                uv.u += uvStep.u;
            }

            pos.z -= posStep.y;
            uv.v += uvStep.v;
        }

        for (eInt y=0; y<segs.y; y++)
        {
            for (eInt x=0; x<segs.x; x++)
            {
                const eU32 base0 = y*(segs.x+1)+x;
                const eU32 base1 = (y+1)*(segs.x+1)+x;

                m_mesh.addQuad(base1, base0, base0+1, base1+1)->normal = eVector3::YAXIS;
            }
        }

        // Set material if specified.
        if (matOp)
        {
            m_mesh.setMaterial(&matOp->getResult().material);
        }

        m_mesh.getBoundingBox().setCenterSize(eVector3::ORIGIN, eVector3(size.x, 0.01f, size.y));
    }
OP_END(ePlaneOp);
#endif

// Cube (mesh) operator
// --------------------
// Creates a one segment cube.

#if defined(HAVE_OP_MESH_CUBE) || defined(eEDITOR)
OP_DEFINE_MESH(eCubeOp, eCubeOp_ID, "Cube", 'q', 0, 0, "")
    OP_INIT()
    {
        eOP_PARAM_ADD_FXYZ("Size", eALMOST_ZERO, eF32_MAX, 1.0f, 1.0f, 1.0f);
        eOP_PARAM_ADD_LINK("Material", "Misc : Material");
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, const eVector3 &size, eIMaterialOp *matOp)
    {
        m_mesh.reserveSpace(8, 6);

        eVector3 vertices[] =
        {
            // top
            eVector3(-1.0f,  1.0f, -1.0f), //loh
            eVector3(-1.0f,  1.0f,  1.0f), //lov
            eVector3( 1.0f,  1.0f,  1.0f), //rov
            eVector3( 1.0f,  1.0f, -1.0f), //roh

            // bottom
            eVector3(-1.0f, -1.0f, -1.0f), //luh
            eVector3(-1.0f, -1.0f,  1.0f), //luv
            eVector3( 1.0f, -1.0f,  1.0f), //ruv
            eVector3( 1.0f, -1.0f, -1.0f), //ruh
        };

        for (eU32 i=0; i<8; i++)
        {
            m_mesh.addVertex(vertices[i]*0.5f);
        }

        eU32 indices[][4] =
        {
            { 0, 1, 2, 3 }, // top
            { 5, 4, 7, 6 }, // bottom
            { 1, 5, 6, 2 }, // front
            { 3, 7, 4, 0 }, // back
            { 0, 4, 5, 1 }, // left
            { 2, 6, 7, 3 }, // right
        };

        for (eU32 i=0; i<6; i++)
        {
            m_mesh.addFace(indices[i], eNULL, 4);
        }

        m_mesh.updateNormals();
	    m_mesh.mapUVs(eEditMesh::CUBE, eVector3::ZAXIS, eVector3::YAXIS, -eVector3(1)*0.5f, eVector2(eTWOPI));

		for(eU32 i = 0; i < 8; i++)
        {
			m_mesh.getVertex(i)->position.scale(size);
        }

        // Set material if specified.
        if (matOp)
        {
            m_mesh.setMaterial(&matOp->getResult().material);
        }
    }
OP_END(eCubeOp);
#endif

// Bicubic Patch (mesh) operator
// --------------------
#if defined(HAVE_OP_MESH_BICUBICPATCH) || defined(eEDITOR)
OP_DEFINE_MESH(eBicubicPatchOp, eBicubicPatchOp_ID, "BicubicPatch", ' ', 0, 0, "")
    eVector3 m_a[16];
	eVector3 m_size;

	OP_INIT()
    {
        eOP_PARAM_ADD_IXY("Segments", 1, 64, 3, 3);
        eOP_PARAM_ADD_FXYZ("Size", eALMOST_ZERO, eF32_MAX, 1.0f, 1.0f, 1.0f);

        for(eU32 i = 0; i < 3; i++) {
            eChar name[] = "xrow  ";
            name[0] += i;
    		eOP_PARAM_ADD_LABEL(name, name);
            for(eU32 k = 3; k < 4; k--) {
                name[5] = '0' + k;
        		eOP_PARAM_ADD_FXYZW(name, -eF32_MAX, eF32_MAX, 0.0f, 0.0f, 0.0f, 0.0f);
            }
        }
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, const ePoint &segs, const eVector3 &size, 
		const eVector4 &mx0, const eVector4 &mx1, const eVector4 &mx2, const eVector4 &mx3,
		const eVector4 &my0, const eVector4 &my1, const eVector4 &my2, const eVector4 &my3,
		const eVector4 &mz0, const eVector4 &mz1, const eVector4 &mz2, const eVector4 &mz3
		)
    {
		m_size = size;
		for(eU32 y = 0; y < 4; y++) {
			eVector4 mx,my,mz;
			switch(y) {
			case 0: mx = mx0; my = my0; mz = mz0; break;
			case 1: mx = mx1; my = my1; mz = mz1; break;
			case 2: mx = mx2; my = my2; mz = mz2; break;
			case 3: mx = mx3; my = my3; mz = mz3; break;
			}
			for(eU32 x = 0; x < 4; x++)
				m_a[y * 4 + x] = eVector3(mx[x], my[x], mz[x]);				
		}

		eU32 vcntx = (segs.x+1);
		eU32 vcnt = (segs.y+1) * vcntx;
        m_mesh.reserveSpace(vcnt, segs.x*segs.y);

		eF32 s_step = 1.0f / (eF32)segs.x;
		eF32 t_step = 1.0f / (eF32)segs.y;
		for(eInt y = 0; y <= segs.y; y++) {
			eF32 t = (eF32)y * t_step;
			for(eInt x = 0; x <= segs.x; x++) {
				eF32 s = (eF32)x * s_step;
				eVector3 cpy[4]; 
				eVector3 tany[4]; 
				eVector3::cubicBezier(t, m_a[0], m_a[1], m_a[2], m_a[3], cpy[0], tany[0]);
				eVector3::cubicBezier(t, m_a[4], m_a[5], m_a[6], m_a[7], cpy[1], tany[1]);
				eVector3::cubicBezier(t, m_a[8], m_a[9], m_a[10], m_a[11], cpy[2], tany[2]);
				eVector3::cubicBezier(t, m_a[12], m_a[13], m_a[14], m_a[15], cpy[3], tany[3]);
				eVector3 resultPos;
				eVector3 resultTangent;

				eVector3::cubicBezier(s, cpy[0], cpy[1], cpy[2], cpy[3], resultPos, resultTangent);

				m_mesh.addVertex(resultPos, eVector2(s,t));

				if((x!=0)&&(y!=0)) {
					eU32 idx = (y * vcntx + x);
					eU32 indices[] = {idx - vcntx - 1,idx - vcntx, idx, idx - 1};
					m_mesh.addFace(indices, eNULL, 4);
				}
			}
		}
        m_mesh.updateNormals();
		for(eU32 i = 0; i < vcnt; i++)
			m_mesh.getVertex(i)->position.scale(size);
		m_mesh.updateBoundingBox();
    }

    OP_INTERACT(eGraphicsApiDx9 *gfx, eSceneData &sg)
    {
        /*
        eGeometry geo(gfx, 16*3*2 , 16*3*2 , 16*3, eVTXTYPE_WIRE, eGeometry::TYPE_DYNAMIC_INDEXED, ePRIMTYPE_LINELIST);
        eWireVertex *verts = eNULL;
        eU32 *inds = eNULL;

		eF32 size2 = 0.1f;

        geo.startFilling((ePtr *)&verts, &inds);
        {
			eInt vcnt = 0;
			eInt icnt = 0;

			for(eU32 c = 0; c < 16; c++)
            {
				eVector3 pos = m_a[c];

				for(eU32 d = 0; d < 3; d++)
                {
					eVector3 dir = (d == 0) ? eVector3(1,0,0) : (d == 1) ? eVector3(0,1,0) : eVector3(0,0,1);
					eVector3 p1 = pos + dir * size2;
					p1.scale(m_size);
					verts[vcnt++].set(p1, eColor::CYAN);
					eVector3 p2 = pos - dir * size2;
					p2.scale(m_size);
					verts[vcnt++].set(p2, eColor::CYAN);
					inds[icnt] = icnt++;
					inds[icnt] = icnt++;
				}
			}
        }

        geo.stopFilling();
        geo.render();
        */
    }

OP_END(eBicubicPatchOp);
#endif

// Set material (mesh) operator
// ----------------------------
// Sets the material given in the second input
// operator to the mesh given in first input.

#if defined(HAVE_OP_MESH_SET_MATERIAL) || defined(eEDITOR)
OP_DEFINE_MESH(eSetMaterialOp, eSetMaterialOp_ID, "Set material", 's', 2, 2, "0,Mesh|1,Misc : Material")
    OP_INIT()
    {
        eOP_PARAM_ADD_ENUM("Apply on", "All faces|Selected faces|Unselected faces", 0);
        eOP_PARAM_ADD_INT("Tag", 0, 255, 0);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eInt selection, eU32 tag)
    {
        _copyFirstInputMesh();

        const eMaterial *mat = &((eIMaterialOp *)getInputOperator(1))->getResult().material;

        for (eU32 i=0; i<m_mesh.getFaceCount(); i++)
        {
            eEditMesh::Face *face = m_mesh.getFace(i);
            eASSERT(face != eNULL);

            const eBool tagging = (face->tag == tag || !tag);

            switch (selection)
            {
                case 0: // All
                {
                    if (tagging)
                    {
                        face->material = mat;
                    }

                    break;
                }

                case 1: // Selected
                {
                    if (face->selected && tagging)
                    {
                        face->material = mat;
                    }

                    break;
                }

                case 2: // Unselected
                {
                    if (!face->selected && tagging)
                    {
                        face->material = mat;
                    }

                    break;
                }
            }
        }
    }
OP_END(eSetMaterialOp);
#endif

#if defined(HAVE_OP_MESH_TRANSFORM) || defined(eEDITOR)
OP_DEFINE_MESH(eMeshTransformOp, eMeshTransformOp_ID, "Transform", 't', 1, 1, "-1,Mesh")
    OP_INIT()
    {
        eOP_PARAM_ADD_FXYZ("Translate", eF32_MIN, eF32_MAX, 0, 0, 0);
        eOP_PARAM_ADD_FXYZ("Rotate", eF32_MIN, eF32_MAX, 0, 0, 0);
        eOP_PARAM_ADD_FXYZ("Scale", eF32_MIN, eF32_MAX, 1, 1, 1);
        eOP_PARAM_ADD_BOOL("Reverse", eFALSE);
        eOP_PARAM_ADD_ENUM("Selection", "All|Selected|Unselected", 0);
        eOP_PARAM_ADD_INT("Tag", 0, 255, 0);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, const eVector3 &trans, const eVector3 &rotVal,
            const eVector3 &scale, eBool reverse, eInt selection, eU32 tag)
    {
        _copyFirstInputMesh();

        const eVector3 rot = rotVal*eTWOPI;
        eMatrix4x4 mtxPos;
        mtxPos.transformation(rot, trans, scale, reverse);

        eMatrix4x4 mtxNrm = mtxPos;
        mtxNrm.invert();
        mtxNrm.transpose();

        for (eU32 i=0; i<m_mesh.getVertexCount(); i++)
        {
            eEditMesh::Vertex *vtx = m_mesh.getVertex(i);
            eASSERT(vtx != eNULL);

            const eBool tagging = (vtx->tag == tag || !tag);

            switch (selection)
            {
                case 0: // All
                {
                    if (tagging)
                    {
                        vtx->position *= mtxPos;
                        vtx->normal *= mtxNrm;
                    }

                    break;
                }

                case 1: // Selected
                {
                    if (vtx->selected && tagging)
                    {
                        vtx->position *= mtxPos;
                        vtx->normal *= mtxNrm;
                    }

                    break;
                }

                case 2: // Unselected
                {
                    if (!vtx->selected && tagging)
                    {
                        vtx->position *= mtxPos;
                        vtx->normal *= mtxNrm;
                    }

                    break;
                }
            }
        }

        // Update face normals.
        for (eU32 i=0; i<m_mesh.getFaceCount(); i++)
        {
            m_mesh.getFace(i)->updateNormal();
        }

        // Finally update bounding box and normals.
        m_mesh.updateBoundingBox();
    }
OP_END(eMeshTransformOp);
#endif

// Delete (mesh) operator
// ----------------------
// Deletes selected primitives from a mesh.

#if defined(HAVE_OP_MESH_DELETE) || defined(eEDITOR)
OP_DEFINE_MESH(eMeshDeleteOp, eMeshDeleteOp_ID, "Delete", 'l', 1, 1, "-1,Mesh")
    OP_INIT()
    {
        eOP_PARAM_ADD_ENUM("Primitive", "Vertex|Edge|Face", 0);
        eOP_PARAM_ADD_INT("Tag", 0, 255, 0);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eInt prim, eU32 tag)
    {
        _copyFirstInputMesh();

        if (prim == 0)
        {
            for (eInt i=(eInt)m_mesh.getVertexCount()-1; i>=0; i--)
            {
                eEditMesh::Vertex *vtx = m_mesh.getVertex(i);
                eASSERT(vtx != eNULL);

                if (vtx->selected && (vtx->tag == 0 || vtx->tag == tag))
                {
                    m_mesh.removeVertex(i);
                }
            }
        }
        else if (prim == 1)
        {
            for (eInt i=(eInt)m_mesh.getEdgeCount()-1; i>=0; i--)
            {
                eEditMesh::Edge *edge = m_mesh.getEdge(i);
                eASSERT(edge != eNULL);

                if (edge->selected && (edge->tag == 0 || edge->tag == tag))
                {
                    m_mesh.removeEdge(i);
                }
            }
        }
        else if (prim == 2)
        {
            for (eInt i=(eInt)m_mesh.getFaceCount()-1; i>=0; i--)
            {
                eEditMesh::Face *face = m_mesh.getFace(i);
                eASSERT(face != eNULL);

                if (face->selected && (face->tag == 0 || face->tag == tag))
                {
                    m_mesh.removeFace(i);
                }
            }
        }
    }
OP_END(eMeshDeleteOp);
#endif

// Merge (mesh) operator
// ---------------------
// Merges multiple mesh operators together.

#if defined(HAVE_OP_MESH_MERGE) || defined(eEDITOR)
OP_DEFINE_MESH(eMeshMergeOp, eMeshMergeOp_ID, "Merge", 'm', 1, 64, "-1,Mesh")
    OP_EXEC(eGraphicsApiDx9 *gfx)
    {
        for (eU32 i=0; i<getInputCount(); i++)
        {
            const eEditMesh &mesh = ((eIMeshOp *)getInputOperator(i))->getResult().mesh;
            m_mesh.merge(mesh);
        }
    }
OP_END(eMeshMergeOp);
#endif

// Extract (mesh) operator
// -----------------------
// Extracts the selected faces to a new mesh.

#if defined(HAVE_OP_MESH_EXTRACT) || defined(eEDITOR)
OP_DEFINE_MESH(eMeshExtractOp, eMeshExtractOp_ID, "Extract faces", ' ', 1, 1, "-1,Mesh")
    OP_INIT()
    {
        eOP_PARAM_ADD_INT("Tag from", 0, 255, 0);
        eOP_PARAM_ADD_INT("Tag to", 0, 255, 0);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eU32 tagFrom, eU32 tagTo)
    {
        // Remove unselected faces from the copy. Maybe this
        // method is too inefficient for large meshes?
        _copyFirstInputMesh();

        for (eInt i=(eInt)m_mesh.getFaceCount()-1; i>=0; i--)
        {
            const eEditMesh::Face *face = m_mesh.getFace(i);
            eASSERT(face != eNULL);

            if (!face->selected || face->tag < tagFrom || face->tag > tagTo)
            {
                m_mesh.removeFace(i);
            }
        }

        m_mesh.updateBoundingBox();
        m_mesh.updateNormals();
    }
OP_END(eMeshExtractOp);
#endif

// Extract outline (mesh) operator
// -------------------------------
// Extracts the outline edges of a mesh.

#if defined(HAVE_OP_MESH_OUTLINE) || defined(eEDITOR)
OP_DEFINE_MESH(eMeshOutlineOp, eMeshOutlineOp_ID, "Outline", ' ', 1, 1, "-1,Mesh")
    OP_INIT()
    {
        eOP_PARAM_ADD_FLOAT("Subdiv", 0, eF32_MAX, 1.0f);
        eOP_PARAM_ADD_INT("Outline Points", 0, 100000, 1000);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eF32 subDiv, eU32 numPoints)
    {
        _copyFirstInputMesh();

        m_mesh.triangulate();
        m_mesh.updateNormals();
        m_mesh.updateOutlineNormals();

        eArray<eF32> len(m_mesh.getEdgeCount());
        eArray<eVector3> pos0(m_mesh.getEdgeCount());
        eArray<eVector3> pos1(m_mesh.getEdgeCount());
        eArray<eVector3> norm0(m_mesh.getEdgeCount());
        eArray<eVector3> norm1(m_mesh.getEdgeCount());

        for (eU32 i=0; i<m_mesh.getEdgeCount(); i++)
        {
            m_mesh.getEdge(i)->selected = eTRUE;
        }

        eF32 sumLen = 0;
        eInt cnt = 0;

        for (eU32 i=0; i<m_mesh.getEdgeCount(); i++)
        {
            const eEditMesh::Edge *edge = m_mesh.getEdge(i);

            if (edge->selected && edge->isBoundary())
            {
                const eEditMesh::HalfEdge *startEdge = edge->he1;
                const eEditMesh::HalfEdge *curEdge = startEdge;

                do
                {
                    curEdge->edge->selected = eFALSE;
                    len[cnt] = (curEdge->origin->position - curEdge->twin->origin->position).length();

                    if (len[cnt] != 0)
                    {
                        pos0[cnt] = curEdge->origin->position;
                        pos1[cnt] = curEdge->twin->origin->position;
                        norm0[cnt] = curEdge->origin->normal;
                        norm1[cnt] = curEdge->twin->origin->normal;
                        sumLen += len[cnt];
                        cnt++;
                    }

                    curEdge = curEdge->next;

                }
                while (curEdge != startEdge);
            }
        }

        m_mesh.clear();

        if (cnt > 0)
        {
            const eF32 step = sumLen/(eF32)numPoints;

            eU32 seg = 0;
            eF32 lineBase = 0;

            for (eU32 i=0; i<numPoints; i++)
            {
                const eF32 targetLinePos = step*(eF32)i;

                while (targetLinePos >= lineBase+len[seg]) 
                {
                    lineBase += len[seg++];
                }

                const eF32 s = (targetLinePos-lineBase)/len[seg];
                m_mesh.addVertex((pos0[seg])*(1.0f-s)+(pos1[seg])*s, eVector2(),
                                 ((norm0[seg])*(1.0f-s)+(norm1[seg])*s).normalized());
            }

        }
    }
OP_END(eMeshOutlineOp);
#endif

// Options UV (mesh) operator
// --------------------------
// Scrolls and scales texture coordinates
// of input mesh.

#if defined(HAVE_OP_MESH_OPTIONS_UV) || defined(eEDITOR)
OP_DEFINE_MESH(eOptionsUvOp, eOptionsUvOp_ID, "Options UV", ' ', 1, 1, "-1,Mesh")
    OP_INIT()
    {
        eOP_PARAM_ADD_FXY("Scale UV", eF32_MIN, eF32_MAX, 1.0f, 1.0f);
        eOP_PARAM_ADD_FXY("Scroll UV", eF32_MIN, eF32_MAX, 0.0f, 0.0f);
        eOP_PARAM_ADD_ENUM("Selection", "All|Selected|Unselected", 0);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, const eVector2 &scale, const eVector2 &scroll, eInt selection)
    {
        _copyFirstInputMesh();

        for(eU32 i = 0; i< m_mesh.getVertexCount(); i++)
            m_mesh.getVertex(i)->_volatile_attribute0 = eNULL;

        for (eU32 i=0; i<m_mesh.getFaceCount(); i++)
        {
            eEditMesh::Face *face = m_mesh.getFace(i);
            eASSERT(face != eNULL);

            if (selection == 0 || (selection == 1 && face->selected) || (selection == 2 && !face->selected))
            {
                eEditMesh::HalfEdge *he = face->he;
                eASSERT(he != eNULL);

                do
                {
                    he->texCoord.scale(scale);
                    he->texCoord += scroll;

                    eEditMesh::Vertex *vtx = he->origin;

                    if (vtx->_volatile_attribute0 == eNULL)
                    {
                        vtx->texCoord.scale(scale);
                        vtx->texCoord += scroll;
                        vtx->_volatile_attribute0 = (void*)1;
                    }

                    he = he->next;
                }
                while (he != face->he);
            }
        }
    }
OP_END(eOptionsUvOp);
#endif

// Torus (mesh) operator
// ---------------------
// Creates a torus.

#if defined(HAVE_OP_MESH_TORUS) || defined(eEDITOR)
OP_DEFINE_MESH(eTorusOp, eTorusOp_ID, "Torus", 'o', 0, 0, "")
    OP_INIT()
    {
        eOP_PARAM_ADD_FLOAT("Radius", eALMOST_ZERO, eF32_MAX, 1.0f);
        eOP_PARAM_ADD_FLOAT("Thickness", eALMOST_ZERO, eF32_MAX, 0.5f);
        eOP_PARAM_ADD_FLOAT("Phase", 0.0f, 1.0f, 0.0f);
        eOP_PARAM_ADD_IXY("Segments", 3, 32, 8, 16);
        eOP_PARAM_ADD_LINK("Material", "Misc : Material");
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eF32 radius, eF32 thickness, eF32 phaseVal, const ePoint &segs, eIMaterialOp *matOp)
    {
        const eF32 phase = phaseVal*eTWOPI;

        // Calculate the first ring.
        for (eInt i=0; i<segs.x; i++)
        {
            eVector3 v(thickness, 0.0f, 0.0f);
            v.rotate(eVector3(0.0f, 0.0f, i*eTWOPI/(eF32)segs.x+phase));
            v.x += radius;

            m_mesh.addVertex(v, eVector2(1.0f, 1.0f-(eF32)i/(eF32)segs.x));
        }

        // Rotate the current ring vertices in order to
        // obtain the other ring vertices.
        for (eInt j=1; j<segs.y; j++)
        {
            for (eInt i=0; i<segs.x; i++)
            {
                const eEditMesh::Vertex *vtx = m_mesh.getVertex(i);
                eASSERT(vtx != eNULL);

                eVector3 v = vtx->position;
                v.rotate(eVector3(0.0f, j*eTWOPI/(eF32)segs.y, 0.0f));
                m_mesh.addVertex(v, eVector2(1.0f-j/(eF32)segs.y, vtx->texCoord.y)); 
            }
        }

        // Connect vertices to build faces.
        for (eInt j=0; j<segs.y; j++)
        {
            for (eInt i=0; i<segs.x; i++)
            {
                const eU32 indices[4] =
                {
                    j*segs.x+i,
                    ((j+1)%segs.y)*segs.x+i,
                    ((j+1)%segs.y)*segs.x+((i+1)%segs.x),
                    j*segs.x+((i+1)%segs.x)
                };

                eVector2 texCoords[4] =
                {
                    m_mesh.getVertex(indices[0])->texCoord,
                    m_mesh.getVertex(indices[1])->texCoord,
                    m_mesh.getVertex(indices[2])->texCoord,
                    m_mesh.getVertex(indices[3])->texCoord
                };

                // Fix texture coordinates.
                if (i == segs.x-1)
                {
                    texCoords[2].y = 0.0f;
                    texCoords[3].y = 0.0f;
                }

                if (j == segs.y-1)
                {
                    texCoords[2].x = 0.0f;
                    texCoords[1].x = 0.0f;
                }

                m_mesh.addFace(indices, texCoords, 4);
            }
        }

        // Finally calculate the normals.
        m_mesh.updateNormals();

        // Set material if specified.
        if (matOp)
        {
            m_mesh.setMaterial(&matOp->getResult().material);
        }
    }
OP_END(eTorusOp);
#endif

// Ring (mesh) operator
// --------------------
// Creates a ring facing upwards or sidewards.

#if defined(HAVE_OP_MESH_RING) || defined(eEDITOR)
OP_DEFINE_MESH(eRingOp, eRingOp_ID, "Ring", 'r', 0, 0, "")
    OP_INIT()
    {
        eOP_PARAM_ADD_FLOAT("Radius", eALMOST_ZERO, eF32_MAX, 1.0f);
        eOP_PARAM_ADD_FLOAT("Height", eALMOST_ZERO, eF32_MAX, 1.0f);
        eOP_PARAM_ADD_INT("Segments X", 3, 32, 8);
        eOP_PARAM_ADD_INT("Segments Y", 1, 32, 1);
        eOP_PARAM_ADD_ENUM("Mode", "Facing upwards|Facing sidewards", 0);
        eOP_PARAM_ADD_LINK("Material", "Misc : Material");
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eF32 radius, eF32 height, eU32 segsX, eU32 segsY, eInt mode, eIMaterialOp *matOp)
    {
        // Precalculate some step values.
        const eF32 tuStep = 1.0f/segsX;
        const eF32 tvStep = 1.0f/segsY;
        const eF32 phiStep = eTWOPI/segsX;
        const eF32 radiusStep = height/segsY;

        // Generate upwards facing ring.
        if (mode == 0)
        {
            eF32 r = radius;

            for (eU32 i=0; i<segsY+1; i++)
            {
                eF32 phi = 0;

                for (eU32 j=0; j<segsX; j++)
                {
                    const eVector3 pos(eCos(phi)*r, eSin(phi)*r, 0.0f);

                    m_mesh.addVertex(pos);
                    phi += phiStep;
                }

                r += radiusStep;
            }
        }
        else // Generate sidewards facing ring.
        {
            eF32 ringZ = -height/2.0f;

            for (eU32 i=0; i<segsY+1; i++)
            {
                eF32 phi = 0.0f;

                for (eU32 j=0; j<segsX; j++)
                {
                    const eVector3 pos(eCos(phi)*radius, eSin(phi)*radius, ringZ);

                    m_mesh.addVertex(pos);
                    phi += phiStep;
                }

                ringZ += radiusStep;
            }
        }

        // Create ring quads.
        for (eU32 i=0; i<segsY; i++)
        {
            const eInt row0 = i*segsX;
            const eInt row1 = (i+1)*segsX;

            for (eU32 j=0; j<segsX; j++)
            {
                const eU32 col0 = j;
                const eU32 col1 = (j+1)%segsX;

                const eU32 indices[4] =
                {
                    row0+col0,
                    row0+col1,
                    row1+col1,
                    row1+col0
                };

                const eVector2 texCoords[4] =
                {
                    eVector2(1.0f-(eF32)i/segsY,     (eF32)j/segsX),
                    eVector2(1.0f-(eF32)i/segsY,     (eF32)(j+1)/segsX),
                    eVector2(1.0f-(eF32)(i+1)/segsY, (eF32)(j+1)/segsX),
                    eVector2(1.0f-(eF32)(i+1)/segsY, (eF32)j/segsX),
                };

                m_mesh.addFace(indices, texCoords, 4);
            }
        }

        m_mesh.updateNormals();

        // Set material if specified.
        if (matOp)
        {
            m_mesh.setMaterial(&matOp->getResult().material);
        }
    }
OP_END(eRingOp);
#endif

// Attractor (mesh) operator
// -------------------------
// Attracts vertices towards a point.

#if defined(HAVE_OP_MESH_ATTRACTOR) || defined(eEDITOR)
OP_DEFINE_MESH(eMeshAttractorOp, eMeshAttractorOp_ID, "Attractor", 'a', 1, 1, "-1,Mesh")
    OP_INIT()
    {
        eOP_PARAM_ADD_FXYZ("Position", eF32_MIN, eF32_MAX, 0.0f, 0.0f, 0.0f);
        eOP_PARAM_ADD_FLOAT("Range", 0.0f, eF32_MAX, 10.0f);
        eOP_PARAM_ADD_FLOAT("Power", 0.1f, 20.0f, 2.0f);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, const eVector3 &pos, eF32 range, eF32 power)
    {
        _copyFirstInputMesh();

        const eF32 rr = range*range;

        for (eU32 i=0; i<m_mesh.getVertexCount(); i++)
        {
            eEditMesh::Vertex *vtx = m_mesh.getVertex(i);
            eASSERT(vtx != NULL);

            const eVector3 dv = pos-vtx->position;
            const eF32 dist = dv.sqrLength();

            if (dist <= rr)
            {
                const eF32 att = 1.0f-ePow(dist/rr, power);
                vtx->position += dv*att;
            }
        }

        m_mesh.updateNormals();
    }
OP_END(eMeshAttractorOp);
#endif

// Vertex noise (mesh) operator
// ----------------------------
// Randomly translates vertices.

#if defined(HAVE_OP_MESH_VERTEX_NOISE) || defined(eEDITOR)
OP_DEFINE_MESH(eVertexNoiseOp, eVertexNoiseOp_ID, "Vertex noise", 'n', 1, 1, "-1,Mesh")
    OP_INIT()
    {
        eOP_PARAM_ADD_FXYZ("Amount", eF32_MIN, eF32_MAX, 1.0f, 1.0f, 1.0f);
        eOP_PARAM_ADD_INT("Seed", 0, 65535, 0);
        eOP_PARAM_ADD_ENUM("Mode", "All directions|By normal", 0);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, const eVector3 &amount, eU32 seed, eInt mode)
    {
        _copyFirstInputMesh();
        eRandomize(seed);

        if (mode == 0) // all directions
        {
            for (eU32 i=0; i<m_mesh.getVertexCount(); i++)
            {
                eEditMesh::Vertex *vtx = m_mesh.getVertex(i);
                eASSERT(vtx != NULL);

                vtx->position.x += amount.x*(eRandomF()-0.5f);
                vtx->position.y += amount.y*(eRandomF()-0.5f);
                vtx->position.z += amount.z*(eRandomF()-0.5f);
            }
        }
        else if (mode == 1) // by normal
        {
            for (eU32 i=0; i<m_mesh.getVertexCount(); i++)
            {
                eEditMesh::Vertex *vtx = m_mesh.getVertex(i);
                eASSERT(vtx != NULL);

                vtx->position.x += vtx->normal.x*(amount.x*(eRandomF()-0.5f));
                vtx->position.y += vtx->normal.y*(amount.y*(eRandomF()-0.5f));
                vtx->position.z += vtx->normal.z*(amount.z*(eRandomF()-0.5f));
            }
        }

        m_mesh.updateNormals();
    }
OP_END(eVertexNoiseOp);
#endif

// Select cube (mesh) operator
// ---------------------------
// Selects all primitives (choosable: vertices,
// edges or polygons), which lie inside a cube. 

#if defined(HAVE_OP_MESH_SELECT_CUBE) || defined(eEDITOR)
OP_DEFINE_MESH(eSelectCubeOp, eSelectCubeOp_ID, "Select cube", ' ', 1, 1, "-1,Mesh")
    OP_INIT()
    {
        eOP_PARAM_ADD_ENUM("Primitive", "Vertex|Edge|Face", 0);
        eOP_PARAM_ADD_ENUM("Mode", "Set|Add|Remove|Toggle", 0);
        eOP_PARAM_ADD_INT("Tag", 0, 255, 0);
        eOP_PARAM_ADD_LABEL("Cube", "Cube");
        eOP_PARAM_ADD_FXYZ("Translate", eF32_MIN, eF32_MAX, 0, 0, 0);
        eOP_PARAM_ADD_FXYZ("Rotate", eF32_MIN, eF32_MAX, 0, 0, 0);
        eOP_PARAM_ADD_FXYZ("Scale", eF32_MIN, eF32_MAX, 1, 1, 1);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eInt primitive, eInt mode, eU32 tag,
            const eVector3 &trans, const eVector3 &rot, const eVector3 &scale)
    {
        _copyFirstInputMesh();

        // Create transformation matrix.
        eMatrix4x4 mtx(rot*eTWOPI, trans, scale);

        // Create selection cube's vertices and side planes.
        m_cubeVerts[0] = eVector3(-0.5f, -0.5f, -0.5f)*mtx; // ulh
        m_cubeVerts[1] = eVector3(-0.5f, -0.5f,  0.5f)*mtx; // ulv
        m_cubeVerts[2] = eVector3( 0.5f, -0.5f,  0.5f)*mtx; // urv
        m_cubeVerts[3] = eVector3( 0.5f, -0.5f, -0.5f)*mtx; // urh
        m_cubeVerts[4] = eVector3(-0.5f,  0.5f, -0.5f)*mtx; // olh
        m_cubeVerts[5] = eVector3(-0.5f,  0.5f,  0.5f)*mtx; // olv
        m_cubeVerts[6] = eVector3( 0.5f,  0.5f,  0.5f)*mtx; // orv
        m_cubeVerts[7] = eVector3( 0.5f,  0.5f, -0.5f)*mtx; // orh

        const ePlane cubePlanes[6] =
        {
            ePlane(m_cubeVerts[2], m_cubeVerts[0], m_cubeVerts[1]), // bottom
            ePlane(m_cubeVerts[5], m_cubeVerts[4], m_cubeVerts[6]), // top
            ePlane(m_cubeVerts[3], m_cubeVerts[2], m_cubeVerts[6]), // right 
            ePlane(m_cubeVerts[4], m_cubeVerts[1], m_cubeVerts[0]), // left
            ePlane(m_cubeVerts[5], m_cubeVerts[2], m_cubeVerts[1]), // front
            ePlane(m_cubeVerts[0], m_cubeVerts[3], m_cubeVerts[4]), // back
        };

        // Select desired primitve types, if they lie
        // inside the selection m_cubeVerts.
        if (primitive == 0) // Vertex
        {
            for (eU32 i=0; i<m_mesh.getVertexCount(); i++)
            {
                eEditMesh::Vertex *vtx = m_mesh.getVertex(i);
                eASSERT(vtx != eNULL);

                if (vtx->position.isInsideCube(cubePlanes))
                {
                    selectPrimitive(vtx->selected, mode);
                    vtx->tag = tag;
                }
                else if (mode == 0) // Set
                {
                    vtx->selected = eFALSE;
                    vtx->tag = 0;
                }
             }
        }
        else if (primitive == 1) // Edge
        {
            for (eU32 i=0; i<m_mesh.getEdgeCount(); i++)
            {
                eEditMesh::Edge *edge = m_mesh.getEdge(i);
                eASSERT(edge != eNULL);

                const eVector3 &v0 = edge->he0->origin->position;
                const eVector3 &v1 = edge->he1->origin->position;

                if (v0.isInsideCube(cubePlanes) && v1.isInsideCube(cubePlanes))
                {
                    selectPrimitive(edge->selected, mode);
                    edge->tag = tag;
                }
                else if (mode == 0) // Set
                {
                    edge->selected = eFALSE;
                    edge->tag = 0;
                }
            }
        }
        else // Face
        {
            for (eU32 i=0; i<m_mesh.getFaceCount(); i++)
            {
                eEditMesh::Face *face = m_mesh.getFace(i);
                eASSERT(face != eNULL);

                if (mode == 0) // Set
                {
                    face->selected = eFALSE;
                    face->tag = 0;
                }

                eEditMesh::HalfEdge *he = face->he;
                eASSERT(he != eNULL);

                eBool selectFace = eTRUE;

                do
                {
                    if (!he->origin->position.isInsideCube(cubePlanes))
                    {
                        selectFace = eFALSE;
                        break;
                    }

                    he = he->next;
                }
                while (he != face->he);

                if (selectFace)
                {
                    selectPrimitive(face->selected, mode);
                    face->tag = tag;
                }
            }
        }
    }

    OP_INTERACT(eGraphicsApiDx9 *gfx, eSceneData &sg)
    {
        static eMesh mesh(eMesh::TYPE_DYNAMIC);
        static eMesh::Instance mi(mesh);

        if (mesh.getVertexCount() == 0) // Is already initialized?
        {
            eMesh::createWireCube(mesh, eVector3(1.0f), eMaterial::getWireframe());
        }

        const eVector3 &rot = getParameter(5).getValue().fxyz;
        const eVector3 &trans = getParameter(4).getValue().fxyz;
        const eVector3 &scale = getParameter(6).getValue().fxyz;

        sg.addRenderable(&mi, eMatrix4x4(rot*eTWOPI, trans, scale));
    }

    static void selectPrimitive(eBool &selected, eInt mode)
    {
        eASSERT(mode >= 0 && mode <= 3);

        const eBool todo[4] =
        {
            eTRUE,      // set
            eTRUE,      // add
            eFALSE,     // remove
            !selected   // toggle
        };

        selected = todo[mode];
    }

    OP_VAR(eVector3 m_cubeVerts[8]);
OP_END(eSelectCubeOp);
#endif

// Select random (mesh) operator
// -----------------------------
// Randomly selects primitives in the input mesh.

#if defined(HAVE_OP_MESH_SELECT_RANDOM) || defined(eEDITOR)
OP_DEFINE_MESH(eSelectRandomOp, eSelectRandomOp_ID, "Select random", ' ', 1, 1, "-1,Mesh")
    OP_INIT()
    {
        eOP_PARAM_ADD_ENUM("Primitive", "Vertex|Edge|Face", 0);
        eOP_PARAM_ADD_ENUM("Mode", "Set|Add|Remove|Toggle", 0);
        eOP_PARAM_ADD_INT("Tag", 0, 255, 0);
        eOP_PARAM_ADD_BOOL("Allow face neighbours", eFALSE);
        eOP_PARAM_ADD_FLOAT("Amount", 0.0f, 1.0f, 0.25f);
        eOP_PARAM_ADD_INT("Seed", 0, 65535, 0);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eInt primitive, eInt mode, eU32 tag, eBool afn, eF32 amount, eU32 seed)
    {
        _copyFirstInputMesh();

        eRandomize(seed);

        // If selection mode is "set", unselect
        // primitives first.
        if (mode == 0) // Set
        {
            if (primitive == 0) // Vertex
            {
                for (eU32 i=0; i<m_mesh.getVertexCount(); i++)
                {
                    m_mesh.getVertex(i)->selected = eFALSE;
                    m_mesh.getVertex(i)->tag = 0;
                }
            }
            else if (primitive == 1) // Edge
            {
                for (eU32 i=0; i<m_mesh.getEdgeCount(); i++)
                {
                    m_mesh.getEdge(i)->selected = eFALSE;
                    m_mesh.getEdge(i)->tag = 0;
                }
            }
            else // Face
            {
                for (eU32 i=0; i<m_mesh.getFaceCount(); i++)
                {
                    m_mesh.getFace(i)->selected = eFALSE;
                    m_mesh.getFace(i)->tag = 0;
                }
            }
        }

        // Select given amount of primtives.
        if (primitive == 0) // Vertex
        {
            for (eU32 i=0; i<m_mesh.getVertexCount(); i++)
            {
                if (eRandomF() <= amount)
                {
                    eSelectCubeOp::selectPrimitive(m_mesh.getVertex(i)->selected, mode);
                    m_mesh.getVertex(i)->tag = tag;
                }
            }
        }
        else if (primitive == 1) // Edge
        {
            for (eU32 i=0; i<m_mesh.getEdgeCount(); i++)
            {
                if (eRandomF() <= amount)
                {
                    eSelectCubeOp::selectPrimitive(m_mesh.getEdge(i)->selected, mode);
                    m_mesh.getEdge(i)->tag = tag;
                }
            }
        }
        else // Face
        {
            for (eU32 i=0; i<m_mesh.getFaceCount(); i++)
            {
                eEditMesh::Face *face = m_mesh.getFace(i);
                eASSERT(face != eNULL);

                // Is there already a selected neighbour face.
                eEditMesh::HalfEdge *he = face->he;
                eBool selNeighbour = eFALSE;

                if (!afn)
                {
                    do
                    {
                        if (he->twin->face && he->twin->face->selected)
                        {
                            selNeighbour = eTRUE;
                            break;
                        }

                        he = he->next;
                    }
                    while (he != face->he);
                }

                // Is this face allowed to be selected?
                if (eRandomF() <= amount && (afn || (!afn && !selNeighbour)))
                {
                    eSelectCubeOp::selectPrimitive(m_mesh.getFace(i)->selected, mode);
                    m_mesh.getFace(i)->tag = tag;
                }
            }
        }
    }
OP_END(eSelectRandomOp);
#endif

// Displace (mesh) operator
// ------------------------
// Displaces a mesh (first input operator) using
// pixel information from a bitmap (second operator).

#if defined(HAVE_OP_MESH_DISPLACE) || defined(eEDITOR)
OP_DEFINE_MESH(eDisplaceOp, eDisplaceOp_ID, "Displace", 'd', 2, 2, "0,Mesh|1,Bitmap")
    OP_INIT()
    {
        eOP_PARAM_ADD_ENUM("Mode", "All directions|Normal direction", 0);
        eOP_PARAM_ADD_FLOAT("Range", eF32_MIN, eF32_MAX, 1.0f);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eInt mode, eF32 range)
    {
        _copyFirstInputMesh();

        const eIBitmapOp::Result &bmpRes = ((eIBitmapOp *)getInputOperator(1))->getResult();

        const eU32 mulWidth = bmpRes.width-1;
        const eU32 mulHeight = bmpRes.height-1;

        for (eU32 i=0; i<m_mesh.getVertexCount(); i++)
        {
            eEditMesh::Vertex *vtx = m_mesh.getVertex(i);
            eASSERT(vtx != eNULL);

            const eVector2 &texCoord = vtx->he->texCoord;
            const eInt tx = eFtoL(eClamp(0.0f, texCoord.u, 1.0f)*(eF32)mulWidth);
            const eInt ty = eFtoL(eClamp(0.0f, texCoord.v, 1.0f)*(eF32)mulHeight);

            const eColor pixel = bmpRes.bitmap[ty*bmpRes.width+tx];
            const eF32 displace = (eF32)pixel.grayScale()/255.0f*range;

            if (mode == 0) // All directions
            {
                vtx->position *= displace;
            }
            else // Normal direction
            {
                vtx->position += vtx->normal*displace;
            }
        }

        m_mesh.updateNormals();
    }
OP_END(eDisplaceOp);
#endif

// Inside out (mesh) operator
// --------------------------
// Negates the position of all vertices of the mesh.

#if defined(HAVE_OP_MESH_INSIDEOUT) || defined(eEDITOR)
OP_DEFINE_MESH(eInsideOutOp, eInsideOutOp_ID, "Insideout", 'i', 1, 1, "-1,Mesh")
    OP_EXEC(eGraphicsApiDx9 *gfx)
    {
        _copyFirstInputMesh();

        for (eU32 i=0; i<m_mesh.getVertexCount(); i++)
        {
            m_mesh.getVertex(i)->position.negate();
        }

        m_mesh.updateBoundingBox();
        m_mesh.updateNormals();
    }
OP_END(eInsideOutOp);
#endif

// Sphere (mesh) operator
// ----------------------
// Generates a sphere.

#if defined(HAVE_OP_MESH_SPHERE) || defined(eEDITOR)
OP_DEFINE_MESH(eSphereOp, eSphereOp_ID, "Sphere", 'h', 0, 0, "")
    OP_INIT()
    {
        eOP_PARAM_ADD_IXY("Segments", 3, 64, 8, 4);
        eOP_PARAM_ADD_FLOAT("Radius", 0.01f, eF32_MAX, 0.5f);
        eOP_PARAM_ADD_LINK("Material", "Misc : Material");
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, const eIXY &segs, eF32 radius, eIMaterialOp *matOp)
    {
        const eVector2 rotStep(eTWOPI/(eF32)segs.x, ePI/(eF32)segs.y);

        // Add vertices to mesh.
        m_mesh.reserveSpace(2+segs.x*(segs.y-1), 2*segs.x + segs.x*(segs.y-2));

        m_mesh.addVertex(eVector3(0, radius, 0));
        m_mesh.addVertex(eVector3(0, -radius, 0));

        const eF32 stepAlpha = 360.0f/segs.x;
        const eF32 stepBeta = 180.0f/segs.y;

        for (eInt j=1; j<segs.y; j++)
        {
            for (eInt i=0; i<segs.x; i++)
            {
                eVector3 pos(radius, 0.0f, 0.0f);

                pos.rotate(eVector3::YAXIS*eDegToRad(-90.0f+stepBeta*j));
                pos.rotate(eVector3::ZAXIS*eDegToRad(stepAlpha*i));
                eSwap(pos.y, pos.z);

                m_mesh.addVertex(pos);
            }
        }

        // Add faces to mesh.
        for (eInt i=0; i<segs.x; i++)
        {
            // Add top triangles.
            const eU32 indices_top[3] =
            {
                0,
                _wrapGetVertex(i+1, 0, segs.x),
                _wrapGetVertex(i, 0, segs.x),
            };

            m_mesh.addFace(indices_top, eNULL, 3);

            // Add bottom triangles.
            const eU32 indices_bottom[3] =
            {
                _wrapGetVertex(i, segs.y-2, segs.x),
                _wrapGetVertex(i+1, segs.y-2, segs.x),
                1
            };

            m_mesh.addFace(indices_bottom, eNULL, 3);

            // Add faces of sphere.
            for (eInt j=0; j<segs.y-2; j++)
            {
                const eU32 indices[4] =
                {
                    _wrapGetVertex(i,   j,   segs.x),
                    _wrapGetVertex(i+1, j,   segs.x),
                    _wrapGetVertex(i+1, j+1, segs.x),
                    _wrapGetVertex(i,   j+1, segs.x),
                };

                m_mesh.addFace(indices, eNULL, 4);
            }
        }

        // Finally setup bounding box and calculate normals.
	    m_mesh.mapUVs(eEditMesh::SPHERICAL, eVector3::ZAXIS, eVector3::YAXIS, eVector3(), eVector2(1));
        m_mesh.updateNormals();

        // Set material if specified.
        if (matOp)
        {
            m_mesh.setMaterial(&matOp->getResult().material);
        }
    }

    eU32 _wrapGetVertex(eInt around, eInt down, eInt segsX)
    {
        return (around%(segsX))+down*segsX+2;
    }
OP_END(eSphereOp);
#endif


// Subdivide (mesh) operator
// -------------------------
// Refinement operation, which subdivides and
// smoothes input mesh (after n iterations, a
// cube gets a sphere).

#if defined(HAVE_OP_MESH_SUBDIVIDE) || defined(eEDITOR)
OP_DEFINE_MESH(eSubdivideOp, eSubdivideOp_ID, "Subdivide", 'u', 1, 1, "-1,Mesh")
    OP_INIT()
    {
        eOP_PARAM_ADD_ENUM("Iterations", "1|2|3|4|5", 0);
        eOP_PARAM_ADD_FLOAT("Smoothness", 0.0f, 1.0f, 1.0f);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eU32 iterations, eF32 smoothness)
    {
        _copyFirstInputMesh();

        eEditMesh wm;

        eEditMesh *workMesh = &wm;
        eEditMesh *realMesh = &m_mesh;

        for (eU32 iters = 0; iters <= iterations; iters++)
        {
            eASSERT(workMesh->isEmpty());
            const eU32 newVertexCount = realMesh->getVertexCount() + realMesh->getEdgeCount() + realMesh->getFaceCount();
            const eU32 newPolyCount = realMesh->getFaceCount() * realMesh->getMaxFaceValence();

            workMesh->clear();
            workMesh->reserveSpace(newVertexCount, newPolyCount);

            // Add the P vertices, which are inserted into the middle of the faces.
		    for (eU32 i = 0; i < realMesh->getFaceCount(); i++)
            {
                eEditMesh::Face *face = realMesh->getFace(i);
                eASSERT(face != eNULL);

			    eEditMesh::Vertex *newVtx = workMesh->addVertex(face->getCenter());
			    face->_volatile_attribute0 = newVtx;
            }

            // Add the E vertices that are inserted in the middle of the edges.
		    for (eU32 i = 0; i < realMesh->getEdgeCount(); i++)
            {
                eEditMesh::Edge *edge = realMesh->getEdge(i);
                eASSERT(edge != eNULL);

                // Calculate non-smoothed position.
                const eEditMesh::Vertex *endVtx0 = edge->he0->origin;
                const eEditMesh::Vertex *endVtx1 = edge->he1->origin;
			    const eEditMesh::Vertex *faceVtx0 = (const eEditMesh::Vertex *)edge->he0->face->_volatile_attribute0;
			    const eEditMesh::Vertex *faceVtx1 = edge->he1->face ? (const eEditMesh::Vertex *)edge->he1->face->_volatile_attribute0 : eNULL;
            
			    eEditMesh::Vertex vtx;
			    vtx.lerp(*endVtx0, *endVtx1, 0.5f);

			    if (!edge->isBoundary())
                {
                    eASSERT(edge->he0 == edge->he1->twin);

				    // Sum up the two end vertices of the edge and
                    // add the data from the center face vertex from
                    // both adjacent faces.
				    eEditMesh::Vertex vtx2;

				    vtx2.lerp(*endVtx0, *endVtx0, 0);
				    vtx2.add(*endVtx1);

				    if (faceVtx1)
                    {
					    vtx2.add(*faceVtx0);
					    vtx2.add(*faceVtx1);
					    vtx2.mul(0.25f);
				    }
                    else
                    {
					    vtx2.mul(0.5f);
				    }
			
				    vtx.lerp(vtx,vtx2,smoothness);
			    };

			    eEditMesh::Vertex* newVtx = workMesh->addVertex(vtx);
			    edge->_volatile_attribute0 = newVtx;
            }

            // Add the V vertices (the original vertices of the mesh).
		    for (eU32 i = 0; i < realMesh->getVertexCount(); i++)
            {
                eEditMesh::Vertex *vtx = realMesh->getVertex(i);
                eASSERT(vtx != eNULL);

			    if(vtx->he == eNULL)
                {
				    continue;
                }

			    eEditMesh::HalfEdge *he = vtx->he;
			    eEditMesh::Vertex v;

			    v.position = eVector3(0.0f);
			    v.normal = eVector3(0.0f);
			    v.texCoord = eVector2(0.0f);

                if (!vtx->isBoundary())
                {
                    // Accumulate mid vertices of edges.
                    eU32 edgeCount = 0;

                    do
                    {
					    v.add(*((const eEditMesh::Vertex *)he->edge->_volatile_attribute0));

                        edgeCount++;
                        he = he->twin->next;
                    }
                    while (he != vtx->he);

                    // Accumulate mid vertices of faces.
                    he = vtx->he;

                    do
                    {
                        if(he->face)
                        {
						    v.add(*((const eEditMesh::Vertex *)he->face->_volatile_attribute0));
                        }

                        he = he->twin->next;
                    }
                    while (he != vtx->he);

                    // Do affine combination between vertices.
                    const eF32 ee = 1.0f/((eF32)(edgeCount*edgeCount));
                    const eF32 nm2dn = (eF32)(edgeCount-2)/(eF32)edgeCount;

				    v.mul(ee);

				    eEditMesh::Vertex v2;

				    v2.lerp(*vtx,*vtx,0);
				    v2.mul(nm2dn);
				    v.add(v2);
                }
                else
                {
				    v.lerp(*vtx,*vtx,0.0f);
				    v.mul(6.0f);

                    eEditMesh::HalfEdge *he = vtx->he;

                    do
                    {
                        if (he->edge->isBoundary())
                        {
                            v.add(*((eEditMesh::Vertex *)he->edge->_volatile_attribute0));
                        }

                        he = he->twin->next;
                    }
                    while (he != vtx->he);

				    v.mul(1.0f / 8.0f);
                }

			    v.lerp(*vtx, v, smoothness);
                eEditMesh::Vertex *newVtx = workMesh->addVertex(v);

                he = vtx->he;
                do {
				    he->_volatile_attribute0 = newVtx;
                    he = he->twin->next;
                } while (he != vtx->he);

            }

            // Now add the faces.
            for (eU32 i=0; i<realMesh->getFaceCount(); i++)
            {
                const eEditMesh::Face *face = realMesh->getFace(i);
                eASSERT(face != eNULL);

                eEditMesh::HalfEdge *he = face->he;

                do
                {
                    const eEditMesh::HalfEdge *he0 = he;
                    const eEditMesh::HalfEdge *he1 = he->next;
                    const eEditMesh::HalfEdge *he2 = he->next->next;
                    const eEditMesh::HalfEdge *he3 = he->next->next->next;

                    eEditMesh::Vertex *verts[4] =
                    {
					    (eEditMesh::Vertex *)he->_volatile_attribute0,
					    (eEditMesh::Vertex *)he->edge->_volatile_attribute0,
					    (eEditMesh::Vertex *)face->_volatile_attribute0,
					    (eEditMesh::Vertex *)he->prev->edge->_volatile_attribute0
                    };

                    const eVector2 texCoords[4] =
                    {
                        he0->texCoord,
                        (he0->texCoord+he1->texCoord)*0.5f,
                        (he0->texCoord+he1->texCoord+he2->texCoord+he3->texCoord)*0.25f,
                        (he3->texCoord+he0->texCoord)*0.5f
                    };

                    workMesh->addFace(verts, texCoords, 4)->material = face->material;
                    he = he->next;
                }
                while (he != face->he);
            }
            eSwap(workMesh, realMesh);
            workMesh->clear();
        }

        // Delete working mesh and copy back result.
        if (realMesh != &m_mesh)
        {
            m_mesh.clear();
            m_mesh.merge(*realMesh);
        }

        // Finally update bounding box and normals.
        m_mesh.updateBoundingBox();
        m_mesh.updateNormals();
    }
OP_END(eSubdivideOp);
#endif

// Center (mesh) operator
// ----------------------
// Centers the mesh around the origin of
// the coordinate system (0,0,0).

#if defined(HAVE_OP_MESH_CENTER) || defined(eEDITOR)
OP_DEFINE_MESH(eMeshCenterOp, eMeshCenterOp_ID, "Center", 'e', 1, 1, "-1,Mesh")
    OP_EXEC(eGraphicsApiDx9 *gfx)
    {
        _copyFirstInputMesh();
	    m_mesh.centerMesh();
    }
OP_END(eMeshCenterOp);
#endif

// Cylinder (mesh) operator
// ---------------------
// Creates a cylinder.

#if defined(HAVE_OP_MESH_CYLINDER) || defined(eEDITOR)
OP_DEFINE_MESH(eCylinderOp, eCylinderOp_ID, "Cylinder", 'c', 0, 0, "")
    OP_INIT()
    {
        eU32 flags = 0;
        eSetBit(flags, 0);
        eSetBit(flags, 1);

        eOP_PARAM_ADD_FLOAT("Radius", eALMOST_ZERO, eF32_MAX, 0.5f);
        eOP_PARAM_ADD_FLOAT("Height", eALMOST_ZERO, eF32_MAX, 1.0f);
        eOP_PARAM_ADD_INT("Segments", 1, 256, 4);
        eOP_PARAM_ADD_INT("Edges", 3, 256, 8);
        eOP_PARAM_ADD_FLOAT("Angle", 0.0f, 1.0f, 1.0f);
        eOP_PARAM_ADD_FLOAT("Constriction", eALMOST_ZERO, eF32_MAX, 1.0f);
        eOP_PARAM_ADD_FLAGS("Closeness", "Top closed|Bottom closed", flags);
        eOP_PARAM_ADD_BOOL("Generate cut edge", eTRUE);
        eOP_PARAM_ADD_LINK("Material", "Misc : Material");
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eF32 radius, eF32 height, eU32 segments, eU32 edges,
            eF32 angle, eF32 constriction, eU8 closeness, eBool genCutEdge, eIMaterialOp *matOp)
    {
        const eBool topClosed = eGetBit(closeness, 0);
        const eBool bottomClosed = eGetBit(closeness, 1);

        const eF32 rad = angle*eTWOPI;
        const eF32 radStep = -(radius-radius*constriction)/(eF32)segments;
        const eF32 phiStep = rad/(eF32)edges;
        const eF32 heightStep = height/(eF32)segments;

        eF32 tlen = 1;
        eF32 uvinlen = 0;
        eF32 uvoulen = 1;

        if (angle < 1.0f)
        {
            tlen = 2*radius + radius*rad;
            uvinlen = radius/tlen;
            uvoulen = (radius*rad)/tlen + ((radius*rad)/tlen)/(edges-1);
        }

        m_mesh.reserveSpace((segments+1)*edges+2, segments*edges+(2*edges));

        // Generate outer vertices.
        eF32 curRadius = radius;
        eF32 curHeight = -0.5f*height;

        for (eU32 seg=0; seg<segments+1; seg++)
        {
            eF32 phi = 0.0f;

            for (eU32 edge=0; edge<edges; edge++)
            {
                m_mesh.addVertex(eVector3(eCos(phi)*curRadius, curHeight, eSin(phi)*curRadius));
                phi += phiStep;
            }

            curRadius += radStep;
            curHeight += heightStep;
        }

        // Generate top vertex.
        eU32 topVertex = 0;

        if (topClosed || (genCutEdge && angle < 1.0f))
        {
            m_mesh.addVertex(eVector3(0.0f, height*0.5f, 0.0f), eVector2(0.5f, 0.5f));
            topVertex = m_mesh.getVertexCount()-1;
        }

        // Generate the cut edge vertices.
        if (genCutEdge && angle < 1.0f)
        {
            for (eU32 i=segments-1; i>=1; i--)
            {
                m_mesh.addVertex(eVector3(0.0f, (eF32)i*heightStep-height*0.5f, 0.0f));
            }
        }

        // Generate bottom vertex.
        eU32 bottomVertex = 0;

        if (bottomClosed || (genCutEdge && angle < 1.0f))
        {
            m_mesh.addVertex(eVector3(0.0f, -height*0.5f, 0.0f), eVector2(0.5f, 0.5f));
            bottomVertex = m_mesh.getVertexCount()-1;
        }

        // Create outer faces.
        for (eU32 i=0; i<segments; i++)
        {
            for (eU32 j=0; j<edges-(angle < 1.0f ? 1 : 0); j++)
            {
                const eU32 row0 = i*edges;
                const eU32 row1 = (i+1)*edges;
                const eU32 col0 = j;
                const eU32 col1 = (j+1)%edges;

                const eU32 indices[4] =
                {
                    row0+col0,
                    row1+col0,
                    row1+col1,
                    row0+col1,
                };

                m_mesh.addFace(indices, eNULL, 4);
            }
        }

        // Create top and bottom faces.
        const eU32 topVertexRowOffset = segments*edges;

        for (eU32 j=0; j<edges-(angle < 1.0f ? 1 : 0); j++)
        {
            if (topClosed)
            {
                const eU32 indicesTop[3] =
                {
                    topVertex, topVertexRowOffset+(j+1)%edges, topVertexRowOffset+j
                };

			    m_mesh.addFace(indicesTop, eNULL, 3);
            }

            if (bottomClosed)
            {
                const eU32 indicesBottom[3] =
                {
                    bottomVertex, j, (j+1)%edges,
                };

                m_mesh.addFace(indicesBottom, eNULL, 3);
            }
        }

        // Generate the cut edge faces if requested.
        if (genCutEdge && angle < 1.0f)
        {
            for (eU32 i=0; i<segments; i++)
            {
                const eU32 leftCapIndices[4] =
                {
                    bottomVertex-i-1,
                    edges*(i+1),
                    edges*i,
                    bottomVertex-i,
                };

                m_mesh.addFace(leftCapIndices, eNULL, 4);

                const eU32 rightCapIndices[4] =
                {
                    bottomVertex-i,
                    edges*i+edges-1,
                    edges*(i+1)+edges-1,
                    bottomVertex-i-1,
                };

                m_mesh.addFace(rightCapIndices, eNULL, 4);
            }
        }

        m_mesh.updateNormals();
	    m_mesh.mapUVs(eEditMesh::CYLINDER, eVector3::ZAXIS, eVector3::YAXIS, eVector3(), eVector2(1.0f, 6.0f/height));

        // Set material if specified.
        if (matOp)
        {
            m_mesh.setMaterial(&matOp->getResult().material);
        }
    }
OP_END(eCylinderOp);
#endif

// Extrude (mesh) operator
// -----------------------
// Extrudes faces of a mesh.

#if defined(HAVE_OP_MESH_EXTRUDE) || defined(eEDITOR)
OP_DEFINE_MESH(eExtrudeOp, eExtrudeOp_ID, "Extrude", 'x', 1, 1, "-1,Mesh")
    OP_INIT()
    {
        eOP_PARAM_ADD_FLOAT("Length", eF32_MIN, eF32_MAX, 1.0f);
        eOP_PARAM_ADD_INT("Segments", 1, 64, 1);
        eOP_PARAM_ADD_ENUM("Direction", "Face normal|Vertex normal", 0);
        eOP_PARAM_ADD_FXYZ("Scale", eF32_MIN, eF32_MAX, 1.0f, 1.0f, 1.0f);
        eOP_PARAM_ADD_FXYZ("Rotate", eF32_MIN, eF32_MAX, 0.0f, 0.0f, 0.0f);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eF32 length, eU32 segments, eInt dir, const eVector3 &scale, const eVector3 &rot)
    {
        eMatrix4x4 trans;
        trans.transformation(rot*eTWOPI, eVector3(), scale);

        _copyFirstInputMesh();

        for (eU32 i=0; i<segments; i++)
        {
            const eU32 oldFaceCount = m_mesh.getFaceCount();

            // Find contour of selection (all edges of all
            // selected faces, that have one neighbouring
            // face selected and one not selected).
            eEditMesh::HalfEdgePtrArray contour;

            for (eU32 j=0; j<m_mesh.getFaceCount(); j++)
            {
                const eEditMesh::Face *face = m_mesh.getFace(j);
                eASSERT(face != eNULL);

                if (face->selected)
                {
                    eEditMesh::HalfEdge *he = face->he;

                    do
                    {
                        if (he->face->selected && (he->twin->face == eNULL || !he->twin->face->selected))
                        {
                            contour.append(he);
                        }

                        he = he->next;
                    }
                    while (he != face->he);
                }
            }

            // Add all cap vertices and cap faces.
            eHashMap<const eEditMesh::HalfEdge *, eEditMesh::Vertex *> he2v;
            eEditMesh::VertexPtrArray va;

            for (eU32 j=0; j<oldFaceCount; j++)
            {
                eEditMesh::Face *face = m_mesh.getFace(j);
                eASSERT(face != eNULL);

                if (face->selected)
                {
                    eEditMesh::VertexPtrArray verts;
                    eVector2Array texCoords;

                    eEditMesh::HalfEdge *he = face->he;

                    do
                    {
                        // Is vertex existing multiple times in contour?
                        eEditMesh::Vertex *vtx = he->origin;
                        eASSERT(vtx != eNULL);

                        eU32 count = 0;

                        for (eU32 k=0; k<contour.size(); k++)
                        {
                            if (contour[k]->origin == vtx)
                            {
                                count++;
                            }
                        }

                        // If vertex hasn't been added yet or exists more
                        // than one time in contour then add it.
                        if (va.exists(vtx) == -1 || count > 1)
                        {
                            const eVector3 normal = (dir == 0 ? face->normal : vtx->normal);
                            const eVector3 newPos = vtx->position*trans+normal*trans*length;

                            eEditMesh::Vertex *newVtx = m_mesh.addVertex(newPos, vtx->texCoord, vtx->normal);
                            eASSERT(newVtx != eNULL);

                            // For vertices existing multiple times a direct
                            // half-edge -> vertex mapping must be possible.
                            if (count > 1)
                            {
                                he2v.insert(he, newVtx);

                                eEditMesh::HalfEdge *ihe = vtx->he;

                                do
                                {
                                    ihe = ihe->twin->next;
                                }
                                while (ihe != vtx->he);
                            }
                            else
                            {
                                // For all other vertices all incident
                                // half-edges have to be mapped to this vertex.
                                eEditMesh::HalfEdge *ihe = vtx->he;

                                do
                                {
                                    he2v.insert(ihe, newVtx);
                                    ihe = ihe->twin->next;
                                }
                                while (ihe != vtx->he);
                            }

                            va.append(vtx);
                            verts.append(newVtx);
                        }
                        else
                        {
                            eEditMesh::Vertex *newVtx = he2v[he];
                            eASSERT(newVtx != eNULL);
                            verts.append(newVtx);
                        }

                        texCoords.append(he->texCoord);
                        he = he->next;
                    }
                    while (he != face->he);

                    eEditMesh::Face *newFace = m_mesh.addFace(&verts[0], &texCoords[0], verts.size());
                    eASSERT(newFace != eNULL);
                    newFace->selected = eTRUE;
                    newFace->material = face->material;
                }
            }

            // Save some topology information that get
            // destroyed in the following.
            eArray<eBool> boundaryFlags(oldFaceCount);
            eArray<const eMaterial *> faceMats;

            for (eInt j=(eInt)oldFaceCount-1; j>=0; j--)
            {
                boundaryFlags[j] = m_mesh.getFace(j)->isBoundary();
            }

            for (eU32 j=0; j<contour.size(); j++)
            {
                faceMats.append(contour[j]->face->material);
            }

            // Remove extruded faces.
            for (eInt j=(eInt)oldFaceCount-1; j>=0; j--)
            {
                eEditMesh::Face *face = m_mesh.getFace(j);
                eASSERT(face != eNULL);

                if (face->selected)
                {
                    m_mesh.removeFace(j, !boundaryFlags[j]);
                }
            }

            // Add faces on side of extrusion.
            eArray<eEditMesh::HalfEdge> contourCopy;
        
            for (eU32 j=0; j<contour.size(); j++)
            {
                contourCopy.append(*contour[j]);
            }

            for (eU32 j=0; j<contour.size(); j++)
            {
                const eEditMesh::HalfEdge *he = contour[j];
                eASSERT(he != eNULL);

                eEditMesh::Vertex *orgVtx0 = he->origin;
                eEditMesh::Vertex *orgVtx1 = contourCopy[j].next->origin;
                eEditMesh::Vertex *newVtx0 = he2v[he];
                eEditMesh::Vertex *newVtx1 = he2v[contourCopy[j].next];

                eEditMesh::Vertex * verts[4] =
                {
                    orgVtx0, orgVtx1, newVtx1, newVtx0
                };

                const eVector2 texCoords[4] =
                {
                    he->texCoord,
                    he->twin->texCoord,
                    // Offset texture coordiantes, because
                    // tangents and binormals for lighting
                    // are calculated using texture coordinate
                    // differences.
                    he->twin->texCoord+0.001f,
                    he->texCoord+0.001f
                };

                m_mesh.addFace(verts, texCoords, 4)->material = faceMats[j];
            }

            // Remove edges hanging around freely.
            for (eInt j=(eInt)m_mesh.getEdgeCount()-1; j>=0; j--)
            {
                const eEditMesh::Edge *edge = m_mesh.getEdge(j);
                eASSERT(edge != eNULL);

                if (edge->he0->face == eNULL && edge->he1->face == eNULL)
                {
                    m_mesh.removeEdge(j);
                }
            }

            m_mesh.updateNormals();
        }

        m_mesh.updateBoundingBox();
    }
OP_END(eExtrudeOp);
#endif

// BC logo (mesh) operator
// -----------------------
// Generates a two-dimensional Brain Control logo.

#if defined(HAVE_OP_MESH_BC_LOGO) || defined(eEDITOR)
OP_DEFINE_MESH(eBcLogoOp, eBcLogoOp_ID, "BC Logo", ' ', 0, 0, "")
    OP_INIT()
    {
        eOP_PARAM_ADD_LINK("Material", "Misc : Material");
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eIMaterialOp *matOp)
    {
        const eVector3 vertices[] =
        {
            eVector3(-0.250824f, 0.0f,  0.073242f),
            eVector3(-0.409866f, 0.0f,  0.134430f),
            eVector3(-0.350372f, 0.0f, -0.026154f),
            eVector3( 0.091481f, 0.0f, -0.087286f),
            eVector3( 0.165737f, 0.0f, -0.013035f),
            eVector3( 0.140244f, 0.0f,  0.012466f),
            eVector3( 0.093686f, 0.0f, -0.034098f),
            eVector3( 0.053761f, 0.0f,  0.005840f),
            eVector3( 0.139267f, 0.0f,  0.091324f),
            eVector3( 0.244686f, 0.0f, -0.014107f),
            eVector3( 0.092587f, 0.0f, -0.166208f),
            eVector3(-0.076981f, 0.0f,  0.003366f),
            eVector3( 0.137962f, 0.0f,  0.218330f),
            eVector3( 0.369087f, 0.0f, -0.012810f),
            eVector3( 0.093437f, 0.0f, -0.288425f),
            eVector3(-0.201508f, 0.0f,  0.006516f),
            eVector3( 0.139473f, 0.0f,  0.347427f),
            eVector3( 0.494225f, 0.0f, -0.007294f),
            eVector3( 0.094559f, 0.0f, -0.407234f),
            eVector3(-0.280838f, 0.0f, -0.031807f),
            eVector3(-0.374870f, 0.0f, -0.125992f),
            eVector3(-0.489258f, 0.0f,  0.213760f),
            eVector3(-0.148537f, 0.0f,  0.100511f),
            eVector3(-0.242554f, 0.0f,  0.006470f),
            eVector3( 0.095924f, 0.0f, -0.331944f),
            eVector3( 0.419647f, 0.0f, -0.008087f),
            eVector3( 0.142059f, 0.0f,  0.269440f),
            eVector3(-0.122963f, 0.0f,  0.004423f),
            eVector3( 0.093929f, 0.0f, -0.212473f),
            eVector3( 0.293671f, 0.0f, -0.012695f),
            eVector3( 0.138882f, 0.0f,  0.142113f),
            eVector3( 0.000492f, 0.0f,  0.003702f),
        };

        const eU32 indices[] =
        {
            22,  2, 21,
             2,  3, 21,
            22, 23,  2,
             2, 23,  1,
            21,  3, 20,
            23, 24,  1,
             3,  1, 20,
             1, 24, 20,
             7,  8,  6,
             8,  9,  6,
             9, 10,  6,
             6, 10,  5,
            10, 11,  5,
             5, 11,  4,
            11, 12,  4, 
             4, 12, 32,
            12, 13, 32,
            32, 13, 31,
            13, 14, 31,
            31, 14, 30,
            14, 15, 30,
            30, 15, 29,
            15, 16, 29,
            29, 16, 28,
            16, 17, 28,
            28, 17, 27,
            17, 18, 27,
            27, 18, 26,
            18, 19, 26,
            26, 19, 25,
            19, 20, 25,
            20, 24, 25,
        };

        const eVector2 texCoords[] =
        {
            0.307515f, 0.433466f,
            0.369618f, 0.433466f,
            0.365292f, 0.572047f,
            0.199846f, 0.554811f,
            0.211639f, 0.291257f,
            0.474845f, 0.322239f,
            0.460749f, 0.687600f,
            0.099505f, 0.645803f,
            0.116740f, 0.169842f,
            0.577559f, 0.237387f,
            0.557812f, 0.801828f,
            0.000000f, 0.728002f,
            0.022050f, 0.061754f,
            0.632056f, 0.146466f,
            0.662271f, 0.000000f,
            1.000000f, 0.242132f,
            0.609518f, 0.367595f,
            0.619008f, 0.205847f,
            0.082199f, 0.144372f,
            0.055614f, 0.687461f,
            0.494174f, 0.724792f,
            0.510432f, 0.290140f,
            0.171028f, 0.251902f,
            0.164259f, 0.586910f,
            0.400809f, 0.618310f,
            0.403182f, 0.376038f,
            0.272905f, 0.362152f,
            0.270184f, 0.499407f,
            0.304026f, 0.501151f,
            0.676764f, 0.286006f,
            0.704420f, 0.093293f,
            0.881475f, 0.220232f,
        };

        for (eU32 i=0; i<sizeof(vertices)/sizeof(eVector3); i++)
        {
            m_mesh.addVertex(vertices[i], texCoords[i]);
        }

        for (eU32 i=0; i<sizeof(indices)/sizeof(eU32); i+=3)
        {
            m_mesh.addTriangle(indices[i]-1, indices[i+1]-1, indices[i+2]-1);
        }

        m_mesh.updateNormals();

        // Set material if specified.
        if (matOp)
        {
            m_mesh.setMaterial(&matOp->getResult().material);
        }
    }
OP_END(eBcLogoOp);
#endif

// Text3D (mesh) operator
// ----------------------
// Creates a 3D text.

#if defined(HAVE_OP_MESH_TEXT_3D) || defined(eEDITOR)
OP_DEFINE_MESH(eText3dOp, eText3dOp_ID, "Text 3D", ' ', 0, 0, "")
    OP_INIT()
    {
        eOP_PARAM_ADD_STRING("Font", "");
        eOP_PARAM_ADD_TEXT("Text", "Brain Control");
        eOP_PARAM_ADD_FLOAT("Size", 0.0f, eF32_MAX, 1.0f);
        eOP_PARAM_ADD_FLOAT("Depth", 0.0f, eF32_MAX, 0.25f);
        eOP_PARAM_ADD_BOOL("Inverted", eFALSE);
        eOP_PARAM_ADD_LINK("Material", "Misc : Material");
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, const eChar *fontName, const eChar *text, eF32 size, eF32 depth, eBool inverted, eIMaterialOp *matOp)
    {
        const eF32 scale = size*0.0025f;

        HDC memDc = CreateCompatibleDC(NULL);
        eASSERT(memDc != eNULL);
        HDC screenDc = GetDC(NULL);
        eASSERT(screenDc != eNULL);
        HBITMAP memBmp = CreateCompatibleBitmap(screenDc, 300, 300);
        eASSERT(memBmp != eNULL);
        SelectObject(memDc, memBmp);
        HFONT font = CreateFont(500, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, fontName);
        SelectObject(memDc, font);
        SetBkMode(memDc, (inverted ? OPAQUE : TRANSPARENT));
        BeginPath(memDc);
        TextOut(memDc, 0, 0, text, eStrLength(text));
        EndPath(memDc);
        FlattenPath(memDc);
    
        const eU32 pointCount = GetPath(memDc, NULL, NULL, 0);

        if (pointCount > 0)
        {
            eArray<POINT> points(pointCount);
            eByteArray types(pointCount);

            GetPath(memDc, &points[0], &types[0], pointCount);

            eVector3Array conture;

		    // Triangulate generated contures.
            eTriangulator trg;

            for (eU32 i=0; i<pointCount; i++)
            {
                if (types[i] == PT_MOVETO)
                {
				    trg.addContour(conture);
				    conture.clear();
                }

                conture.append(eVector3((eF32)points[i].x*scale, -(eF32)points[i].y*scale, -depth*0.5f));
            }
		    trg.addContour(conture);
            trg.triangulate();

            // Add front and back triangles to mesh.
            const eVector3Array &verts = trg.getVertices();

            for (eU32 i=0; i<verts.size(); i++)
            {
                eEditMesh::Vertex *vf = m_mesh.addVertex(verts[i]);
                eEditMesh::Vertex *vb = m_mesh.addVertex(verts[i]);
			    vb->position.z = -vb->position.z;
			    vf->_volatile_attribute1 = vb;
            }

            const eArray<eU32> &indices = trg.getIndices();

            for (eU32 i=0; i<indices.size(); i+=3)
            {
                m_mesh.addTriangle(indices[i]*2, indices[i+1]*2, indices[i+2]*2);
                m_mesh.addTriangle(indices[i+2]*2+1, indices[i+1]*2+1, indices[i]*2+1);
            }

            // Add side faces to mesh.
            for (eU32 i=0; i<m_mesh.getFaceCount(); i++)
            {
			    eEditMesh::Face* face = m_mesh.getFace(i);
                eEditMesh::HalfEdge *he = face->he;

                do
                {
				    const eEditMesh::HalfEdge *twin = he->twin;

                    if (twin->isBoundary())
                    {
                        m_mesh.addQuad((eEditMesh::Vertex*)twin->destination()->_volatile_attribute1, 
						               (eEditMesh::Vertex*)twin->origin->_volatile_attribute1,
								       twin->origin, 
								       twin->destination());

                    }

                    he = he->next;
                }
                while (he != face->he);
            }
        }

        // Center text mesh.
	    m_mesh.centerMesh();
        m_mesh.updateNormals();
        m_mesh.triangulate();
	    m_mesh.mapUVs(eEditMesh::CUBE, eVector3::ZAXIS, eVector3::YAXIS, eVector3(), eVector2(5));

        // delete the GDI objects
        ReleaseDC(NULL, screenDc);
        DeleteDC(memDc);
        DeleteObject(memBmp);
        DeleteObject(font);

        // Set material if specified.
        if (matOp)
        {
            m_mesh.setMaterial(&matOp->getResult().material);
        }
    }
OP_END(eText3dOp);
#endif

// Wave transform (mesh) operator
// ------------------------------
// Transforms vertices with sine waves.

#if defined(HAVE_OP_MESH_WAVE) || defined(eEDITOR)
OP_DEFINE_MESH(eWaveOp, eWaveOp_ID, "Wave", 'w', 1, 1, "-1,Mesh")
    OP_INIT()
    {
        eU32 flags = 0;
        eSetBit(flags, 0);
        eSetBit(flags, 1);
        eSetBit(flags, 2);

        eOP_PARAM_ADD_ENUM("Target", "Transform|Rotate|Scale", 0);
        eOP_PARAM_ADD_FLOAT("Speed", 0.1f, eF32_MAX, 1.0f);
        eOP_PARAM_ADD_FLOAT("Amount", -eF32_MAX, eF32_MAX, 1.0f);
        eOP_PARAM_ADD_FLOAT("Radius", 0.1f, eF32_MAX, 100.0f);
        eOP_PARAM_ADD_FLOAT("Time", 0.0f, eF32_MAX, 0.0f);
        eOP_PARAM_ADD_FLAGS("Affect", "X-axis|Y-axis| Z-axis", flags);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eInt target, eF32 speed, eF32 amount, eF32 radius, eF32 time, eU8 affect)
    {
        _copyFirstInputMesh();

        const eBool affectX = eGetBit(affect, 0);
        const eBool affectY = eGetBit(affect, 1);
        const eBool affectZ = eGetBit(affect, 2);
        const eF32 sqrRadius = radius*radius;

        for (eU32 i=0; i< m_mesh.getVertexCount(); i++)
        {
            eEditMesh::Vertex *vtx = m_mesh.getVertex(i);
            eASSERT(vtx != eNULL);

            const eF32 dist = vtx->position.sqrLength()/sqrRadius;
            const eF32 sine = eSin(time*speed+dist)*amount;

            switch (target)
            {
                case 0:  // Translate
                {
                    vtx->position += eVector3(affectX ? sine : 0.0f, affectY ? sine : 0.0f, affectZ ? sine : 0.0f);
                    break;
                }


                case 1:  // Rotate
                {
                    const eF32 val = eDegToRad(sine);
                    const eVector3 rot(affectX ? val : 0.0f, affectY ? val : 0.0f, affectZ ? val : 0.0f);

                    vtx->position.rotate(rot);
                    break;
                }


                case 2: // Scale
                {
                    const eF32 val = sine+(1.0f-amount);
                    const eVector3 scale(affectX ? val : 1.0f, affectY ? val : 1.0f, affectZ ? val : 1.0f);

                    vtx->position.scale(scale);
                    break;
                }
            }
        }

        m_mesh.updateNormals();
    }
OP_END(eWaveOp);
#endif

// Ribbon (mesh) operator
// ----------------------
// Generates a ribbon mesh.

#if defined(HAVE_OP_MESH_RIBBON) || defined(eEDITOR)
OP_DEFINE_MESH(eRibbonOp, eRibbonOp_ID, "Ribbon", ' ', 0, 0, "")
    OP_INIT()
    {
        eOP_PARAM_ADD_LINK("Position path", "Path");
        eOP_PARAM_ADD_FXYZ("Scale", eF32_MIN, eF32_MAX, 1.0f, 1.0f, 1.0f);
        eOP_PARAM_ADD_FLOAT("Length", 0.0f, 1.0f, 1.0f);
        eOP_PARAM_ADD_INT("Segments", 1, 128, 16);
        eOP_PARAM_ADD_INT("Edges", 3, 32, 8);
        eOP_PARAM_ADD_LINK("Material", "Misc : Material");
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, const eIPathOp *posOp, const eVector3 &scale, eF32 length, eU32 totalSegs, eU32 edges, eIMaterialOp *matOp)
    {
        // Get parameters and check if ribbon can be generated.
        if (posOp == eNULL || eIsFloatZero(length))
        {
            return;
        }

        const ePath &path = posOp->getResult().path;
        const eF32 endTime = path.getEndTime()*length;
        const eU32 segments = eFtoL((eF32)totalSegs*length);
        const eF32 timeStep = path.getRunningTime()/(eF32)totalSegs;

        // Generate vertices of ribbon.
        m_mesh.reserveSpace((segments+1)*edges, segments*edges);

        eVector3 dir;

        for (eU32 i=0; i<=segments; i++)
        {
            // Sample path at two different times.
            const eF32 time0 = path.getStartTime()+(eF32)i*timeStep;
            const eF32 time1 = eMin(time0+timeStep, endTime);

            const eVector3 pos0 = path.process(time1, this).toVec3();
            const eVector3 pos1 = path.process(time0, this).toVec3();

            // Check if length of direction is zero and if it is
            // used old direction (can happen in the last segment).
            const eVector3 tempDir = (pos1-pos0).normalized();

            if (!eIsFloatZero(tempDir.sqrLength()))
            {
                dir = tempDir;
            }

            // Create two vectors perpendicular to direction.
            const eVector3 axis0 = dir;
            const eVector3 axis1 = eVector3(axis0.z, axis0.y*axis0.z/(axis0.x-1.0f),  1.0f+(axis0.z*axis0.z)/(axis0.x-1.0f));
            const eVector3 axis2 = axis0^axis1;

            // Add a ring of vertices.
            const eF32 step = eTWOPI/(eF32)edges;

            for (eU32 j=0; j<edges; j++)
            {
                eVector3 pos = axis2*eQuat(dir, (eF32)j*step)+pos0;

                pos.scale(scale);
                m_mesh.addVertex(pos, eVector2((eF32)i/(eF32)(totalSegs+1), (eF32)j/(eF32)edges));
            }
        }

        // Generate faces of ribbon.
        for (eU32 i=0; i<segments; i++)
        {
            for (eU32 j=0; j<edges; j++)
            {
                const eU32 indices[4] =
                {    
                    i*edges+j,
                    i*edges+(j+1)%edges,
                    (i+1)*edges+(j+1)%edges,
                    (i+1)*edges+j,
                };

                const eVector2 texCoords[4] =
                {
                    eVector2((eF32)i/(eF32)totalSegs,     (eF32)j/(eF32)edges),
                    eVector2((eF32)i/(eF32)totalSegs,     (eF32)(j+1)/(eF32)edges),
                    eVector2((eF32)(i+1)/(eF32)totalSegs, (eF32)(j+1)/(eF32)edges),
                    eVector2((eF32)(i+1)/(eF32)totalSegs, (eF32)j/(eF32)edges),
                };

                m_mesh.addFace(indices, texCoords, 4);
            }
        }      

        m_mesh.updateNormals();

        // Set material if specified.
        if (matOp)
        {
            m_mesh.setMaterial(&matOp->getResult().material);
        }
    }
OP_END(eRibbonOp);
#endif

// Color (mesh) operator
// ---------------------
// Sets the vertex colors of a mesh to a given color.

#if defined(HAVE_OP_MESH_COLOR) || defined(eEDITOR)
OP_DEFINE_MESH(eMeshColorOp, eMeshColorOp_ID, "Color", ' ', 1, 1, "-1,Mesh")
    OP_INIT()
    {
        eOP_PARAM_ADD_RGB("Color", 1.0f, 1.0f, 1.0f);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, const eFloatColor &color)
    {
        _copyFirstInputMesh();

        for (eU32 i=0; i<m_mesh.getVertexCount(); i++)
        {
            m_mesh.getVertex(i)->color = color;
        }
    }
OP_END(eMeshColorOp);
#endif

// Multiply (mesh) operator
// ------------------------
// Duplicates the mesh multiple times.

#if defined(HAVE_OP_MESH_MULTIPLY) || defined(eEDITOR)
OP_DEFINE_MESH(eMeshMultiplyOp, eMeshMultiplyOp_ID, "Multiply", ' ', 1, 1, "-1,Mesh")
    OP_INIT()
    {
        eOP_PARAM_ADD_FXYZ("Translate", eF32_MIN, eF32_MAX, 0, 0, 0);
        eOP_PARAM_ADD_FXYZ("Rotate", eF32_MIN, eF32_MAX, 0, 0, 0);
        eOP_PARAM_ADD_FXYZ("Scale", eF32_MIN, eF32_MAX, 1, 1, 1);
        eOP_PARAM_ADD_INT("Count", 1, 255, 2);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, const eVector3 &trans, const eVector3 &rot, const eVector3 &scale, eU32 count)
    {
        eEditMesh em = ((eIMeshOp *)getInputOperator(0))->getResult().mesh;

        eMatrix4x4 mtxPos;
        mtxPos.transformation(rot*eTWOPI, trans, scale);   

        m_mesh.merge(em);

        for (eU32 i=0; i<count; i++)
        {
            eMatrix4x4 mtxNrm = mtxPos;
            mtxNrm.invert();
            mtxNrm.transpose();

            for (eU32 j=0; j<em.getVertexCount(); j++)
            {
                eEditMesh::Vertex *vtx = em.getVertex(j);
                eASSERT(vtx != eNULL);

                vtx->position *= mtxPos;
                vtx->normal *= mtxNrm;
            }

            m_mesh.merge(em);
        }

        // Update face normals.
        for (eU32 i=0; i<m_mesh.getFaceCount(); i++)
        {
            m_mesh.getFace(i)->updateNormal();
        }
    }
OP_END(eMeshMultiplyOp);
#endif

// Bend (mesh) operator
// --------------------
// Bend the input mesh.

#if defined(HAVE_OP_MESH_BEND) || defined(eEDITOR)
OP_DEFINE_MESH(eBendOp, eBendOp_ID, "Bend", 'b', 1, 1, "-1,Mesh")
    OP_INIT()
    {
        eOP_PARAM_ADD_LABEL("General", "General");
        eOP_PARAM_ADD_FXYZ("Up", eF32_MIN, eF32_MAX, 0.0f, 0.0f, 1.0f);
	    eOP_PARAM_ADD_BOOL("Keep Alignment", false);
        eOP_PARAM_ADD_LABEL("Bend Region", "Bend Region");
        eOP_PARAM_ADD_FXYZ("Axis0", eF32_MIN, eF32_MAX, 0.0f, 0.0f, 0.0f);
        eOP_PARAM_ADD_FXYZ("Axis1", eF32_MIN, eF32_MAX, 0.0f, 1.0f, 0.0f);
        eOP_PARAM_ADD_LABEL("Control Points", "Control Points");
        eOP_PARAM_ADD_FXYZ("Control0", eF32_MIN, eF32_MAX, 0.0f, 0.0f, 0.0f);
        eOP_PARAM_ADD_FXYZ("Control1", eF32_MIN, eF32_MAX, 0.0f, 0.0f, 0.333f);
        eOP_PARAM_ADD_FXYZ("Control2", eF32_MIN, eF32_MAX, 0.0f, 0.0f, 0.666f);
        eOP_PARAM_ADD_FXYZ("Control3", eF32_MIN, eF32_MAX, 0.0f, 0.0f, 1.0f);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, const eVector3 &upVal, eBool keepAlignment, const eVector3 &axis0, const eVector3 &axis1,
            const eVector3 &control0, const eVector3 &control1, const eVector3 &control2, const eVector3 &control3)
    {
        const eVector3 up = upVal.normalized();

        _copyFirstInputMesh();

	    eVector3 axis = axis1 - axis0;
	    const eF32 axisLen = axis.length();
        eMatrix4x4 mtx;
	    mtx.lookAt(eVector3(0,0,0), axis.normalized(), up);
	    const eMatrix4x4 mtxInv = mtx.inverse();

        eVector3 bezPos;
	    eVector3 bezTangent;

        for (eU32 i=0; i<m_mesh.getVertexCount(); i++)
        {
            eVector3 pos = m_mesh.getVertex(i)->position-axis0;
		    eVector3 posRelAxis = mtxInv*pos;
		    eF32 t = posRelAxis.z/axisLen;
		    eF32 tclamp = eClamp(0.0f, t, 1.0f);

		    eVector3::cubicBezier(tclamp, control0, control1, control2, control3, bezPos, bezTangent);
		    bezPos.z *= axisLen;
		    bezPos = mtx * bezPos+axis0;
		    bezTangent.z *= axisLen;
		    bezTangent = mtx * bezTangent.normalized();

		    eMatrix4x4 bezMtx;
		    bezMtx.lookAt(eVector3(0,0,0), bezTangent, up);
		    posRelAxis.z = (t-tclamp)*axisLen;

		    if (keepAlignment)
            {
			    m_mesh.getVertex(i)->position = mtx*posRelAxis+bezPos;
		    }
            else
            {
			    m_mesh.getVertex(i)->position = bezMtx*posRelAxis+bezPos;
		    }
	    }
    }

    OP_INTERACT(eGraphicsApiDx9 *gfx, eSceneData &sg)
    {
        /*
        const eVector3 up = eVector3(getParameter(1).getValue().fxyz).normalized();
		const eVector3 axis0 = getParameter(4).getValue().fxyz;
		const eVector3 axis1 = getParameter(5).getValue().fxyz;

		const eVector3 control[] =
        {
            getParameter(7).getValue().fxyz,
			getParameter(8).getValue().fxyz,
            getParameter(9).getValue().fxyz,
            getParameter(10).getValue().fxyz
        };

		eMatrix4x4 mtx;
		mtx.lookAt(eVector3(0,0,0), (axis1 - axis0).normalized(), up);
		const eF32 axisLen = (axis1 - axis0).length();

		eU32 segs = 20;
        eGeometry geo(gfx, 8 + 4*3*2 + segs*2, 8 + 4*3*2 + segs*2, 4 + 4*3 + segs, eVTXTYPE_WIRE, eGeometry::TYPE_DYNAMIC_INDEXED, ePRIMTYPE_LINELIST);
        eWireVertex *verts = eNULL;
        eU32 *inds = eNULL;

		eF32 size = 0.5f;
		eF32 size2 = 0.1f;

        geo.startFilling((ePtr *)&verts, &inds);
        {
			eInt vcnt = 0;
			eInt icnt = 0;

			for(eU32 d = 0; d < 2; d++)
            {
				eVector3 pos = (d == 0) ? axis0 : axis1;
				verts[vcnt++].set(pos + mtx.getVector(0) * size, eColor::YELLOW);
				verts[vcnt++].set(pos - mtx.getVector(0) * size, eColor::YELLOW);
				verts[vcnt++].set(pos + mtx.getVector(1) * size, eColor::YELLOW);
				verts[vcnt++].set(pos - mtx.getVector(1) * size, eColor::YELLOW);
				inds[icnt] = icnt++;
				inds[icnt] = icnt++;
				inds[icnt] = icnt++;
				inds[icnt] = icnt++;
			}

			for(eU32 c = 0; c < 4; c++)
            {
				eVector3 pos = control[c];
				pos.z *= axisLen;
				pos = mtx * pos + axis0;

				for(eU32 d = 0; d < 3; d++)
                {
					verts[vcnt++].set(pos + mtx.getVector(d) * size2, eColor::CYAN);
					verts[vcnt++].set(pos - mtx.getVector(d) * size2, eColor::CYAN);
					inds[icnt] = icnt++;
					inds[icnt] = icnt++;
				}
			}

			for(eU32 s = 0; s < segs; s++)
            {
				eF32 t0 = (eF32)s / (eF32)segs;
				eF32 t1 = (eF32)(s+1) / (eF32)segs;
				eVector3 tangent0;
				eVector3 tangent1;
				eVector3 pos0;
				eVector3 pos1;
				eVector3::cubicBezier(t0, control[0], control[1], control[2], control[3], pos0, tangent0);
				eVector3::cubicBezier(t1, control[0], control[1], control[2], control[3], pos1, tangent1);
				pos0.z *= axisLen;
				pos1.z *= axisLen;
				pos0 = mtx * pos0 + axis0;
				pos1 = mtx * pos1 + axis0;
				verts[vcnt++].set(pos0, eColor::PINK);
				verts[vcnt++].set(pos1, eColor::PINK);
				inds[icnt] = icnt++;
				inds[icnt] = icnt++;
			}
        }

        geo.stopFilling();
        geo.render();
        */
    }
OP_END(eBendOp);
#endif

// Duplicate circular (mesh) operator
// ----------------------------------
// Duplicates meshes in a circular arrangement.

#if defined(HAVE_OP_MESH_DUP__CIRCULAR) || defined(eEDITOR)
OP_DEFINE_MESH(eMeshDuplicateCircularOp, eMeshDuplicateCircularOp_ID, "Dup. circular", ' ', 1, 1, "-1,Mesh")
    OP_INIT()
    {
        eOP_PARAM_ADD_FLOAT("Radius", eF32_MIN, eF32_MAX, 0.0f);
        eOP_PARAM_ADD_INT("Count", 1, 255, 1);
        eOP_PARAM_ADD_FXYZ("Rotation", eF32_MIN, eF32_MAX, 0, 0, 0);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eF32 radius, eU32 count, const eVector3 &rotVal)
    {
        const eVector3 &rotation = rotVal*eTWOPI;
        const eVector3 trans(0.0f, 0.0f, radius);

        eVector3 rot = rotation;
        eF32 step = eTWOPI/(count+1);

        _copyFirstInputMesh();

        const eEditMesh &em = ((eIMeshOp *)getInputOperator(0))->getResult().mesh;
        const eU32 oldVertexCount = m_mesh.getVertexCount();

        for (eU32 i=0; i<count; i++)
        {
            rot += rotation;
            rot.y += step;

            eMatrix4x4 mtxPos;
            mtxPos.translate(trans);
            mtxPos.rotate(rot);

            eMatrix4x4 mtxNrm = mtxPos;
            mtxNrm.invert();
            mtxNrm.transpose();

            const eU32 curVertexCount = m_mesh.getVertexCount();
            m_mesh.merge(em);

            for (eU32 j=curVertexCount; j<m_mesh.getVertexCount(); j++)
            {
                eEditMesh::Vertex *vtx = m_mesh.getVertex(j);
                eASSERT(vtx != eNULL);

                vtx->position *= mtxPos;
                vtx->normal *= mtxNrm;
            }
        }

        // Translate models got from input operator.
        eMatrix4x4 mtx;
        mtx.translate(trans);

        for (eU32 i=0; i<oldVertexCount; i++)
        {
            m_mesh.getVertex(i)->position *= mtx;
        }

        // Update face normals.
        for (eU32 i=0; i<m_mesh.getFaceCount(); i++)
        {
            m_mesh.getFace(i)->updateNormal();
        }
    }
OP_END(eMeshDuplicateCircularOp);
#endif


// UV-Mapping (mesh) operator
// ---------------------
// Maps UV coordinates

#if defined(HAVE_OP_MESH_MODEL2MESH) || defined(eEDITOR)
OP_DEFINE_MESH(eModel2MeshOp, eModel2MeshOp_ID, "Model2Mesh", ' ', 1, 1, "-1,Model")
    OP_INIT()
    {
    }

    OP_EXEC(eGraphicsApiDx9 *gfx) 
    {
        m_mesh.clear();
        const eSceneData& sd = ((eIModelOp*)getInputOperator(0))->getResult().sceneData;

        for(eU32 i = 0; i < sd.getEntryCount(); i++) {
            const eSceneData::Entry& e = sd.getEntry(i);
            if((e.renderableList == eNULL) && (e.renderableObject->getType() == eIRenderable::TYPE_MESH)) {
                const eMesh::Instance& mi = (const eMesh::Instance&)*e.renderableObject;
                const eMesh& mesh = mi.getMesh();
                m_mesh.reserveSpace(mesh.getVertexCount(), mesh.getPrimitiveCount());
                m_mesh.clearAndPreallocate(mesh.getVertexCount(), mesh.getPrimitiveCount(), mesh.getPrimitiveCount() * 6);
                for(eU32 i = 0; i < mesh.getVertexCount(); i++) {
                    const eVertex& v = mesh.getVertex(i);
                    m_mesh.addVertex(v.position, v.texCoord, v.normal);
                }
                for(eU32 i = 0; i < mesh.getPrimitiveCount(); i++) {
                    const eMesh::Primitive& p = mesh.getPrimitive(i);
                    eEditMesh::Vertex* verts[] = {m_mesh.getVertex(p.indices[0]), 
                                                  m_mesh.getVertex(p.indices[1]),
                                                  m_mesh.getVertex(p.indices[2])};
                    const eVector2 texCoords[] = { verts[0]->texCoord,
                                                   verts[1]->texCoord,
                                                   verts[2]->texCoord };
                    eEditMesh::Face* face = m_mesh.addTriangleFast(verts, texCoords);
                    face->material = p.material;
                }
            }
        }
    }
OP_END(eModel2MeshOp);
#endif
