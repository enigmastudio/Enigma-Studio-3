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

eParticleSystem::eParticleSystem() :
    m_geometry(eNULL),
    m_lastTime(0.0f),
    m_timer(-1.0f),
    m_emitTime(0.0f),
    m_timeNowEmitter(0.0f),
    m_count(0),
    m_emitterLifeTime(0.0f),
    m_maxCount(0),
    m_tex(eNULL),
    m_integrator(INTEGRATOR_VERLET),
	m_particleSizePath(eNULL),
	m_particleColorPath(eNULL),
	m_emitterMesh(eNULL),
	m_stretchAmount(0),
	m_particleLifeTime(10),
	m_emitterAreaSums(0),
	m_gravity(0)
{
}

eParticleSystem::~eParticleSystem()
{
    eSAFE_DELETE(m_geometry);
}

// Assuming time in seconds.
void eParticleSystem::update(eF32 time)
{
    ePROFILER_DEFSCOPE(profUpdateParticleSys, "Update particle system");

	this->m_boundingBox.clear();

    const eF32 deltaTime = eMax(0.0f, time-m_lastTime);

    m_lastTime = time;
    time = deltaTime;

    eF32 maxTimeStep = 1.0f/30.0f;
    eF32 timeLeft = time;

    if (timeLeft > 1.0f)
    {
        timeLeft = 1.0f;
    }

    while (timeLeft > 0.0f)
    {
        if (timeLeft <= maxTimeStep)
        {
            time = timeLeft;
            timeLeft = 0.0f;
        }
        else
        {
            timeLeft -= maxTimeStep;
            time = maxTimeStep;
        }

        const eF32 timeNow = m_timer+time;
        m_timeNowEmitter += time/m_emitterLifeTime;

        while (m_timeNowEmitter > 1.0f)
        {
            m_timeNowEmitter -= 1.0f;
        }

        // Add life and delete unused particles.
        Particle *newP = &m_particles[0];
        eU32 newCount = 0;

        for (eU32 i=0; i<m_count; i++)
        {
            Particle &p = m_particles[i];
            p.timeToLive -= time*p.timeConstant;

            if (p.timeToLive >= 0.0f)
            {
                *newP = p;
                newP++;
                newCount++;
            }
        }

        m_count = newCount;

        // Calculate movement of particles.
        _moveParticles(&m_particles[0], m_count, m_timer, time);

        // Emit new particles.
        eF32 emissionFreq = this->m_emissionFrequency;

        if (emissionFreq > 0.0f)
        {
            eF32 emitDelay = 1.0f/emissionFreq;
            eF32 relTime = m_timer;
            m_emitTime += time;

            while (m_emitTime >= emitDelay)
            {
                m_emitTime -= emitDelay;

                eF32 timeRemaining = m_emitTime;

                // Emit new particles.
                if (m_count < m_maxCount)
                {
                    Particle &p = m_particles[m_count];
                    eF32 a = eRandomF();
					eF32 s = 5.0;
                    eF32 initVel = this->m_emissionVelocity;
					if((this->m_emitterMesh == eNULL) || (this->m_emitterMesh->getFaceCount() == 0))
					{
						p.dynamicEntity.position = eVector3(0,0,0);
						p.dynamicEntity.velocity = eVector3(eRandomF() * 2.0 - 1.0,1,eRandomF() * 2.0 - 1.0) * initVel;
					} else {
						// lookup random face
						eF32 a = eRandomF() * this->m_emitterMeshArea;
						eU32 l = 0;
						eU32 r = this->m_emitterMesh->getFaceCount();
						while(l < r)
						{
							eU32 m = (l + r) >> 1;
							if(a < this->m_emitterAreaSums[m])
								r = m;
							else
								l = m+1;
						}
						eU32 f = r;
						if(f >= this->m_emitterMesh->getFaceCount())
							f = r - 1;

						eF32 w0 = eRandomF();
						eF32 w1 = eRandomF();
						if(w0 + w1 > 1.0f)
						{
							w0 = 1.0f - w0;
							w1 = 1.0f - w1;
						}
						eF32 w2 = 1.0f - w0 - w1;

						const eEditMesh::Face* tri = this->m_emitterMesh->getFace(f);
						eVector3 pos0 = tri->getHalfEdge(0)->origin->position;
						eVector3 pos1 = tri->getHalfEdge(1)->origin->position;
						eVector3 pos2 = tri->getHalfEdge(2)->origin->position;
						eVector3 norm0 = tri->getHalfEdge(0)->origin->normal;
						eVector3 norm1 =  tri->getHalfEdge(1)->origin->normal;
						eVector3 norm2 =  tri->getHalfEdge(2)->origin->normal;

						eVector3 res = ((pos0 * w0) + (pos1 * w1) + (pos2 * w2));
						eVector3 norm = ((norm0 * w0) + (norm1 * w1) + (norm2 * w2)) ;
						norm.normalize();
						p.dynamicEntity.position = res;
						p.dynamicEntity.velocity = norm * initVel;
/*
						int nr = eRandom(0, this->m_emitterMesh->getVertexCount());
						const eEditMesh::Vertex* vert = this->m_emitterMesh->getVertex(nr);
						p.dynamicEntity.position = vert->position;
						p.dynamicEntity.velocity = vert->normal * initVel;
*/
					}
                    p.color = eColor::WHITE;
//                    p.mass = eRandomF(1.0E0f, 1.0E4f); 

                    p.timeConstant = 1.0f/this->m_particleLifeTime;
                    p.timeToLive = 1.0f-timeRemaining*p.timeConstant;
                    _moveParticles(&p, 1, timeNow-timeRemaining, timeRemaining);

                    m_count++;
                }
            }
        }

        m_timer = timeNow;
    }
	this->m_boundingBox.updateExtentsFinalize();
}


void eParticleSystem::setGraphicsApi(eIGraphicsApi *gfx)
{
	this->m_gfx = gfx;
}

void eParticleSystem::setMaxParticleCount(eU32 maxCount)
{
    eSAFE_DELETE(m_geometry);

    m_particles.resize(maxCount);
    m_maxCount = maxCount;
	if(this->m_count > this->m_maxCount)
		this->m_count = this->m_maxCount;
    m_geometry = new eGeometry(m_gfx, maxCount*4, maxCount*6, maxCount*2, eVDECL_PARTICLE_VERTEX,
                               eGeometry::TYPE_DYNAMIC_INDEXED, ePRIMTYPE_TRIANGLELIST, _fillDynamicBuffers, this);
    eASSERT(m_geometry != eNULL);
}

void eParticleSystem::setTexture(eITexture2d *tex)
{
    m_tex = tex;
}

void eParticleSystem::setParticleSizePath(ePath *path)
{
	if(path != 0)
	{
		if(!this->m_particleSizePath)
			this->m_particleSizePath = new ePathSampler();
		this->m_particleSizePath->sample(path,0,1,100);
	}
	else
	{
		if(this->m_particleSizePath)
			delete this->m_particleSizePath;
		this->m_particleSizePath = eNULL;
	}
}

void eParticleSystem::setAttractorMesh(eEditMesh *mesh, eU32 detail)
{
/*
	this->m_attractorMesh = mesh;

	if(mesh == eNULL)
		this->m_approxSpace = eNULL;
	else
	{
		// generate sample points
		int vertCnt = mesh->getVertexCount();
		eF32* points = new eF32[vertCnt * 3];
		eF32* normals = new eF32[vertCnt * 3];
		int p = 0;
		int n = 0;
		for(int i = 0; i < vertCnt; i++)
		{
			eEditMesh::Vertex* vert = mesh->getVertex(i);
			points[p++] = vert->position.x;
			points[p++] = vert->position.y;
			points[p++] = vert->position.z;
			eVector3 normal = vert->normal.normalized();
			normals[n++] = normal.x;
			normals[n++] = normal.y;
			normals[n++] = normal.z;
		}
//		Approximation* app = new Approximation(2,3,1,vertCnt*2,points,vals);
		this->m_approxSpace = new Approximation(detail, vertCnt, points, normals);
		delete [] points;
		delete [] normals;
	}
*/
}

void					
eParticleSystem::setEmitter(eEditMesh *mesh)
{
	this->m_emitterMesh = mesh;
	if(this->m_emitterAreaSums)
		delete [] this->m_emitterAreaSums;
	this->m_emitterAreaSums = 0;
	if(this->m_emitterMesh)
	{
		this->m_emitterMesh->triangulate();
		int numFaces = this->m_emitterMesh->getFaceCount();
		this->m_emitterAreaSums = new eF32[numFaces];
		this->m_emitterMeshArea = 0;
		for(eU32 i = 0; i < numFaces; i++)
		{

			// Calc face area
			const eEditMesh::Face* tri = this->m_emitterMesh->getFace(i);
			const eVector3& P = tri->getHalfEdge(0)->origin->position;
            const eVector3& Q = tri->getHalfEdge(1)->origin->position;
            const eVector3& X = tri->getHalfEdge(2)->origin->position;
            eVector3 u = X - P;
            eF32 ul = u.length();
            eF32 t = ((Q - P) * u) / (ul * ul);
            eVector3 S = P + u * t;
            eF32 h = (S - Q).length();
            eF32 area = h * ul * 0.5f;

			// add to sum
			this->m_emitterMeshArea += area;
			this->m_emitterAreaSums[i] = this->m_emitterMeshArea;
		}
	}
}

void
eParticleSystem::setStretchAmount(eF32 amount)
{
	this->m_stretchAmount = amount;
}

Approximation* eParticleSystem::getApproximation()
{
	return this->m_approxSpace;
}


void eParticleSystem::setParticleColorPath(ePath *path)
{
	if(path != 0)
	{
		if(!this->m_particleColorPath)
			this->m_particleColorPath = new ePathSampler();
		this->m_particleColorPath->sample(path,0,1,100);
	}
	else
	{
		if(this->m_particleColorPath)
			delete this->m_particleColorPath;
		this->m_particleColorPath = eNULL;
	}
}

void
eParticleSystem::setFadeOut(eF32 amount)
{
	this->m_fadeOutAmount = amount;
}


void eParticleSystem::setEmitterLifeTime(eF32 emitterLifeTime)
{
    eASSERT(emitterLifeTime > 0.0f);
    m_emitterLifeTime = emitterLifeTime;
}

void eParticleSystem::setEmissionFrequency(const eF32 val)
{
	this->m_emissionFrequency = val;
}

void eParticleSystem::setEmissionVelocity(const eF32 val)
{
	this->m_emissionVelocity = val;
}

void
eParticleSystem::setParticleLifeTime(eF32 amount)
{
	this->m_particleLifeTime = amount;
}

void
eParticleSystem::setGravity(eF32 value)
{
	this->m_gravity = value;
}


void eParticleSystem::setIntegrator(Integrator integrator)
{
    m_integrator = integrator;
}

eU32 eParticleSystem::getMaxParticleCount() const
{
    return m_maxCount;
}

eU32 eParticleSystem::getParticleCount() const
{
    return m_count;
}

const eParticleSystem::Particle & eParticleSystem::getParticle(eU32 index) const
{
    return m_particles[index];
}

eAABB eParticleSystem::getBoundingBox() const
{
	return this->m_boundingBox;
}


eITexture2d * eParticleSystem::getTexture() const
{
    return m_tex;
}

eGeometry * eParticleSystem::getGeometry() const
{
    return m_geometry;
}

static eVector3 GetGravitationalForce(eF32 time, const eVector3 &pos0, eF32 mass0, const eVector3 &pos1, eF32 mass1)
{
    const eF32 GRAVITATIONAL_CONSTANT = 6.67259e-11f; // [m^3 * kg^-1 * s^-1]

    const eVector3 d = pos1-pos0;
    eF32 distSqr = d.sqrLength();

    if (eIsFloatZero(distSqr))
    {
        distSqr = eALMOST_ZERO;
    }

    const eF32 dist = eSqrt(distSqr);

    // Apply gravitational law.
    const eF32 gravForce = GRAVITATIONAL_CONSTANT*(mass1*mass0)/distSqr;
    return d*(gravForce/dist);
}

eVector3 eParticleSystem::_calcAcceleration(const DynamicEntity &state, eF32 mass, eF32 nowTime) const
{
    eVector3 forceVector(0,0,0);
    const eF32 GRAVITATIONAL_CONSTANT = 6.67259e-11f; // [m^3 * kg^-1 * s^-1]

	eF32 forceLimit = 10000.0;
	eVector3 norm;
/*
	if(this->m_approxSpace)
	{
		eF32 dist = this->m_approxSpace->EvaluateFast3DimGrade2(&state.position, &norm);
		eF32 distSquared = dist * dist;
//		if(!eIsFloatZero(distSquared))
		if(distSquared != 0)
		{
			eF32 massAttr = 1.0E16;
			eF32 force = (GRAVITATIONAL_CONSTANT * (mass * massAttr) / (distSquared));
			if(force > forceLimit)
				force = forceLimit;
			if(dist >= 0)
				forceVector += -norm * force;
			else
				forceVector += norm * force;
		}
	}
*/
	// add friction
//	forceVector -= state.velocity * 1.0E04;
	forceVector -= eVector3(0,this->m_gravity,0);
    return forceVector; // acc = force / mass
}

eParticleSystem::DynamicEntity eParticleSystem::_calcDerivation(const eF32 time, const DynamicEntity& state, const eF32 mass) const
{
    DynamicEntity result;
    result.position = state.velocity;
    result.velocity = _calcAcceleration(state,mass,time);
    return result;
}

void eParticleSystem::_moveParticles(Particle* particles, eU32 howMany, eF32 nowTime, eF32 deltaTime)
{
    ePROFILER_DEFSCOPE(profMoveParticles, "Move particles");

    if (m_integrator == INTEGRATOR_VERLET)
    {
        for (eU32 i=0; i<howMany; i++)
        {
            // Time step of verlet integration.
            Particle &p = particles[i];

            const eVector3 oldAccel = _calcAcceleration(p.dynamicEntity, p.mass, nowTime);
            p.dynamicEntity.position += p.dynamicEntity.velocity*deltaTime+oldAccel*(0.5f*deltaTime*deltaTime);
            const eVector3 newAccel = _calcAcceleration(p.dynamicEntity, p.mass, nowTime+deltaTime);
            p.dynamicEntity.velocity += (newAccel+oldAccel)*0.5f*deltaTime;

			this->m_boundingBox.updateExtentsFast(p.dynamicEntity.position);

            // Update color and size of particle.
			if(this->m_particleColorPath)
			{
				const eF32* res = this->m_particleColorPath->evaluate(1.0f - p.timeToLive);
				p.color.setRedF(res[0] / 255.0);
				p.color.setGreenF(res[1] / 255.0);
				p.color.setBlueF(res[2] / 255.0);
				p.color.setAlphaF(1.0);
			}
			p.curScale = 0.04;
			if(this->m_particleSizePath)
				p.curScale = *this->m_particleSizePath->evaluate(1.0f - p.timeToLive);
//				p.curScale = this->m_particleSizePath->process(p.timeToLive, this).pos.x;
        }
    } 
    else if (m_integrator == INTEGRATOR_RUNGEKUTTA)
    {
        for (eU32 i=0; i<howMany; i++)
        {
            // Time step of runge-kutta integration.
            Particle &p = particles[i];

            const eF32 t0 = nowTime;
            const eF32 t = nowTime+deltaTime;
            const eF32 h = t-t0;
            const eF32 halfH = h*0.5f;

            const DynamicEntity k1 = _calcDerivation(t0, p.dynamicEntity, p.mass);
            const DynamicEntity k2 = _calcDerivation(t0+halfH, p.dynamicEntity+k1*halfH, p.mass);
            const DynamicEntity k3 = _calcDerivation(t0+halfH, p.dynamicEntity+k2*halfH, p.mass);
            const DynamicEntity k4 = _calcDerivation(t0+h, p.dynamicEntity+k3*h, p.mass);

            p.dynamicEntity = p.dynamicEntity+(k1+(k2+k3)*2.0f+k4)*(h/6.0f);
			this->m_boundingBox.updateExtents(p.dynamicEntity.position);

            // Update color and size of particle.
            eF32 col[] = {1.0f, 1.0f, 1.0f, 1.0f};

			p.curScale = 0.04;
        }
    }
}

void eParticleSystem::_fillDynamicBuffers(ePtr param, eGeometry *geo)
{
    eASSERT(geo != eNULL);

    const eParticleSystem *psys = (eParticleSystem *)param;
    eASSERT(psys != eNULL);

    const eMatrix4x4 &m = geo->getGraphics()->getActiveModelViewMatrix();
//    const eVector3 r = eVector3(m.m11, m.m21, m.m31).normalized();
//    const eVector3 u = eVector3(m.m12, m.m22, m.m32).normalized();
    const eVector3 view = eVector3(m.m13, m.m23, m.m33).normalized();
//	const eVector3 u = view^p

    eParticleVertex *vertices = eNULL;
    eU32 *indices = eNULL;
    eU32 idxCount=0, vtxCount=0, primCount=0;
	
    geo->startFilling((ePtr *)&vertices, &indices);
    {
        for (eU32 i=0; i<psys->getParticleCount(); i++)
        {
			const Particle *p = &psys->m_particles[i];
			eVector3 vel = p->dynamicEntity.velocity;
			vel *= (1.0f / vel.length());
			eF32 cos = view * vel;
			const eVector3 r = (view^p->dynamicEntity.velocity).normalized();
			const eVector3 u = view^r;
            const eVector3 s = r*p->curScale;
            const eVector3 t = u*p->curScale;
			const eVector3 pos2 = p->dynamicEntity.position + p->dynamicEntity.velocity * psys->m_stretchAmount;
			eColor colPre = p->color;
			colPre.setAlphaF(colPre.alphaF() * (1.0f - psys->m_fadeOutAmount * cos*cos));
			eRGBA col = (colPre).toRgba();
			vertices[vtxCount+0].position = pos2-s-t;
            vertices[vtxCount+0].texCoord = eVector2(0.0f, 0.0f);
            vertices[vtxCount+0].color = col;

            vertices[vtxCount+1].position = pos2+s-t;
            vertices[vtxCount+1].texCoord = eVector2(1.0f, 0.0f);
            vertices[vtxCount+1].color = col;

            vertices[vtxCount+2].position = p->dynamicEntity.position+s+t;
            vertices[vtxCount+2].texCoord = eVector2(1.0f, 1.0f);
            vertices[vtxCount+2].color = col;

            vertices[vtxCount+3].position = p->dynamicEntity.position-s+t;
            vertices[vtxCount+3].texCoord = eVector2(0.0f, 1.0f);
            vertices[vtxCount+3].color = col;

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