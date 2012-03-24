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

#include "../../eshared.hpp"
#include "lsystem.hpp"

#ifdef eDEBUG
#include <windows.h>
#include <stdio.h>
#endif


struct tSymbolLink {
    eIOperator*     modOp;
    const eChar*    symbolString;
    eBool           instancing;
};

#if defined(HAVE_OP_MISC_LATTRACTOR) || defined(eEDITOR)
OP_DEFINE(eLAttractorOp, eLAttractorOp_ID, eIGenericOp, "LAttractor", Misc, Misc_CID, eColor(170, 170, 170), ' ', 0, 0, "")
    OP_INIT()
    {
        this->m_genData = &m_forceGen;
	    eOP_PARAM_ADD_LABEL("Attractor", "Attractor");
        eOP_PARAM_ADD_LINK("Mesh", "Mesh");
//	    eOP_PARAM_ADD_ENUM("Method", "Center|Faces", 0);
	    eOP_PARAM_ADD_LABEL("Effect", "Effect");
        eOP_PARAM_ADD_FLOAT("Amount", -eF32_MAX, eF32_MAX, 1);
	    eOP_PARAM_ADD_ENUM("Axis", "X|Y|Z", 0);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eIMeshOp* meshOp, /*eU32 method, */eF32 amount, eU32 axis)
    {
        m_forceGen.triDefs = &m_triDefs;
        m_forceGen.attractAxis = axis;
//        m_forceGen.gen_type = (eLSystem::FORCE_GEN_TYPE)method;
        m_forceGen.mass = amount < 0.0f ? - amount * amount : amount * amount;
        

        m_triDefs.clear();
        if(meshOp) {
            const eEditMesh& mesh = meshOp->getResult().mesh;
/*
#ifdef eEDITOR
            if(m_forceGen.gen_type == eLSystem::FG_CLOSEST_FACE) {
                m_triDefs.reserve(mesh.getFaceCount() * 3);
                for(eU32 f = 0; f < mesh.getFaceCount(); f++) {
                    const eEditMesh::Face* face = mesh.getFace(f);
                    const eVector3& v0 = face->he->origin->position;
                    const eVector3& v1 = face->he->next->origin->position;
                    const eVector3& v2 = face->he->next->next->origin->position;
                    m_triDefs.append(v0);
                    m_triDefs.append(v1 - v0);
                    m_triDefs.append(v2 - v0);
                }
            } else if(m_forceGen.gen_type == eLSystem::FG_POINT) {
#endif
*/
                m_triDefs.append(mesh.getBoundingBox().getCenter());
/*
#ifdef eEDITOR
            }
#endif
*/
        }
    }
    eLSystem::tForceGenerator m_forceGen;
    eArray<eVector3>    m_triDefs;
OP_END(eLAttractorOp);
#endif


#if defined(HAVE_OP_MISC_LSYMBOL) || defined(eEDITOR)
OP_DEFINE(eLSymbolOp, eLSymbolOp_ID, eIGenericOp, "LSymbol", Misc, Misc_CID, eColor(170, 170, 170), ' ', 0, 0, "")
    OP_INIT()
    {
        this->m_genData = &m_symbolLink;
        eOP_PARAM_ADD_LINK("Model or Lsys", "Model|Misc");
        eOP_PARAM_ADD_BOOL("Instancing", eFALSE);
		eOP_PARAM_ADD_STRING("Symbols", "");
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eIOperator* modOp, eBool instancing, const eChar *symbolString)
    {
        m_symbolLink.modOp = modOp;
        m_symbolLink.symbolString = symbolString;
        m_symbolLink.instancing = instancing;
    }
    tSymbolLink m_symbolLink;
OP_END(eLSymbolOp);
#endif


#if defined(HAVE_OP_MODEL_LSYSTEM) || defined(eEDITOR)

OP_DEFINE_MODEL(eLSystemOp2, eLSystemOp2_ID, "LSystem", 'l', 0, 20, "-1,Path|Misc")

	eLSystem		m_lsys;


//#define TEST_RANDOM_BUG
#ifdef TEST_RANDOM_BUG
    eArray<eF32>    debug_randomNrs;
    eArray<eF32>    debug_randomSubSum;
    eF32            debug_randomSum;
    eU32            debug_seed;
    eU32            debug_cnt;
    eU32            debug_gseed;
#endif
	OP_INIT()
    {
#ifdef TEST_RANDOM_BUG
        __asm {
            finit;
        }
        debug_gseed = 17;
        debug_seed = 17;
        debug_cnt = 1000;
		eRandomize(debug_seed);
        debug_randomSum = 0.0f;
        for(eU32 i = 0; i < debug_cnt; i++) {
            eF32 r = eRandomF(debug_gseed);
            debug_randomNrs.append(r);
            debug_randomSum += r;
            debug_randomSubSum.append(debug_randomSum);
        }
#endif
		eOP_PARAM_ADD_BOOL("Throws shadows", eFALSE);
        eOP_PARAM_ADD_FLOAT("Detail", 0, 1.0f, 1.0f);
        eOP_PARAM_ADD_INT("Seed", 0, 10000, 0);
		eOP_PARAM_ADD_FXYZ("Orig.Position", -eF32_MAX, eF32_MAX, 0.0f, 0.0f, 0.0f);
		eOP_PARAM_ADD_FXYZ("Orig.Rotation", -eF32_MAX, eF32_MAX, 0.0f, 0.0f, 0.0f);

		eOP_PARAM_ADD_LABEL("Structure", "Structure");
		eOP_PARAM_ADD_STRING("Axiom", "X(0.1)");
		eOP_PARAM_ADD_TEXT("Productions", "X(1)(1) A-<[[X]+>X]+>A[+>AX]-<X; \nA(1)(1) AA;");
        eOP_PARAM_ADD_INT("Iterations", 0, 1000, 5);

		eOP_PARAM_ADD_LABEL("Variation", "Variation");
		eOP_PARAM_ADD_FLOAT("Size", 0, eF32_MAX, 1.0f);
		eOP_PARAM_ADD_FLOAT("Size Decay", 0, eF32_MAX, 0.88f);
		eOP_PARAM_ADD_FXYZ("Rotation", -eF32_MAX, eF32_MAX, 0.0f, 0.0f, 0.0f);
		eOP_PARAM_ADD_FLOAT("Gravity", -eF32_MAX, eF32_MAX, 0.0f);

		eOP_PARAM_ADD_LABEL("Drawing", "Drawing");
        eOP_PARAM_ADD_INT("Steps", 1, 1000, 5);
        eOP_PARAM_ADD_INT("Edges", 3, LSYS_MAX_EDGES, 5);
        eOP_PARAM_ADD_FLOAT("Radius", 0, eF32_MAX, 1.0);
		eOP_PARAM_ADD_FXY("TexOffset/Scale",0, eF32_MAX, 1,1);
		eOP_PARAM_ADD_LINK("Material1","Misc");
		eOP_PARAM_ADD_LINK("Material2","Misc");
		eOP_PARAM_ADD_LINK("Material3","Misc");
		eOP_PARAM_ADD_LINK("Material4","Misc");

        m_mi = eNULL;
    }

    OP_DEINIT()
    {
        eSAFE_DELETE(m_mi);
    }

#ifdef eEDITOR
    OP_OPTIMIZE_FOR_EXPORT() {
        eParameter& grammarPar = this->getParameter("Productions");
        eChar* grammar = grammarPar.m_value.string;
        // remove 
        eU32 srcPos = 0;
        eU32 dstPos = 0;
        while(true) {
            if((grammar[srcPos] != '\n') &&
               (grammar[srcPos] != '\r') &&
               (grammar[srcPos] != ' ') 
                ) {
                grammar[dstPos] = grammar[srcPos];
                dstPos++;
            }
            if(grammar[srcPos] == 0)
                break;
            srcPos++;
        }
//        eShowError(grammar);
    }
#endif

    OP_EXEC(eGraphicsApiDx9 *gfx, 
				eBool throwsShadows, eF32 detail, eInt seed,
                const eVector3 &origPosition, const eVector3 &origRotation,
				const eChar *axiomStr, const eChar *productions, eInt maxIterations,
                eF32 sizePar, eF32 decayPar,
                const eVector3 &rotationPar_, 
                eF32 gravity,
                eInt numRings, eInt numEdges, eF32 radius, const eVector2 &texOffScale, 
                const eIMaterialOp *matOp1, const eIMaterialOp *matOp2, const eIMaterialOp *matOp3, const eIMaterialOp *matOp4
			)
    {
		ePROFILER_ZONE("L-System");

#ifdef TEST_RANDOM_BUG
        __asm {
            finit;
        }
        debug_gseed = 17;
		eRandomize(debug_seed);
        eArray<eF32>    d_randomNrs;
        eF32 d_randomSum = 0.0f;
        for(eU32 i = 0; i < debug_cnt; i++) {
            eF32 r = eRandomF(debug_gseed);
            d_randomNrs.append(r);
            if(r != debug_randomNrs[i])
                eShowError("ERRORR Random number different!!!");

            eASSERT(r == debug_randomNrs[i]);
            d_randomSum += r;
            eASSERT(d_randomSum == debug_randomSubSum[i]);
        }
        if(debug_randomSum != d_randomSum)
            eShowError("ERRORR sum different !!!");
        debug_randomSum = d_randomSum;
#endif
		eRandomize(seed);

        // reset subsystems
        m_lsys.resetSubsystems();
        m_lsys.m_rotationPar = rotationPar_;
        m_lsys.m_initialPosition = origPosition;
        eQuat q;
        q.fromRotation(origRotation);
        q.y = -q.y;
        q.z = -q.z;
        m_lsys.m_initialRotation = (DEFAULT_ROTATION * q) * DEFAULT_ROTATION_INV;
        m_lsys.m_sizePar = sizePar;
        m_lsys.m_sizeDecayPar = decayPar;
        m_lsys.m_gravity = gravity * gravity;

		eArray<eLSystem::tForceGenerator>		forces;
		for(eU32 a = 0; a < this->getInputCount(); a++) {
            // check for symbol interpretation
            if(TEST_OP_TYPE(getInputOperator(a), "Misc : LSymbol", eLSymbolOp_ID)) {
                tSymbolLink* symLink = (tSymbolLink*)((eIGenericOp *)getInputOperator(a))->getResult().genericDataPtr;
                if(symLink->modOp) {
			        eU32 p = 0;
                    while(symLink->symbolString[p] != 0) {
                        if(TEST_OP_TYPE(symLink->modOp, "Model : LSystem", eLSystemOp2_ID)) {
                            // override sub-lsystems
                            m_lsys.setSubSystem(symLink->symbolString[p], &((eLSystemOp2*)symLink->modOp)->m_lsys);
                        } else {
                            // symbol is a model
                            m_lsys.setSubMesh(symLink->symbolString[p], &((eIModelOp*)symLink->modOp)->getResult().sceneData, symLink->instancing);
                        }
                        p++;
                    }
                }
            } else if(TEST_OP_TYPE(getInputOperator(a), "Misc : LAttractor", eLAttractorOp_ID)) {
                eLSystem::tForceGenerator* forceLink = (eLSystem::tForceGenerator*)((eIGenericOp *)getInputOperator(a))->getResult().genericDataPtr;
                if(forceLink->triDefs->size() != 0)
			        forces.append(*forceLink);
            };
		}

		eArray<eMaterial*>		materials;
        for(eU32 i = 0; i < 4; i++) {
            const eIMaterialOp* matOp = eNULL;
            switch(i) {
            case 0: matOp = matOp1; break;
            case 1: matOp = matOp2; break;
            case 2: matOp = matOp3; break;
            case 3: matOp = matOp4; break;
            }
            if(matOp != eNULL)
                materials.append(&matOp->getResult().material);
        }


		m_lsys.setParams(axiomStr, productions, maxIterations, detail, radius, forces, numRings, numEdges, texOffScale.x, texOffScale.y, materials);
		m_lsys.evaluate();

#ifdef eEDITOR
/*
		if(showProductions != m_lastShowProd) {
			m_lastShowProd = showProductions;
			char buffer[10000];
			m_lsys.debugPrintProductions(buffer);
			eShowError(buffer);
		}
*/
#endif



		// Create dynamic mesh if it is affected
        // by an animation.
//        const eMesh::Type meshType = (isAffectedByAnimation() ? eMesh::TYPE_DYNAMIC : eMesh::TYPE_STATIC);
        const eMesh::Type meshType = eMesh::TYPE_DYNAMIC;

		m_rmesh.clear();
	    eU32 neededVerts = 0;
	    eU32 neededFaces = 0;
	    // prepare mesh generation
	    m_lsys.addGeometryNeeds(neededVerts, neededFaces);
	    m_rmesh.reserveSpace(neededFaces, neededVerts);

		m_lsys.draw(m_sceneData, m_rmesh, 1.0f);

        m_rmesh.finishLoading(meshType);

        eSAFE_DELETE(m_mi);
        m_mi = new eMesh::Instance(m_rmesh, throwsShadows);
        eASSERT(m_mi != eNULL);

        m_sceneData.addRenderable(m_mi);
    }
    OP_VAR(eMesh::Instance *m_mi);
    OP_VAR(eMesh m_rmesh);
OP_END(eLSystemOp2);
#endif


// Populate Surface (model) operator
// -----------------------------------
// Populates the surface of a mesh with models.

#if defined(HAVE_OP_MODEL_POPULATE_SURFACE) || defined(eEDITOR)
OP_DEFINE_MODEL(eModelPopulateSurfaceOp, eModelPopulateSurfaceOp_ID, "Populate Surface", 'p', 1, 1, "-1,Mesh")
    struct tEntry {
        eMatrix4x4  matrix;
        eQuat       rotation;
        eVector3    position;
        eF32        size;
    };

    eArray<tEntry>  m_entries;

    struct rEntry {
        eMesh       rmesh;
        eMesh::Instance* mi;
        eSceneData  sd;
    };

    eArray<rEntry>  m_rentries;
	eArray<eF32>    m_faceAreas;

    OP_VAR(eMesh::Instance *m_mi);
    OP_VAR(eMesh m_rmesh);


	OP_INIT()
    {
        eOP_PARAM_ADD_INT("Seed", 0, 10000, 0);
        eOP_PARAM_ADD_INT("Count", 1, 100000, 5);
        eOP_PARAM_ADD_FLOAT("Minimum Distance", 0.0f, 1.0f, 0.0f);
        eOP_PARAM_ADD_FLOAT("Size", 0.0f, eF32_MAX, 1.0f);
        eOP_PARAM_ADD_FLOAT("Size Variation", 0.0f, 1.0f, 0.0f);
        eOP_PARAM_ADD_ENUM("Normal Calculation", "Smooth|Flat", 0);
        eOP_PARAM_ADD_LINK("Model", "Model");
        eOP_PARAM_ADD_BOOL("Recalc Lsys", eFALSE);
        eOP_PARAM_ADD_BOOL("Instancing", eTRUE);
        eOP_PARAM_ADD_BOOL("Throws shadows", eFALSE);
        m_mi = eNULL;
    }

#define POP_SURF_MAX_RETRIES 10
	OP_EXEC(eGraphicsApiDx9 *gfx, eInt seed, eU32 count, eF32 minDist, eF32 size, 
                                eF32 sizeVariation, eInt normalCalc, eIModelOp* model, eBool recalcLsys, 
                                eBool instancing, eBool throwsShadows)
    {
		ePROFILER_ZONE("Populate Surface");
		if(model != eNULL) {

	        eBool doRecalc = (this->getParameter(0).getChanged() ||
			                      this->getParameter(1).getChanged() ||
			                      this->getParameter(2).getChanged() ||
			                      this->getParameter(3).getChanged() ||
			                      this->getParameter(4).getChanged() ||
			                      this->getParameter(5).getChanged() ||
			                      this->getParameter(7).getChanged() ||
			                      (!this->getParameter(6).getChanged()) ||
						          m_entries.size() == 0);
            if(doRecalc) {
                ePROFILER_ZONE("Populate Generate");
     
    		    eRandomize(seed);
                const eEditMesh &mesh = ((eIMeshOp *)getInputOperator(0))->getResult().mesh;
			    // TODO: the following code is largely similar to the random face-point selection in the psys module

			    eF32 areaSum = 0.0f;
			    m_faceAreas.resize(mesh.getFaceCount());
			    for (eU32 i=0; i<mesh.getFaceCount(); i++) {
				    // Calc face area.
				    areaSum += mesh.getFace(i)->getArea();
				    // Add to sum.
				    m_faceAreas[i] = areaSum;
			    }

                m_entries.clear();
    
			    for(eU32 i = 0; i < count; i++) {
				    eVector3 pos, normal;
    //					eF32 curVariation = eRandomFNormal(useed) * sizeVariation;
				    eF32 curVariation = eRandomF() * sizeVariation;
				    eF32 curSize = size * (1.0f + curVariation);
				    eF32 minimumDistance = minDist * (0.5f * eSqrt(areaSum)) * (1.0f + curVariation);
				    eF32 minDistSqr = minimumDistance * minimumDistance;
				    eU32 retries = 0;


				    while(retries < POP_SURF_MAX_RETRIES) {
					    // lookup random entity with binary search
					    eF32 a = eRandomF() * areaSum;
					    eU32 l = 0;
					    eU32 r = m_faceAreas.size();
					    while (l < r) {
						    eU32 m = (l + r) >> 1;
						    if (a < m_faceAreas[m])   r = m;
						    else                    l = m+1;
					    }
					    eU32 f = eClamp((eU32)0, r , m_faceAreas.size()-1);

					    const eEditMesh::Face *tri = mesh.getFace(f);
					    eASSERT(tri != eNULL);
					    tri->getRandomSurfacePoint(pos, normal);
					    if(normalCalc == 1)
						    normal = tri->normal;
					    bool isnear = false;
					    for(eU32 k = 0; k < m_entries.size(); k++) 
                            isnear |= (pos - m_entries[k].position).sqrLength() < minDistSqr;
					    if(!isnear)
						    break;
					    retries++;
				    }

				    if(retries < POP_SURF_MAX_RETRIES) {
                        tEntry& entry = m_entries.push();
                        entry.position = pos;
                        entry.size = curSize;

					    eQuat rot(eVector3(0,1,0), eRandomF() * ePI * 2.0f);
					    eVector3 up(0,1,0);
					    normal.normalize();
					    eF32 dot = up * normal;
					    if(dot < 0) {
						    rot = rot * eQuat(eVector3(1,0,0), ePI);
						    up = eVector3(0,-1,0);
						    dot = up * normal;
					    }
					    eVector3 cross = (normal ^ up).normalized();
					    eF32 angle = eACos(dot);

					    if(eAreFloatsEqual(cross.sqrLength(), 1.0f))
						    rot = rot * eQuat(cross, angle);

                        entry.rotation = rot;
                        entry.rotation.x = -entry.rotation.x;

                        entry.matrix = eMatrix4x4(rot);
					    entry.matrix.scale(eVector3(curSize));
					    entry.matrix.translate(pos);

				    }

			    }
            }

            m_sceneData.clear();
            if(recalcLsys) {
#if defined(HAVE_OP_MODEL_LSYSTEM) || defined(eEDITOR)
                eLSystemOp2* lsysOp = (eLSystemOp2*)model;
                eLSystem& lsys = lsysOp->m_lsys;
                m_rentries.reserve(count);
                while(m_rentries.size() < count) {
                    rEntry& r = m_rentries.push(rEntry());
                    r.mi = eNULL;
                }

	            // prepare mesh generation
	            eU32 neededVerts = 0;
	            eU32 neededFaces = 0;
	            lsys.addGeometryNeeds(neededVerts, neededFaces);

                // Create dynamic mesh if it is affected
                // by an animation.
                const eMesh::Type meshType = (isAffectedByAnimation() ? eMesh::TYPE_DYNAMIC : eMesh::TYPE_STATIC);

                eVector3 oldPos = lsys.m_initialPosition;
                eQuat oldRot = lsys.m_initialRotation;
                eF32 oldSize = lsys.m_sizePar;
			    for(eU32 k = 0; k < m_entries.size(); k++) {
                    tEntry& entry = m_entries[k];
                    rEntry& rentry = m_rentries[k];
                    eMesh& mesh = rentry.rmesh;
                    mesh.clear();
                    mesh.reserveSpace(neededFaces, neededVerts);

                    lsys.m_initialPosition = entry.position;
                    lsys.m_initialRotation = (DEFAULT_ROTATION * entry.rotation) * DEFAULT_ROTATION_INV;
                    lsys.m_sizePar = entry.size;

                    // clear translation
                    lsys.reset();
                    lsys.evaluate();
		            lsys.draw(m_sceneData, mesh, 1.0f);

                    mesh.finishLoading(meshType);

                    eSAFE_DELETE(rentry.mi);
                    rentry.mi = new eMesh::Instance(mesh, throwsShadows);
//                    eASSERT(mi != eNULL);

                    rentry.sd.clear();
                    rentry.sd.addRenderable(rentry.mi);
                    m_sceneData.merge(rentry.sd);
                }

                lsys.m_initialPosition = oldPos;
                lsys.m_initialRotation = oldRot;
                lsys.m_sizePar = oldSize;
#endif
            } else {
                // normal models
			    eSceneData &sd = model->getResult().sceneData;

                if(instancing)
			        for(eU32 k = 0; k < m_entries.size(); k++) 
                        m_sceneData.merge(sd, m_entries[k].matrix);
                else {
                    eU32 vcnt = 0, fcnt = 0;
                    sd.convertToMeshOrCount(vcnt, fcnt);
                    m_rmesh.clear();
                    m_rmesh.reserveSpace(fcnt * m_entries.size(), vcnt * m_entries.size());
			        for(eU32 k = 0; k < m_entries.size(); k++) 
                        sd.convertToMeshOrCount(vcnt, fcnt, m_entries[k].matrix, &m_rmesh);

                    const eMesh::Type meshType = (isAffectedByAnimation() ? eMesh::TYPE_DYNAMIC : eMesh::TYPE_STATIC);
                    m_rmesh.finishLoading(meshType);

                    eSAFE_DELETE(m_mi);
                    m_mi = new eMesh::Instance(m_rmesh, throwsShadows);
                    eASSERT(m_mi != eNULL);

                    m_sceneData.addRenderable(m_mi);
                }
            }
		}
    }
OP_END(eModelPopulateSurfaceOp);
#endif
