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

#ifdef eEDITOR
#include <windows.h>
#include <stdio.h>
#endif

#include "lsystem.hpp"

eBezierSpline::eBezierSpline(const eVector3& pos0, const eVector3& pos1, const eQuat& rotation0, const eQuat& rotation1, eU32 axis, eF32 distLen) 
    : rot0(rotation0), rot1(rotation1), control0(pos0), control3(pos1), ax(axis) {
	control1 = pos0   + rotation0.getVector(axis) * 0.333333f * distLen;
	control2 = pos1   - rotation1.getVector(axis) * 0.333333f * distLen;
}

void eBezierSpline::evaluate(eF32 t, eVector3& resultPos, eQuat& resultRot) const {
	resultRot = rot0.slerp(t, rot1);

    __declspec(align(16)) eVector3 distNorm;
	eVector3::cubicBezier(t, control0, control1, control2, control3, resultPos, distNorm);

	eVector3 look = resultRot.getVector(ax);
	eVector3 side = (look^distNorm);
	eF32 sideLenSqr = side.sqrLength();
	if(sideLenSqr > eALMOST_ZERO) {
		side /= eSqrt(sideLenSqr);
		eF32 dot = eClamp(-1.0f, look * distNorm, 1.0f);
		eF32 alpha = eACos(dot) * (1.0f / (2.0f * ePI));
		eQuat rotation(side, alpha);
		resultRot = rotation * resultRot;
	}
/**/
}

#if defined(eEDITOR)
eBool checkValidity(const char* str) {
	eU32 num1 = 0;
	eU32 num2 = 0;
	eU32 num3 = 0;
	eU32 pos = 0;
	while(str[pos] != 0) {
		if(str[pos] == '[')				num1++;
		if(str[pos] == ']')				num1--;
		if(str[pos] == '{')				num2++;
		if(str[pos] == '}')				num2--;
		if(str[pos] == '(')				num3++;
		if(str[pos] == ')')				num3--;
		if((num1 < 0) || (num2 < 0) || (num3 < 0))
			return eFALSE;
		pos++;
	}
	if((num1 != 0) || (num2 != 0) || (num3 != 0))
		return eFALSE;
	return eTRUE;
}
#endif

// calculates the expression in the string
// returns the position after second number
eS32 calculateTerm(const eString& s, int start, int end, const eU32* parMap, const eF32* params, eF32& result) {
	int pos = start;
	eF32 tuple[] = {0,0};
	eU32 tpos = 0;
	eU32 op = 0;
    eU32 c;
	while((pos < end) && ((c = s.at(pos)) != ',') && (c != ')')) {
        if(c == '(') {
			pos = calculateTerm(s, pos + 1, end, parMap, params, tuple[tpos]);
			eASSERT(pos != -1);
			pos++;
        } else {
            eS32 oldPos = pos;
			// try to read constant
            eF32 number = 0;
	        eF32 v = 1.0f;
	        while(pos < end) {
		        int cc = s.at(pos) - '0';
		        if(cc == '.' - '0')
			        v = 0.1f;
		        else if((eU32)cc <= 9) {// is a number
				        if(v >= 1.0f)
					        number = number * 10.0f + cc;
				        else {
					        number += v * cc;
					        v /= 10.0f;
				        }
                    tuple[tpos] = number;
               }
		        else break;
		        pos++;
	        }

 			// try to read symbol
            eASSERT(parMap != eNULL);
			for(eU32 i = 0; i < LSYS_PAR_MAX; i++)
				if(parMap[i] == c) {
					tuple[tpos] = params[i];
					pos++;
				}

            if(pos == oldPos) {
                // is an operator
                pos++;  
                op = c;	
                tpos++;
            };
		}
	}
	switch(op) {
	case '+': result = tuple[0] + tuple[1]; break;
	case '-': result = tuple[0] - tuple[1]; break;
	case '*': result = tuple[0] * tuple[1]; break;
	case '/': result = tuple[0] / tuple[1]; break;
	case '<': result = (tuple[0] < tuple[1]) ? 1.0f : 0.0f; break;
	case '=': result = (tuple[0] == tuple[1]) ? 1.0f : 0.0f; break;
	case '>': result = (tuple[0] > tuple[1]) ? 1.0f : 0.0f; break;
	case '^': 
        result = ePow(tuple[0], tuple[1]); break;
    default:
		result = tuple[0]; // single term
	}
	return pos;
}

eS32 readValInstanciation(const eString& s, int start, int end, eU32* parMap, eF32* params, const eF32* prevParams) {
	int pos = start;
	int v = 0;

	if((pos >= end) || (s.at(pos) != '(')) {
		return pos;
	} else {
		pos++;
		while((pos < end) && (s.at(pos) != ')')) {
			if(s.at(pos) == ',') {
				pos++;
				continue;
			}
			pos = calculateTerm(s, pos, end, parMap, prevParams, params[v++]); 
			eASSERT(pos != -1);
		}
	}
	return pos + 1;
}

eLSystem::eLSystem() :
    m_initialRotation(eQuat())
{
	numProductions = 0;
	stackPos = 0;
    polyStackPos = 0;
}

void eLSystem::reset() {

	this->productions[0].clear();

	// read axiom
	eS32 pos = 0;
	while(pos < (eS32)axiom.length()) {
		// read symbol
		tSymbol& symbol = this->productions[0].push(tSymbol());			

		symbol.symbol = axiom.at(pos++);
		this->setDefaultParams(symbol.symbol, &symbol.params[0], eNULL);
		symbol.texVecAngle = 0.0f;
		symbol.texVec = eVector2(0,0);
		symbol.mass = 1;
//		symbol.rotation = eQuat();
        symbol.rotation = this->m_initialRotation;

		// read symbol params
		pos = readValInstanciation(axiom, pos, axiom.length(), eNULL, &symbol.params[0], eNULL);
		eASSERT(pos != -1);
	}
}

void eLSystem::setDefaultParams(eU32 symbol, eF32* params, const eF32* prevParams) {
	for(eU32 i = 0; i < LSYS_PAR_MAX; i++)
		params[i] = 1.0;
	switch(symbol) {
        case '+':	params[0] = this->m_rotationPar.x;	break;
		case '-':	params[0] = this->m_rotationPar.x;	break;
		case '<':	params[0] = this->m_rotationPar.y;	break;
		case '>':	params[0] = this->m_rotationPar.y;	break;
		case '\\':	params[0] = this->m_rotationPar.z;	break;
		case '/':	params[0] = this->m_rotationPar.z;	break;
	}
}

void eLSystem::setSubSystem(eU32 symbol, eLSystem* subSys) {
    this->m_subSystem[symbol] = subSys;
}

void eLSystem::setSubMesh(eU32 symbol, eSceneData* subMesh, eBool instancing) {
	m_subMesh[symbol] = subMesh;
    m_subMeshInstancing[symbol] = instancing;
}

void eLSystem::setParams(const eString& axiomStr, const eString& grammarStr, eU32 iterations, 
                eF32 detail, eF32 baseWidth, const eArray<tForceGenerator>& forces,
	            eU32 rings,eU32 edges, eF32 texOffset, eF32 texScale, const eArray<eMaterial*>&	materials
                ) {
    this->m_gen_rings = rings;
    this->m_gen_edges = edges;
    this->m_gen_texOffset = texOffset;
    this->m_gen_texScale = texScale;
    this->m_gen_materials = materials;
	this->m_baseWidth = baseWidth;
	this->m_iterations = iterations;
	this->detail = detail;
    this->axiom = axiomStr;
	this->grammar = grammarStr;
    this->m_forces = forces;

#if defined(eEDITOR)
	// check lsystem for regularity
	if( (!checkValidity(this->axiom)) ||
		(!checkValidity(this->grammar)))
		return;
#endif

	// read rules
	eS32 pos = 0;
	int end = this->grammar.length();
#ifdef eEDITOR
	// clear whitespaces
	while(pos < end) {
		if((this->grammar.at(pos) == ' ') || (this->grammar.at(pos) == '\n')) {
			// copy end of string
			for(int k = pos + 1; k < end; k++)
				this->grammar.at(k-1) = this->grammar.at(k);
			end--;
		} else
			pos++;
	}
	pos = 0;
#endif				

	// clear rules
	for(int i = 0; i < 256; i++)
		symRules[i].clear();
    eMemSet(paramTable, 0, LSYS_MAX_RULES * LSYS_PAR_MAX * sizeof(eU32));

	// iterate through rules
	eU32 ruleCnt = 0;
	while(pos < end) {
		// read pred symbol
		int symbol = this->grammar.at(pos++);
		// read symbol table
	    eU32 resultCnt = 0;
        while((pos < end) && ((eU32)(this->grammar.at(pos) - 'a') < 26)) 
		    paramTable[ruleCnt][resultCnt++] = this->grammar.at(pos++);

		ruleOffsets[ruleCnt] = pos;
		symRules[symbol].append(ruleCnt);
		// find next rule offset
		while((pos < end) && (this->grammar.at(pos) != ';'))
			pos++;
		pos++;
		ruleCnt++;
	}

	// calc sincos table
	for(eU32 i = 0; i <= m_gen_edges; i++) {
		eF32 et = (eF32)i / (eF32)m_gen_edges;
        eSinCos(et * eTWOPI, this->m_gen_edge_sinCosTable[i * 2], this->m_gen_edge_sinCosTable[i * 2 + 1]);
//		this->m_gen_edge_sinCosTable[i * 2] = eSin(et * eTWOPI);
//		this->m_gen_edge_sinCosTable[i * 2 + 1] = eCos(et * eTWOPI);
	}

	this->reset();
}


void eLSystem::evaluate() {
#if defined(eEDITOR)
	// check lsystem for regularity
	if( (!checkValidity(this->axiom)) ||
		(!checkValidity(this->grammar)))
		return;
#endif
	this->evaluate(*this->getDefaultState(1.0f));
}

void eLSystem::evaluate(const tState& baseState) {
	this->numProductions = 1;
	ePROFILER_ZONE("L-System - Evaluate");
	// interpret product
	eU32 initialStackpos = stackPos;
	this->reset();
	// now we can start derivating
	for(eU32 it = 0; it < m_iterations; it++) {

		tState					sstate = baseState;
		tState				    *state = &sstate;

		state->scopeStart = 0;

		eArray<tSymbol>& curProduction = this->productions[numProductions - 1];			
		eArray<tSymbol>& nextProduction = this->productions[numProductions++];	
		nextProduction.clear();		
		int ppos = 0;
		while(ppos < (eS32)curProduction.size()) {
			const tSymbol& curSymbol = curProduction[ppos];
			eS32 symbolRaw = curSymbol.symbol;
			eBool isVariable = (symbolRaw >= LSYS_VAR_MIN) && (symbolRaw <= LSYS_VAR_MAX);

			state->turtle.rotation = state->curBaseRotation * curSymbol.rotation;
			state->turtle.texAngle = curSymbol.texVecAngle;

			if(symbolRaw == '[')	this->pushState(&state);
			if(symbolRaw == ']')	this->popState(&state);

			//////////////////////////////
			/// Calculate next product ///
			//////////////////////////////
							
			// initialize next
			tSymbol& nextSymbol = nextProduction.push(curSymbol);
			nextSymbol.rotation = state->turtle.rotation;
			nextSymbol.texVecAngle = state->turtle.texAngle;
            eSinCos(state->turtle.texAngle, nextSymbol.texVec.x, nextSymbol.texVec.y);
//			nextSymbol.texVec = eVector2(eSin(state->turtle.texAngle), eCos(state->turtle.texAngle));
			nextSymbol.parentIdx = ppos;
			nextSymbol.scopeStartIdx = state->scopeStart;

			// TODO: reenable parameter copy
			//for(int i = 0; i < LSYS_MAX_SYMBOLS; i++)
			//	nextParam[nextProdLen * LSYS_MAX_SYMBOLS + i] = param[ppos * LSYS_MAX_SYMBOLS + i];

			if(symbolRaw == '[') {
				this->pushState(&state);
				state->scopeStart = nextProduction.size() - 1;
			} else if(symbolRaw == ']') {
				this->popState(&state);
			} else /*if(isVariable)*/ {
				int		rpos[LSYS_MAX_RULES];
				eU32	ruleNr[LSYS_MAX_RULES];
				eU32 numRules = 0;

				// iterate through rules and lookup those with matching boolean conditions
				for(eU32 i = 0; i < symRules[symbolRaw].size(); i++) {
					ruleNr[i] = symRules[symbolRaw][i];
					rpos[numRules] = ruleOffsets[ruleNr[i]];

					// read boolean expression
					eF32 ruleCondition = 1.0f;
                    if(this->grammar.at(rpos[numRules]) == '(') {
					    rpos[numRules] = 1 + calculateTerm(this->grammar, rpos[numRules] + 1, this->grammar.length(), &paramTable[ruleNr[i]][0], &curSymbol.params[0], ruleCondition);
					    eASSERT(rpos[numRules] != 0);
                    }
                    if(ruleCondition != 0) {
						// rule can be applied
						eASSERT(rpos[numRules] != 0);
						numRules++;
					}
				}

				if(numRules != 0) {
					nextProduction.removeLastElement();
					// pick rule at random
                    eU32 i = eRandom(0, numRules);
					eASSERT(rpos[i] >= 0);
					// take this rule
					while(rpos[i] < (eS32)this->grammar.length()) {
						// read symbol
						int nsym = this->grammar.at(rpos[i]++);
						if(nsym == ';')
							break;
									
						eF32 sparams[LSYS_PAR_MAX];
						if(this->grammar.at(rpos[i]) == '(') 
							rpos[i] = readValInstanciation(this->grammar, rpos[i], this->grammar.length(), 
															&paramTable[ruleNr[i]][0], &sparams[0], &curSymbol.params[0]);
						else
							this->setDefaultParams(nsym, &sparams[0], &curSymbol.params[0]);
									
//								eVector3 rotateAxis;
						eF32 rotateAmount = 0.0f;
                        eU32 rotateAxis;
						switch(nsym) {

							case '|': rotateAxis = 0; rotateAmount = ePI; break;
							case '+': rotateAxis = 0; rotateAmount = -sparams[0]; state->turtle.texAngle -= sparams[0]; break;
							case '-': rotateAxis = 0; rotateAmount = sparams[0]; state->turtle.texAngle += sparams[0]; break;
							case '<': rotateAxis = 1; rotateAmount = -sparams[0]; break;
							case '>': rotateAxis = 1; rotateAmount = sparams[0]; break;
							case '\\': rotateAxis = 2; rotateAmount = -sparams[0]; break;
							case '/': rotateAxis = 2; rotateAmount = sparams[0]; break;
#ifdef eEDITOR
							case '$':	// roll to horizontal left axis
                                eShowError("Temporarily disabled");
                                break;
#endif
/*                                        
							case '$':	// roll to horizontal left axis
									{
										eVector3 look = state->turtle.rotation.getVector(2);
										eVector3 curLeft =		state->turtle.rotation.getVector(0);
										eVector3 targetLeft = (eVector3(0,1,0) ^ look).normalized();
										eF32 cosAngle = eClamp(-1.0f, curLeft * targetLeft, 1.0f);
										eF32 angle = eACos(cosAngle);
										rotateAxis = look;
										rotateAmount = angle;
										break;
									}
*/
							default:
//										ePROFILER_ZONE("L-System - Evaluate Expand");
								tSymbol& nextSymbol = nextProduction.push(tSymbol());

								nextSymbol.symbol = nsym;
								for(eU32 p = 0; p < LSYS_PAR_MAX; p++)
									nextSymbol.params[p] = sparams[p];
								nextSymbol.parentIdx = ppos;
								nextSymbol.scopeStartIdx = state->scopeStart;
								nextSymbol.rotation = state->turtle.rotation;
								nextSymbol.mass = (state->turtle.size * state->turtle.height) * (state->turtle.size * state->turtle.width);
								nextSymbol.texVecAngle = state->turtle.texAngle;
                                eSinCos(state->turtle.texAngle, nextSymbol.texVec.x, nextSymbol.texVec.y);
//								nextSymbol.texVec = eVector2(eSin(state->turtle.texAngle), eCos(state->turtle.texAngle));

								if(nsym == '[') {
									this->pushState(&state);
									state->scopeStart = nextProduction.size() - 1;
								} else if(nsym == ']') {
									this->popState(&state);
								}

								// propagate mass to parent symbols
								if((nsym >= LSYS_VAR_MIN) && (nsym <= LSYS_VAR_MAX)) {
//											ePROFILER_ZONE("L-System - Propagate Mass");
									eU32 subCnt = 0;
									for(int i = nextProduction.size() - 2; i >= 0; i--) {
										if(nextProduction[i].symbol == ']')
											i = nextProduction[i].scopeStartIdx;
										else {
											// add mass
											nextProduction[i].mass += nextSymbol.mass;
										}
									}
								}

						}

						if(rotateAmount != 0.0f) {
							state->curBaseRotation = eQuat(state->turtle.rotation.getVector(rotateAxis), rotateAmount) * state->curBaseRotation;
							state->turtle.rotation = state->curBaseRotation * curSymbol.rotation;
						}
					}
				}

			}
			ppos++;
		}
	}

	eASSERT(stackPos==initialStackpos);
}

eLSystem::tState* eLSystem::getDefaultState(eF32 scale) {
	tState *state;
	state = &stack[0];

    state->turtle.rotation = DEFAULT_ROTATION * this->m_initialRotation;
//    state->turtle.rotation = eQuat();
    state->turtle.position = this->m_initialPosition;
    state->turtle.polyMatIdx = 0;
	state->turtle.texPos = eVector2(0.5f, 0.5f);
	state->turtle.texAngle = 0.0f;
	state->curPolyVertex = eNULL;
	state->parentState = eNULL;
    state->curBaseRotation = eQuat();
    state->turtle.size = scale * this->m_sizePar;
	state->turtle.height = 1.0f;
	state->turtle.width =this->m_baseWidth;
	return state;
}

eLSystem::tDrawState* eLSystem::getDefaultDrawState() {
	tDrawState *state;
	state = &drawStack[0];

	state->texYOffset = this->m_gen_texOffset;
	state->curVertices = &verticeStack[0];
	state->curTempVertices = &verticeTempStack[0];
	state->lastVertices = &verticeTempStack[0];
	return state;
}

void eLSystem::draw(eSceneData& destsg, eMesh& destMesh, eF32 size) {
	size *= this->m_sizeDecayPar;
	tState state = *getDefaultState(size);
    state.curBaseRotation = DEFAULT_ROTATION;
    state.turtle.rotation = DEFAULT_ROTATION * this->m_initialRotation;
    state.turtle.position = this->m_initialPosition;
	this->drawInternal(destsg, destMesh, &state);
}

void eLSystem::debugPrintProductions(char* str) {
#ifdef eEDITOR
	eU32 s = 0;
	for(eU32 i = 0; i < this->numProductions; i++) {
		s += sprintf(&str[s], "%i: ", i);
		eArray<tSymbol>& production = productions[i];
		for(eU32 k = 0; k < production.size(); k++) {
			str[s++] = production[k].symbol;
		}
		str[s++] = '\n';
	}
	str[s] = 0;
#endif
}

void eLSystem::processSub(eSceneData& destsg, eMesh& destMesh, const tTurtleState& turtle) {
	tState* state = getDefaultState(turtle.size);
	state->curBaseRotation = turtle.rotation;
	state->turtle.position = turtle.position;
    state->turtle.rotation = turtle.rotation;
	this->drawInternal(destsg, destMesh, state);
}

void _mergeIntoMesh(eMesh& destMesh, const eMatrix4x4& mtx, eMatrix4x4& rotMtx, const eSceneData::Entry& e) {
    if(e.renderableList != eNULL) {
        for(eU32 i = 0; i < e.renderableList->getEntryCount(); i++)
            _mergeIntoMesh(destMesh, mtx, rotMtx, e.renderableList->getEntry(i));
    } else if(e.renderableObject->getType() == eIRenderable::TYPE_MESH) {
        const eMesh::Instance& mi = (const eMesh::Instance&)*e.renderableObject;
        const eMesh& mesh = mi.getMesh();
        destMesh.merge(mesh, mtx, rotMtx);
    }
}

void eLSystem::drawInternal(eSceneData& destsg, eMesh& destMesh, tState* state) {
	ePROFILER_ZONE("L-System - Draw Internal Pre");

    this->m_gen_materials_dsIdx.clear();
    for(eU32 i = 0 ; i < this->m_gen_materials.size(); i++)
        this->m_gen_materials_dsIdx.append(destMesh.addDrawSection(this->m_gen_materials[i], eMesh::DrawSection::TYPE_TRIANGLES));

	eQuat originRot = state->curBaseRotation;
	{
		eArray<tProdSymbol>& curProd = m_prodBuffer[0];
		curProd.clear();

		eArray<tSymbol>& production = productions[0];
		for(eU32 s = 0; s < production.size(); s++) {
			tSymbol& curSymbol = production[s];
			tProdSymbol& ps = curProd.push();
			ps.correctRot = originRot;
		}
	}

	eF32 accumulatedStepLen = 0.0f;
	eU32 accumulatedStepCount = 0;
	drawStackPos = 0;
	tDrawState* drawState = getDefaultDrawState();
	drawState->lastTurtle = state->turtle;

    for(eU32 p = 1; p < numProductions; p++) {
		this->pushState(&state);
	
		eBool isLastProduction = p == numProductions - 1;

		state->curBaseRotation = eQuat();

        eU32 polyScopeCnt = 0;
		eArray<tSymbol>& prevProduction = productions[p - 1];
		eArray<tSymbol>& production = productions[p];

		eArray<tProdSymbol>& prevProd = m_prodBuffer[(1-p % 2)];
		eArray<tProdSymbol>& curProd = m_prodBuffer[p % 2];

		curProd.clear();
		curProd.reserve(production.size());

		for(eU32 s = 0; s < production.size(); s++) {
			tSymbol& curSymbol = production[s];
			const tSymbol& parentSymbol = prevProduction[curSymbol.parentIdx];
				
			tProdSymbol& curProdSym = curProd.push();
			const tProdSymbol& parentProdSym = prevProd[curSymbol.parentIdx];

			const eS32 symbolRaw = curSymbol.symbol;
				
			switch(curSymbol.symbol) {
				case '^':	state->turtle.width *= 2.0f; break;
				case '&':	state->turtle.width *= 0.5f; break;
//                case '!':	state->turtle.width = curSymbol.params[0] * this->m_baseWidth; break;
				case '\'':	if(state->turtle.polyMatIdx < this->m_gen_materials.size() - 1) state->turtle.polyMatIdx++; break;
				case '[':	pushState(&state); break;
				case ']':	popState(&state); break;
				case '{':	polyScopeCnt++; break;
				case '}':	polyScopeCnt--; break;
				case 'F':
				case 'G': {
						eQuat currentRot = parentProdSym.correctRot * curSymbol.rotation;
						eQuat currentGlobalRot = state->curBaseRotation * currentRot;
						this->applyForce(curSymbol, currentGlobalRot, state->curBaseRotation, state->turtle.position);

						state->turtle.rotation = (state->curBaseRotation * currentRot);
						eF32 amount = (state->turtle.size * state->turtle.height * curSymbol.params[0]);
						state->turtle.texPos += curSymbol.texVec * (this->m_gen_texScale * amount);
						state->turtle.position += state->turtle.rotation.getVector(2) * amount;
					}
				default:
					state->turtle.size *= m_sizeDecayPar;
					curProdSym.correctRot = state->curBaseRotation * parentProdSym.correctRot;
//                    {
//	                ePROFILER_ZONE("L-System - Draw Internal Calc");
					state->turtle.rotation = curProdSym.correctRot * curSymbol.rotation;
//                    }
			}

			// only draw last production
			if(!isLastProduction)
				continue;
				
			ePROFILER_ZONE("L-System - DrawInterpret");

			switch(curSymbol.symbol) {

			case '%': { // skip remaining
					eU32 scopeCnt = 1;
					while(((++s) < (eS32)production.size()) && (scopeCnt > 0))
						if(production[s].symbol == '[')			scopeCnt++;
						else if(production[s].symbol == ']')	scopeCnt--;
					s -= 2;
				}
				continue;
			case '[': this->pushDrawState(&drawState); break;
			case ']': this->popDrawState(&drawState); break;
			case '{': 
						if(polyStackPos >= polygonStack.size())
							polygonStack.append(eArray<eInt>()); 
						polygon = &polygonStack[polyStackPos++];
						polygon->clear(); 
					break;
			case '}': {
					if((state->turtle.polyMatIdx >= 0) && (state->turtle.polyMatIdx < m_gen_materials.size())) {
						for(eU32 p = 0; (eS32)p < (eS32)polygon->size() - 2; p++) {
							// draw polys
							destMesh.addTriangleFast((*polygon)[p], 
								                 (*polygon)[p+1], 
												 (*polygon)[polygon->size() - 1],
												 m_gen_materials_dsIdx[state->turtle.polyMatIdx]);
						}
					}

					// pop stack
                    polyStackPos--;
                    if(polyStackPos > 0)
					    polygon = &polygonStack[polyStackPos - 1];
				}
				break;
			case 'F':
			case '.': if(polyStackPos > 0) {
							polygon->append(destMesh.addVertex(state->turtle.position, state->turtle.rotation.getVector(2), state->turtle.texPos)); 
                      }
				// no break
			default:
				if((symbolRaw >= LSYS_VAR_MIN) && (symbolRaw <= LSYS_VAR_MAX)) {
					if(this->m_subSystem[symbolRaw] != eNULL) {
//						ePROFILER_ZONE("L-System - Draw Sub LSys");
						this->m_subSystem[symbolRaw]->processSub(destsg, destMesh, state->turtle);
					} else if(this->m_subMesh[symbolRaw] != eNULL) {
						ePROFILER_ZONE("L-System - Draw Sub Model");

						eMatrix4x4 mtx;
						mtx.scale(state->turtle.size);
						eQuat con = state->turtle.rotation;
						con.conjugate();
                        eMatrix4x4 rotMtx((eQuat(eVector3(1,0,0), -ePI * 0.5f) * (con)));
						mtx = mtx * rotMtx;
						mtx.translate(eVector3(state->turtle.position.x, state->turtle.position.y, state->turtle.position.z));

                        if(m_subMeshInstancing[symbolRaw])
                            destsg.merge(*this->m_subMesh[symbolRaw], mtx);
                        else
                            for(eU32 i = 0; i < m_subMesh[symbolRaw]->getEntryCount(); i++) 
                                _mergeIntoMesh(destMesh, mtx, rotMtx, m_subMesh[symbolRaw]->getEntry(i));
//                                _mergeIntoMesh(destMesh, mtx, mtx, m_subMesh[symbolRaw]->getEntry(i));
					} else {
						bool skipDraw = false;
						if((polyStackPos == 0) || (polygon->size() == 0)) {
							// draw aggregated shapes
							if((symbolRaw != 'F') || (state->turtle.polyMatIdx < 0) || (state->turtle.polyMatIdx >= m_gen_materials.size()))
							{
							} else {

								eBool forceDraw = (s >= production.size() - 1) ||
									(production[s + 1].symbol != curSymbol.symbol);
								eF32 stepLen = state->turtle.size * state->turtle.height;
								skipDraw = !this->drawShapes(destMesh, *drawState, drawState->lastTurtle, state->turtle, accumulatedStepLen + stepLen, drawState->texYOffset, drawState->texYOffset + m_gen_texScale, forceDraw, accumulatedStepCount + 1);
								if(skipDraw) {
									accumulatedStepCount++;
									accumulatedStepLen += stepLen;
								} else {
									accumulatedStepCount = 0;
									accumulatedStepLen = 0.0f;
								}
							}
						}

						drawState->texYOffset += m_gen_texScale;
						if(!skipDraw)
							drawState->lastTurtle = state->turtle;
					}
				}
			}
		}
		this->popState(&state);
	} 

}


void eLSystem::applyForce(tSymbol& symbol, const eQuat& curRot, eQuat& targetRot, eVector3& targetPos) {
    ePROFILER_ZONE("L-System - Apply Force");
	for(eS32 a = -1; a < (eS32)m_forces.size(); a++) {

        eF32 attractAmount = m_gravity;
        eU32 axis = 2;
        eVector3 attractionVec = eVector3(0,-1,0);

        if(a != -1) {
    		const tForceGenerator& attractor = m_forces[a];
            eVector3 attractorPos = (*attractor.triDefs)[0];
            attractAmount = attractor.mass;
            axis = attractor.attractAxis;
/*
#ifdef eEDITOR
            if(attractor.gen_type == FG_CLOSEST_FACE) {
                eU32     bestFaceOffset = 0;
                eVector3 bestFaceStats(eF32_MAX, 0, 0);
                for(eU32 i = 0; i < attractor.triDefs->size(); i += 3) {
                    const eVector3& B = (*attractor.triDefs)[i];
                    const eVector3& E0 = (*attractor.triDefs)[i+1];
                    const eVector3& E1 = (*attractor.triDefs)[i+2];
                    eVector3 distTest = targetPos.distanceToTriangle(B, E0, E1);
                    if(distTest.x < bestFaceStats.x) {
                        bestFaceStats = distTest;
                        bestFaceOffset = i;
                    }
                }
                // calculate attractor face
                const eVector3& B = (*attractor.triDefs)[bestFaceOffset];
                const eVector3& E0 = (*attractor.triDefs)[bestFaceOffset+1];
                const eVector3& E1 = (*attractor.triDefs)[bestFaceOffset+2];
                attractorPos = B + E0 * bestFaceStats.y + E1 * bestFaceStats.z;
                attractAmount = attractor.mass * (1.0f / bestFaceStats.x);
            }
#endif
*/
    		attractionVec = attractorPos - targetPos;
        }

		eVector3 look = curRot.getVector(axis);
		eVector3 side = look^attractionVec;

		eF32 cosAngleAttrLen = look * attractionVec;
		eF32 cosAngleSqr = (cosAngleAttrLen * cosAngleAttrLen / attractionVec.sqrLength());
        eF32 sinAngleSqr = 1.0f - cosAngleSqr;
		eF32 amount = (1.0f - (1.0f / (1.0f + attractAmount)));

		// |side| = sin(alpha)
		eF32 sideLen = side.length();
		if(sideLen > eALMOST_ZERO) {
			side /= sideLen;
			eQuat rotation(side, ePI * 0.5f * sinAngleSqr * amount);
			targetRot = rotation * targetRot;
		}
	}						

}


void _addMeshNeeds(eU32 &neededVerts, eU32& neededFaces, const eSceneData::Entry& e) {
    if(e.renderableList != eNULL) {
        for(eU32 i = 0; i < e.renderableList->getEntryCount(); i++)
            _addMeshNeeds(neededVerts, neededFaces, e.renderableList->getEntry(i));
    } else if(e.renderableObject->getType() == eIRenderable::TYPE_MESH) {
        const eMesh::Instance& mi = (const eMesh::Instance&)*e.renderableObject;
        const eMesh& mesh = mi.getMesh();
        neededVerts += mesh.getVertexCount();
        neededFaces += mesh.getPrimitiveCount();
    }
}


void eLSystem::addGeometryNeeds(eU32 &neededVerts, eU32& neededFaces) {
//	ePROFILER_ZONE("L-System - Calc Geometry Needs");
    neededVerts += m_gen_edges; // first ring
    eArray<eU32> polyStack;
	eArray<tSymbol>& production = this->productions[this->numProductions-1];
	for(int s = 0; s < (eS32)production.size(); s++) {
		tSymbol& curSymbol = production[s];
		eS32 symbol = production[s].symbol;
		if((symbol >= LSYS_VAR_MIN) && (symbol <= LSYS_VAR_MAX)) {
            if(m_subSystem[symbol] != eNULL)
				m_subSystem[symbol]->addGeometryNeeds(neededVerts, neededFaces);
            else if(m_subMesh[symbol] != eNULL) {
                for(eU32 i = 0; i < m_subMesh[symbol]->getEntryCount(); i++)
                    _addMeshNeeds(neededVerts, neededFaces, m_subMesh[symbol]->getEntry(i));
			} else {
				if(symbol == 'F') 
                    if(polyStack.size() != 0) {
                        polyStack.lastElement()++;
                    }
                    else {
					    neededVerts += m_gen_rings * m_gen_edges;
					    neededFaces += 2 * m_gen_rings * m_gen_edges;
				    }
            }
		}
        if(symbol == '{')
            polyStack.append(0);
		if(symbol == '.') {
            polyStack.lastElement()++;
        }
        if(symbol == '}') {
            eU32 polyVertCnt = polyStack.pop();
			neededVerts += polyVertCnt;
			neededFaces += polyVertCnt - 2;
		}

	}
}

void eLSystem::pushState(tState** state) {
//		ePROFILER_ZONE("L-System - Push State");
#ifdef eEDITOR
	if(stackPos >= LSYS_MAX_STACK)
		MessageBox(0,"ERROR: Stack maximum exceeded",0,0);
#endif
	tState* lastState = (*state);
	stackPos++;
	(*state) = &stack[stackPos];
	*(*state) = *lastState;
	(*state)->parentState = lastState;
}

void eLSystem::popState(tState** state) {
	stackPos--;
	(*state) = (*state)->parentState;
}

void eLSystem::pushDrawState(tDrawState** state) {
#ifdef eEDITOR
	if(stackPos >= LSYS_MAX_STACK)
		MessageBox(0,"ERROR: Stack maximum exceeded",0,0);
#endif
	tDrawState* lastState = (*state);
	drawStackPos++;
	(*state) = &drawStack[drawStackPos];
	*(*state) = *lastState;
	(*state)->parentState = lastState;
	(*state)->curVertices = &verticeStack[drawStackPos];
	(*state)->curVertices->clear();
	(*state)->curTempVertices = &verticeTempStack[drawStackPos];
	(*state)->curTempVertices->clear();
}

void eLSystem::popDrawState(tDrawState** state) {
	drawStackPos--;
	(*state) = (*state)->parentState;
}

// returns whether the shape was drawn
eBool eLSystem::drawShapes(eMesh& destMesh, tDrawState& state, const tTurtleState& turtle0, const tTurtleState& turtle1, eF32 shapeLen, eF32 stexY0, eF32 stexY1, eBool forceDraw, eU32 numParts) {
    eF32 partLen = eLerp(this->m_sizePar * (eF32)numParts, 0.0001f, detail);
//	eF32 partLen = eLerp(eF32_MAX, 0.0001f, detail);
	if(partLen <= 0.0f)
		partLen = eALMOST_ZERO;
	eF32 numToDrawF = (eF32)shapeLen / partLen;
	if(!forceDraw) {
		if(numToDrawF <= 1.0f)
			return false;
	}
	eU32 numDraw = eCeil(eClamp(1.0f, numToDrawF, (eF32)m_gen_rings)); 


	eU32		numFaces = numDraw * m_gen_edges * 2;
	eU32		faceNr = 0;

    ePROFILER_ZONE("L-System - Draw Shapes");

    __declspec(align(16)) const eVector3  control0 = turtle0.position;
	__declspec(align(16)) const eVector3  control1 = control0   + turtle0.rotation.getVector(2) * 0.333333f * shapeLen;
    __declspec(align(16)) const eVector3  control3 = turtle1.position;
	__declspec(align(16)) const eVector3  control2 = control3   - turtle1.rotation.getVector(2) * 0.333333f * shapeLen;

    eF32 rscale0 = turtle0.size * turtle0.width;
	eF32 rscale1 = turtle1.size * turtle1.width;
	for(eU32 d = 0; d < numDraw; d++) {
		eF32 t0 = ((eF32)d / (eF32)numDraw);
		eF32 t1 = ((eF32)(d + 1) / (eF32)numDraw);
		for(eU32 r = 0; r <= 1; r++) {
			eF32 tt = (r == 0) ? t0 : t1;
			eF32 rscale = eLerp(rscale0, rscale1, tt);
			if((r != 0) || (state.lastVertices->size() == 0)) {
                // create ring vertices
                __declspec(align(16)) eVector3 position;
                __declspec(align(16)) eVector3 normal;

                // calculate bezier curve position
                __m128 mt = _mm_set1_ps(tt);
                __m128 mtinv = _mm_set1_ps(1.0f - tt);
                __m128 mcp0 = _mm_load_ps(&control0.x);
                __m128 mcp1 = _mm_load_ps(&control1.x);
                __m128 m0 = _mm_add_ps(_mm_mul_ps(mcp0, mtinv), _mm_mul_ps(mcp1, mt));
                __m128 mcp2 = _mm_load_ps(&control2.x);
                __m128 m1 = _mm_add_ps(_mm_mul_ps(mcp1, mtinv), _mm_mul_ps(mcp2, mt));
                __m128 mm0 = _mm_add_ps(_mm_mul_ps(m0, mtinv), _mm_mul_ps(m1, mt));
                __m128 mcp3 = _mm_load_ps(&control3.x);
                __m128 m2 = _mm_add_ps(_mm_mul_ps(mcp2, mtinv), _mm_mul_ps(mcp3, mt));
                __m128 mm1 = _mm_add_ps(_mm_mul_ps(m1, mtinv), _mm_mul_ps(m2, mt));
                __m128 bezCurvePosition = _mm_add_ps(_mm_mul_ps(mm0, mtinv), _mm_mul_ps(mm1, mt));

                // calculate bezier tangent
                __m128 vec3mask = _mm_set_ps(0x0,0xFFFFFFFF,0xFFFFFFFF, 0xFFFFFFFF);
                __m128 mrestangent = _mm_and_ps(_mm_sub_ps(mm1, mm0), vec3mask);

                __m128 mdot = _mm_mul_ps(mrestangent, mrestangent);
                __m128 mdotagg = _mm_hadd_ps(mdot, mdot);
                __m128 recipsqrt = _mm_rsqrt_ss( _mm_hadd_ps(mdotagg, mdotagg) );
                __m128 tangentnorm = _mm_mul_ps(mrestangent, _mm_shuffle_ps(recipsqrt, recipsqrt, _MM_SHUFFLE(0,0,0,0)));


                // get look vector on axis 2 (ringRot.getVector(2))
                eQuat       ringRot = turtle0.rotation.slerp(tt, turtle1.rotation);
                __m128 mRingRot = _mm_loadu_ps((eF32*)&ringRot);
                __m128 rrmulparts = _mm_mul_ps(mRingRot, _mm_shuffle_ps(mRingRot, mRingRot, _MM_SHUFFLE(0,1,3,2)));

                __m128 ringRotSqr = _mm_mul_ps(mRingRot, mRingRot);
                __m128 mrdotagg = _mm_hadd_ps(rrmulparts, ringRotSqr);
                __m128 mrdotaggshuf = _mm_shuffle_ps(mrdotagg, mrdotagg, _MM_SHUFFLE(0,2,0,2));
                __m128 mrrotz = _mm_hsub_ps(rrmulparts, mrdotaggshuf);
                __m128 rrecipsqrt = _mm_rsqrt_ss( _mm_hadd_ps(mrdotagg, mrdotagg) );

                __m128 maxisparts = _mm_shuffle_ps(mrrotz,mrdotagg, _MM_SHUFFLE(0,2,0,2)); // -Y-X
                __m128 maxisparts2 = _mm_add_ps(maxisparts, maxisparts); // -Y*2-X*2
                __m128 maxispartsfinal = _mm_shuffle_ps(mrrotz,maxisparts2,_MM_SHUFFLE(0,0,2,0)); //ZZY*2X*2
                __m128 mlook = _mm_and_ps(_mm_mul_ps(maxispartsfinal, _mm_shuffle_ps(rrecipsqrt, rrecipsqrt, _MM_SHUFFLE(0,0,0,0))), vec3mask);

                // calculate side vector (look ^ tangent)
                __m128 mside = _mm_sub_ps(
                    _mm_mul_ps(_mm_shuffle_ps(mlook, mlook, _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(tangentnorm, tangentnorm, _MM_SHUFFLE(3, 1, 0, 2))),
                    _mm_mul_ps(_mm_shuffle_ps(mlook, mlook, _MM_SHUFFLE(3, 1, 0, 2)), _mm_shuffle_ps(tangentnorm, tangentnorm, _MM_SHUFFLE(3, 0, 2, 1)))
                  );

                // normalize side vector
                mdot = _mm_mul_ps(mside, mside);
                mdotagg = _mm_hadd_ps(mdot, mdot);
                __m128 dotsum = _mm_hadd_ps(mdotagg, mdotagg);
                const eF32 sideLenSqr = dotsum.m128_f32[0];
                if(sideLenSqr > eALMOST_ZERO) {
                    recipsqrt = _mm_rsqrt_ss( dotsum );
                    __m128 sidenorm = _mm_mul_ps(mside, _mm_shuffle_ps(recipsqrt, recipsqrt, _MM_SHUFFLE(0,0,0,0)));

                    // calc dot product (look * tangent)
                    __m128 dotprod = _mm_mul_ps(mlook, sidenorm);
                    __m128 dph0 = _mm_hadd_ps(dotprod, dotprod);
                    __m128 dph1 = _mm_hadd_ps(dph0, dph0);
                    const eF32 dot = eClamp(-1.0f, dph1.m128_f32[0], 1.0f);
		            eF32 alpha = eACos(dot) * (1.0f / (2.0f * ePI));

                    eQuat rotation(sidenorm, alpha);
		            ringRot = rotation * ringRot;
	            }


				eMatrix4x4 curveMat(ringRot);
                __declspec(align(16)) eVector3 ringX = curveMat.getVector(0);
                __declspec(align(16)) eVector3 ringY = curveMat.getVector(1);

				eF32 texY = eLerp(stexY0, stexY1, tt);
                const eF32 texXStep = 1.0f / m_gen_edges;
                eVector2 texPos(0, texY);
                
                __m128 mRingX = _mm_load_ps(&ringX.x);
                __m128 mRingY = _mm_load_ps(&ringY.x);
                __m128 mScale = _mm_set1_ps(rscale);

				for(eU32 e = 0; e <= m_gen_edges * 2; e += 2) {
                    __m128 msin = _mm_set1_ps(m_gen_edge_sinCosTable[e]);
                    __m128 mcos = _mm_set1_ps(m_gen_edge_sinCosTable[e+1]);
                    __m128 mnormal = _mm_add_ps(_mm_mul_ps(mRingX, msin), _mm_mul_ps(mRingY, mcos));
                    _mm_store_ps(&normal.x, mnormal);
                    __m128 mposition = _mm_add_ps(bezCurvePosition, _mm_mul_ps(mnormal, mScale));
                    _mm_store_ps(&position.x, mposition);

                    state.curVertices->append(destMesh.addVertex(position, normal, texPos));
                    texPos.x += texXStep;
                }

				// connect triangles
				if(r != 0) {
					eF32 texY0 = eLerp(stexY0, stexY1, t0);
					eF32 texY1 = eLerp(stexY0, stexY1, t1);
					for(eU32 e = 0; e < m_gen_edges; e++) {
						destMesh.addTriangleFast((*state.curVertices)[e], 
							                 (*state.curVertices)[e + 1], 
											 (*state.lastVertices)[e + 1],
											 m_gen_materials_dsIdx[turtle0.polyMatIdx]);
						destMesh.addTriangleFast((*state.curVertices)[e], 
							                 (*state.lastVertices)[e + 1], 
											 (*state.lastVertices)[e],
											 m_gen_materials_dsIdx[turtle0.polyMatIdx]);
					}
				}

				state.lastVertices = state.curVertices;
				eSwap(state.curVertices, state.curTempVertices);
				state.curVertices->clear();
			}
		}
	}
	return true;
}
