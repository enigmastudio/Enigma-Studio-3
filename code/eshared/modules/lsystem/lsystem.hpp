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

#ifndef LSYSTEM_HPP
#define LSYSTEM_HPP

#include "../../eshared.hpp"

#define LSYS_MAX_ATTRACTORS 4
#define LSYS_MAX_PRODUCTIONS 1000
#define LSYS_MAX_SYMBOLS 20
#define LSYS_MAX_RULES 20
#define LSYS_MAX_PRODUCT_LENGTH 100000
#define LSYS_MAX_STACK 100
#define LSYS_VAR_MIN 'A'
#define LSYS_VAR_MAX (LSYS_VAR_MIN + LSYS_MAX_SYMBOLS - 1)
#define LSYS_PAR_MAX 4
#define LSYS_POLY_INTERPRETATION (LSYS_MAX_SYMBOLS)
#define LSYS_NUM_POLY_INTERPRETATIONS 4
#define LSYS_NUM_INTERPRETATIONS (LSYS_POLY_INTERPRETATION + LSYS_NUM_POLY_INTERPRETATIONS)
#define LSYS_MAX_EDGES 10
#define LSYS_MAX_FACES 1000000

#define DEFAULT_ROTATION eQuat(eVector3(1,0,0), -ePI * 0.5f)
#define DEFAULT_ROTATION_INV eQuat(eVector3(1,0,0), ePI * 0.5f)

class eBezierSpline
{
public:
    eBezierSpline(const eVector3& pos0, const eVector3& pos1, const eQuat& rotation0, const eQuat& rotation1, eU32 axis, eF32 distLen);
	void evaluate(eF32 t, eVector3& resultPos, eQuat& resultRot) const;

    eU32        ax;
	eQuat	    rot0;
	eQuat	    rot1;
	eVector3    control0;
	eVector3    control1;
	eVector3    control2;
	eVector3    control3;
};


class eLSystem
{
public:

    enum FORCE_GEN_TYPE {
        FG_POINT,
        FG_CLOSEST_FACE,
    };

	struct tForceGenerator {
		eU32		attractAxis;
		eF32		mass;

//        FORCE_GEN_TYPE      gen_type;
        eArray<eVector3>*   triDefs;
	};

	__declspec(align(16)) struct tTurtleState {
		eQuat	curBaseRotation;
        eQuat		rotation;
		eVector3	position;
		eF32		size;
		eF32		width;
		eF32		height;
		eU32		polyMatIdx;
		eF32		texAngle;
		eVector2	texPos;
	};

	__declspec(align(16)) struct tSymbol {
        // 0
		eQuat		rotation;		// basic rotation
        // 16
		eU32		symbol;				// type of symbol
		eF32		mass;				// mass
		eF32		params[LSYS_PAR_MAX];
		eU32		parentIdx;
        // 32
		eU32		scopeStartIdx;
		eF32		texVecAngle;
		eVector2	texVec;
        // 48
	};

	__declspec(align(16)) struct tProdSymbol {
		eQuat		correctRot;
//		eVector3	correctPos;
	};

private:

	typedef struct tDrawState {
		tTurtleState				lastTurtle;
		tDrawState*					parentState;
		eArray<eInt>*				lastVertices;
		eArray<eInt>*				curVertices;
		eArray<eInt>*				curTempVertices;
		eF32						texYOffset;
	};

	typedef struct tState {
		tTurtleState				turtle;
		__declspec(align(16)) eQuat	curBaseRotation;
		tState*						parentState;
		eInt						curPolyVertex;
		eU32						scopeStart;

	};

public:

	eLSystem();
    void reset();
	void setDefaultParams(eU32 symbol, eF32* params, const eF32* prevParams);
	void setSubSystem(eU32 symbol, eLSystem* subSys);
    void setSubMesh(eU32 symbol, eSceneData* subMesh, eBool instancing);
	void setParams(const eString& axiomStr, const eString& grammarStr, eU32 iterations, 
                   eF32 detail, eF32 baseWidth, const eArray<tForceGenerator>& forces,
	               eU32 rings,eU32 edges, eF32 texOffset, eF32 texScale, const eArray<eMaterial*>&	materials
                   );
	void evaluate();
	void evaluate(const tState& baseState);
	void allocateMesh(eMesh& destMesh);
	tState* getDefaultState(eF32 scale);
	tDrawState* getDefaultDrawState();
	void draw(eSceneData& destsg, eMesh& destMesh, eF32 size);
	void debugPrintProductions(char* str);
    void resetSubsystems() {
        for(eU32 i = 0; i < 256; i++) {
            m_subSystem[i] = eNULL;
            m_subMesh[i] = eNULL;
	    }
    }
	void addGeometryNeeds(eU32 &neededVerts, eU32& neededFaces);

    eVector3    m_rotationPar;
    eVector3    m_initialPosition;
    eQuat       m_initialRotation;
    eF32        m_sizePar;
    eF32        m_sizeDecayPar;
    eF32        m_gravity;

private:

	void processSub(eSceneData& destsg, eMesh& destMesh, const tTurtleState& turtle);
	void drawInternal(eSceneData& destsg, eMesh& destMesh, tState* state);
	void applyForce(tSymbol& symbol, const eQuat& curRot, eQuat& targetRot, eVector3& targetPos);
	void pushState(tState** state);
	void popState(tState** state);
	void pushDrawState(tDrawState** state);
	void popDrawState(tDrawState** state);
	// returns whether the shape was drawn
	eBool drawShapes(eMesh& destMesh, tDrawState& state, const tTurtleState& turtle0, const tTurtleState& turtle1, eF32 shapeLen, eF32 stexY0, eF32 stexY1, eBool forceDraw, eU32 numParts);

	eU32			paramTable[LSYS_MAX_RULES][LSYS_PAR_MAX];
	eArray<eU32>	symRules[256];
	eU32			ruleOffsets[LSYS_MAX_RULES];

	eArray<tSymbol>	productions[LSYS_MAX_PRODUCTIONS];
	eU32			numProductions;

    tState							stack[LSYS_MAX_STACK];
    eArray<eInt>					verticeStack[LSYS_MAX_STACK];
    eArray<eInt>					verticeTempStack[LSYS_MAX_STACK];
    tDrawState						drawStack[LSYS_MAX_STACK];

    eU32                    polyStackPos;
	eArray<eArray<eInt>>	polygonStack;
	eArray<eInt>*	        polygon;

	eU32 stackPos;
	eU32 drawStackPos;

	eArray<eInt>	vtxLoop;
	eVector2Array	texCoords;

	eString axiom;
    eString grammar;

	eF32        detail;
	eU32        m_iterations;
	eF32		m_baseWidth;
	eArray<tForceGenerator>		m_forces;


	eU32						m_gen_rings;
	eU32						m_gen_edges;
	eF32						m_gen_edge_sinCosTable[LSYS_MAX_EDGES * 2];
	eF32						m_gen_texOffset;
	eF32						m_gen_texScale;
	eArray<eMaterial*>		    m_gen_materials;
	eArray<eU32>		        m_gen_materials_dsIdx;
    eLSystem*                   m_subSystem[256];
    eSceneData*				    m_subMesh[256];
    eBool				        m_subMeshInstancing[256];

	eArray<tProdSymbol>		    m_prodBuffer[2];

};

#endif // LSYSTEM_HPP
