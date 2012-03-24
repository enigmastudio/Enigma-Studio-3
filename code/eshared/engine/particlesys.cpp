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
#include <windows.h>

eParticleSystem::Instance::Instance(eParticleSystem &psys) : eIRenderable(),
    m_psys(psys)
{
    m_mat.setZBuffer(eTRUE);
    m_mat.setZMask(eFALSE);
    m_mat.setBlending(eBLEND_SRCALPHA, eBLEND_ONE, eBLENDOP_ADD);
    m_mat.setUseBlending(eTRUE);
    m_mat.setCullingMode(eCULLING_BACK);
}

void eParticleSystem::Instance::update(eF32 time)
{
    eASSERT(time >= 0.0f);

    m_psys.update(time);
}

void eParticleSystem::Instance::getRenderJobs(const eMatrix4x4 &mtx, const eMatrix4x4 &normalMtx, eRenderJobPtrArray &jobs, eU32 passID) const
{
    if (m_psys.m_count > 0)
    {
        m_mat.setBlending(m_psys.m_blendSrc, m_psys.m_blendDst, m_psys.m_blendOp);
        m_mat.setTexture(eMaterial::UNIT_DIFFUSE, m_psys.m_tex);

        eRenderJob *job = eRenderJob::newRenderJob();
        job->set(m_psys.m_geometry, &m_mat, passID, mtx, normalMtx, eFALSE, getType());
        eASSERT(job != eNULL);
        job->setCastsShadows(eFALSE);
        jobs.append(job);
    }
}

const eAABB & eParticleSystem::Instance::getBoundingBox() const
{
    return m_psys.m_boundingBox;
}

eIRenderable::Type eParticleSystem::Instance::getType() const
{
    return TYPE_PARTICLE_SYSTEM;
}

eParticleSystem::eParticleSystem() :
    m_gfx(eNULL),
    m_geometry(eNULL),
    m_lastTime(0.0f),
    m_timer(-1.0f),
    m_emitTime(0.0f),
    m_count(0),
    m_tex(eNULL),
    m_emissionFreq(100.0f),
    m_emissionVel(1.0f),
    m_lifeTime(10.0f),
    m_stretchAmount(0.0f),
    m_gravity(0.0f),
	m_blendSrc(eBLEND_SRCALPHA),
    m_blendDst(eBLEND_ONE),
    m_blendOp(eBLENDOP_ADD),
    m_randomization(0)
{
	for(eU32 i = 0; i < __PATHSAMPLERSIZE__; i++)
		m_pathSampler[i] = eNULL;
	this->m_gravityConst = (eVector3*)eMemAllocAlignedAndZero(4 * sizeof(eF32), 16);
}

eParticleSystem::~eParticleSystem()
{
	for(eU32 i = 0; i < __PATHSAMPLERSIZE__; i++)
	    eSAFE_DELETE(m_pathSampler[i]);
//    eSAFE_DELETE(m_emitterMesh);
    eSAFE_DELETE(m_geometry);
    eFreeAligned(m_gravityConst);
}

void					
eParticleSystem::init(eGraphicsApiDx9 * gfx) {
	if((!this->m_gfx) && (gfx)) {
		m_gfx = gfx;
		m_particles.resize(PSYS_MAX_PARTICLES);
		m_geometry = new eGeometry(PSYS_MAX_PARTICLES*4, PSYS_MAX_PARTICLES*6, PSYS_MAX_PARTICLES*2, eVTXTYPE_PARTICLE, eGeometry::TYPE_DYNAMIC_INDEXED, ePRIMTYPE_TRIANGLELIST, _fillDynamicBuffers, this);
		eASSERT(m_geometry != eNULL);
	}
}

// Assumes time in seconds.
void eParticleSystem::update(eF32 time)
{
    if (time == m_lastTime)
        return;

	eU32 seed = eRandomSeed();

    ePROFILER_ZONE("Update particle system");
    const eF32 deltaTime = eMax(0.0f, time-m_lastTime);
    m_lastTime = time;
    time = deltaTime;
    m_boundingBox.clear();
    eF32 timeLeft = time > 1.0f ? 1.0f : time;
    while (timeLeft > 0.0f) {
        if (timeLeft <= PSYS_MAX_TIME_STEP) {
            time = timeLeft;
            timeLeft = 0.0f;
        } else {
            timeLeft -= PSYS_MAX_TIME_STEP;
            time = PSYS_MAX_TIME_STEP;
        }

        const eF32 timeNow = m_timer+time;

        // Add life and delete unused particles.
		eS32 low = 0;
		eS32 high = m_count - 1;
		bool upwards = true;
		while(high >= low) {
            Particle &p = upwards ? m_particles[low] : m_particles[high];
            p.timeToLive -= time*p.timeConstant;
			if(upwards) {
				if(p.timeToLive >= 0.0f)
					low++;
				else
					upwards = false;
			} else {
				if(p.timeToLive >= 0.0f) {
					m_particles[low++] = p;
					upwards = true;
				}
				high--;
			}
		}
		m_count = high + 1;

        // Calculate movement of particles.
        _moveParticles(&m_particles[0], m_count, m_timer, time);

        // Emit new particles.
        if (m_emissionFreq > 0.0f) {
            eF32 emitDelay = 1.0f/m_emissionFreq;
            eF32 relTime = m_timer;
            m_emitTime += time;
            while (m_emitTime >= emitDelay) {
                m_emitTime -= emitDelay;
                eF32 timeRemaining = m_emitTime;
                // Emit new particles.
                if (m_count < PSYS_MAX_PARTICLES) {
                    Particle &p = m_particles[m_count];
                    p.rotation = 0;
					p.size = 1.0f * (1.0f - eRandomF(seed) * this->m_randomization);
					p.mass = 1.0f * (1.0f - eRandomF(seed) * this->m_randomization);
					eF32 initVel = m_emissionVel * (1.0f - eRandomF(seed) * this->m_randomization);
                    if (m_emitterMesh == eNULL || m_emitterEntities.size() == 0) {
                        p.dynamicEntity.position.null();
                        p.dynamicEntity.velocity = eVector3(eRandomF(-1.0f, 1.0f, seed), 1.0f, eRandomF(-1.0f, 1.0f, seed))*initVel;
                    } else {
                        // lookup random entity with binary search
                        eF32 a = eRandomF(seed) * m_emitterEmitSurfaceArea;
                        eU32 l = 0;
                        eU32 r = m_emitterEntities.size();
                        while (l < r) {
                            eU32 m = (l + r) >> 1;
                            if (a < m_emitterEntities[m])   r = m;
                            else                            l = m+1;
                        }
                        eU32 f = eClamp((eU32)0, r , m_emitterEntities.size()-1);
                        eF32 w0 = eRandomF(seed);
						eF32 w0Inv = 1.0f - w0;
                        switch (m_emitterMode) {
                            case EMITTERMODE_FACES: {
                                const eEditMesh::Face *tri = m_emitterMesh->getFace(f);
                                eASSERT(tri != eNULL);
								tri->getRandomSurfacePoint(p.dynamicEntity.position, p.dynamicEntity.velocity);
                                break;
                            }
                            case EMITTERMODE_EDGES: {
                                const eEditMesh::Edge *edge = m_emitterMesh->getEdge(f);
                                eASSERT(edge != eNULL);
								const eEditMesh::HalfEdge *he = edge->he0;
                                if (he->face == eNULL)
                                    he = edge->he1;
                                const eEditMesh::Vertex *v0 = he->origin;
                                const eEditMesh::Vertex *v1 = he->next->origin;
								p.dynamicEntity.position = v0->position.lerp(w0, v1->position);
								p.dynamicEntity.velocity = v0->normal.lerp(w0, v1->normal);
                                break;
                            }
                            case EMITTERMODE_VERTICES: {
                                const eEditMesh::Vertex *vtx = m_emitterMesh->getVertex(f);
                                eASSERT(vtx != eNULL);
                                p.dynamicEntity.position = vtx->position;
                                p.dynamicEntity.velocity = vtx->normal;
                                break;
                            }
                        }
						p.dynamicEntity.velocity.normalize();
						p.dynamicEntity.velocity *= initVel;
                    }
                        
					const eF32 lifeTime = this->m_lifeTime * (1.0f - eRandomF(seed) * this->m_randomization);
                    p.timeConstant = (lifeTime <= 0.0f) ? 1.0f : 1.0f / lifeTime;
                    p.timeToLive = 1.0f - timeRemaining * p.timeConstant;
                    _moveParticles(&p, 1, timeNow - timeRemaining, timeRemaining);
                    m_count++;
                }
            }
        }
        m_timer = timeNow;
    }
}


void eParticleSystem::setEmitter(const eEditMesh *mesh, EmitterMode mode) {
    m_emitterMode = mode;
    if (mesh) {
		m_emitterMesh = mesh;
        eASSERT(m_emitterMesh != eNULL);
        m_emitterEmitSurfaceArea = 0.0f;
        switch (mode) {
            case EMITTERMODE_FACES: {
                m_emitterEntities.resize(m_emitterMesh->getFaceCount());
                for (eU32 i=0; i<m_emitterMesh->getFaceCount(); i++) {
                    // Calc face area.
                    m_emitterEmitSurfaceArea += m_emitterMesh->getFace(i)->getArea();
                    // Add to sum.
                    m_emitterEntities[i] = m_emitterEmitSurfaceArea;
                }
                break;
            }

            case EMITTERMODE_EDGES: {
                m_emitterEntities.resize(m_emitterMesh->getEdgeCount());
                for (eU32 i=0; i<m_emitterMesh->getEdgeCount(); i++) {
                    // Calc edge area.
                    const eEditMesh::Edge *edge = m_emitterMesh->getEdge(i);
                    eASSERT(edge != eNULL);

                    eEditMesh::HalfEdge *he = edge->he0;
                    if (he->face == eNULL)
                        he = edge->he1;

					const eF32 area = (he->origin->position - he->next->origin->position).length();

                    // Add to sum.
                    m_emitterEmitSurfaceArea += area;
                    m_emitterEntities[i] = m_emitterEmitSurfaceArea;
                }
                break;
            }

            case EMITTERMODE_VERTICES: {
                m_emitterEntities.resize(m_emitterMesh->getVertexCount());
                for (eU32 i=0; i<m_emitterMesh->getVertexCount(); i++) {
                    m_emitterEmitSurfaceArea += 1.0f;
                    m_emitterEntities[i] = m_emitterEmitSurfaceArea;
                }
                break;
            }
        }
    }
}

void eParticleSystem::_moveParticles(Particle *particles, eU32 count, eF32 nowTime, eF32 deltaTime)
{
    ePROFILER_ZONE("Move particles");

	const eF32 dtHalf = 0.5f * deltaTime;
	const eF32 dtdtHalf = dtHalf * deltaTime;
    for (eU32 i=0; i<count; i++) {
        // Time step of verlet integration.
        Particle &p = particles[i];

		__m128 pos = _mm_loadu_ps(p.dynamicEntity.position);
		const __m128 oldAccel = _calcAcceleration(p.mass, pos);

		__m128 vel = _mm_loadu_ps(p.dynamicEntity.velocity);
		pos = _mm_add_ps(pos, _mm_mul_ps(vel, _mm_set1_ps(deltaTime)));
		pos = _mm_add_ps(pos, _mm_mul_ps(oldAccel, _mm_set1_ps(dtdtHalf)));

		const __m128 newAccel = _calcAcceleration(p.mass, pos);
		vel = _mm_add_ps(vel, _mm_mul_ps(_mm_set1_ps(dtHalf), _mm_add_ps(newAccel, oldAccel)));

		_mm_storeu_ps(&p.dynamicEntity.position.x, pos);
		_mm_storeu_ps(&p.dynamicEntity.velocity.x, vel);

        m_boundingBox.updateExtent(p.dynamicEntity.position);
    }
}

void eParticleSystem::_fillDynamicBuffers(ePtr param, eGeometry *geo) {
    const eParticleSystem *psys = (eParticleSystem *)param;
    eASSERT(psys != eNULL);

    eVector3 right, up, view;
    geo->getGraphics()->getBillboardVectors(right, up, &view);
	eMatrix4x4 mat = geo->getGraphics()->getActiveModelMatrix();
	view = mat * view;
	right = mat * right;
	up = mat * up;

	view.normalize();
	right.normalize();
	up.normalize();

	eParticleVertex *vertices = eNULL;
    eU32 *indices = eNULL;
    eU32 idxCount=0, vtxCount=0, primCount=0;
    geo->startFilling((ePtr *)&vertices, &indices);
    {
		for (eU32 i=0; i<psys->m_count; i++) {
            const Particle &p = psys->m_particles[i];

            eF32 ptime = 1.0f-p.timeToLive;
			eColor col = eColor::WHITE;
            eF32 scale = psys->m_pathSampler[SIZE] == eNULL ? eSin(p.timeToLive * ePI) : 
                                                            *psys->m_pathSampler[SIZE]->evaluate(ptime);
			if (psys->m_pathSampler[COLOR]) {
				const eVector4 &res = psys->m_pathSampler[COLOR]->evaluate(ptime);
				col.setRedF(res.x);
				col.setGreenF(res.y);
				col.setBlueF(res.z);
				col.setAlphaF(res.x);
			} else 
				col.setAlphaF(eClamp(0.0f, eSin(p.timeToLive * ePI), 1.0f));
			scale *= p.size;
            eF32 rot = psys->m_pathSampler[ROTATION] == eNULL ? 0.0f : 
                                                         *psys->m_pathSampler[ROTATION]->evaluate(ptime);

            eVector3 r = right * scale;
            eVector3 u = up * scale;
            eVector3 pos2 = p.dynamicEntity.position;
            if(psys->m_stretchAmount != 0.0f) 
			{
                eVector3 flyDir = p.dynamicEntity.velocity;
				const eVector3 velNorm = p.dynamicEntity.velocity.normalized();
                const eF32 rightCos = view*velNorm;
                const eQuat qr(view, rot);
                const eQuat qr90(view, -eHALFPI);
                r = -((velNorm * qr90)*qr) * scale;
                u = (velNorm * qr) * scale;
                pos2 = p.dynamicEntity.position+p.dynamicEntity.velocity*psys->m_stretchAmount;
            } else 
				if(rot != 0) {
					r = (r * eQuat(view, rot));
					u = (u * eQuat(view, rot));
				}

			const eVector3 mid = (p.dynamicEntity.position + pos2) * 0.5f;

            vertices[vtxCount+0].set(pos2                     + u, eVector2(0.0f, 0.0f), col);
            vertices[vtxCount+1].set(mid                      + r, eVector2(1.0f, 0.0f), col);
            vertices[vtxCount+2].set(p.dynamicEntity.position - u, eVector2(1.0f, 1.0f), col);
            vertices[vtxCount+3].set(mid                      - r, eVector2(0.0f, 1.0f), col);

			indices[idxCount++] = vtxCount+0;
            indices[idxCount++] = vtxCount+1;
            indices[idxCount++] = vtxCount+2;
            indices[idxCount++] = vtxCount+0;
            indices[idxCount++] = vtxCount+2;
            indices[idxCount++] = vtxCount+3;

            vtxCount += 4;
            primCount += 2;
        }
    }
    geo->stopFilling(eTRUE, vtxCount, idxCount, primCount);
}