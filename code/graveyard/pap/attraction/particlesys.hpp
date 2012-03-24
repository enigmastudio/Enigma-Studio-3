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

#ifndef PARTICLE_SYS_HPP
#define PARTICLE_SYS_HPP

class eParticleSystem
{
public:
    enum Integrator
    {
        INTEGRATOR_VERLET,      // fast, but error grows super-linear with deltaTime
        INTEGRATOR_RUNGEKUTTA   // slow, but error grows just linear with deltaTime
    };

public:
    struct DynamicEntity
    {
        eVector3        position;            // [m]
        eVector3        velocity;            // [m/s]

        DynamicEntity operator * (eF32 v) const
        {
            DynamicEntity result;
            result.position = position * v;
            result.velocity = velocity * v;
            return result;
        }

        DynamicEntity operator / (const DynamicEntity &other) const
        {
            DynamicEntity result;
            result.position = position;
            result.position.x /= other.position.x;
            result.position.y /= other.position.y;
            result.position.z /= other.position.z;
            result.velocity = velocity;
            result.velocity.x /= other.velocity.x;
            result.velocity.y /= other.velocity.y;
            result.velocity.z /= other.velocity.z;
            return result;
        }

        DynamicEntity operator + (const DynamicEntity& v) const
        {
            DynamicEntity result;
            result.position = position + v.position;
            result.velocity = velocity + v.velocity;
            return result;
        }

        DynamicEntity& operator *= (const eF32 v)
        {
            position *= v;
            velocity *= v;
            return *this;
        }

        DynamicEntity & operator += (const DynamicEntity& other)
        {
            position += other.position;
            velocity += other.velocity;
            return *this;
        }
    };

    struct Particle
    {
        DynamicEntity       dynamicEntity;
        eColor              color;
        eF32                mass;           // [kg]
        eF32                timeToLive;     // (0..1]
        eF32                timeConstant;   // 1 / ttl_max
		eF32			    curScale;
    };

    typedef eArray<Particle> ParticleArray;

public:
    eParticleSystem(/*eIGraphicsApi *gfx*/);
    ~eParticleSystem();

    void                    update(eF32 time);

    void                    setGraphicsApi(eIGraphicsApi *gfx);
    void                    setMaxParticleCount(eU32 count);
    void                    setTexture(eITexture2d *tex);
    void                    setEmitterLifeTime(eF32 emitterLifeTime);
    void                    setIntegrator(Integrator integrator);

    void                    setEmissionFrequency(const eF32 val);
    void                    setEmissionVelocity(const eF32 val);
	void					setParticleSizePath(ePath *path);
	void					setParticleColorPath(ePath *path);

    eU32                    getMaxParticleCount() const;
    eU32                    getParticleCount() const;
    const Particle &        getParticle(eU32 index) const;
    eITexture2d *           getTexture() const;
    eGeometry *             getGeometry() const;
		
	eAABB					getBoundingBox() const;

	void					setAttractorMesh(eEditMesh *mesh, eU32 detail);
	void					setEmitter(eEditMesh *mesh);
	void					setStretchAmount(eF32 amount);
	void					setParticleLifeTime(eF32 amount);
	void					setFadeOut(eF32 amount);
	void					setGravity(eF32 value);

	Approximation*			getApproximation();

private:
    eVector3                _calcAcceleration(const DynamicEntity &state, eF32 mass, eF32 nowTime) const;
    DynamicEntity           _calcDerivation(const eF32 time, const DynamicEntity &state, const eF32 mass) const;
    void                    _moveParticles(Particle* particles, eU32 howMany, eF32 nowTime, eF32 deltaTime);

private:
    static void             _fillDynamicBuffers(ePtr param, eGeometry *geo);
	eAABB					m_boundingBox;

public:

private:
    eIGraphicsApi *         m_gfx;
    eGeometry *             m_geometry;
    ParticleArray           m_particles;

	eF32                    m_lastTime;
    eF32                    m_timer;
    eF32                    m_emitTime;
    eF32                    m_timeNowEmitter;
    eU32                    m_count;

    eF32                    m_emitterLifeTime;
    eU32                    m_maxCount;
    eITexture2d *           m_tex;
    Integrator              m_integrator;

	eF32					m_emissionFrequency;
	eF32					m_emissionVelocity;

	eF32					m_particleLifeTime;
	ePathSampler*			m_particleSizePath;
	ePathSampler*			m_particleColorPath;

	eEditMesh*				m_attractorMesh;
	Approximation*			m_approxSpace;

	eEditMesh*				m_emitterMesh;
	eF32					m_emitterMeshArea;
	eF32*					m_emitterAreaSums;

	eF32					m_fadeOutAmount;

	eF32					m_stretchAmount;
	eF32					m_gravity;
//	TensorProductSpace*		m_approxSpaceSingle;
//	TensorProductSpace*		m_approxSpace[3];
/*
	const eMathFunction *   m_partLifetime;
    const eMathFunction *   m_emitter;
    const eMathFunction *   m_emissionFunc;
    const eMathFunction *   m_sizeFunc;
    const eMathFunction *   m_color;
*/
//	const eF32*				m_partLifetime;
//    const eF32*				m_emitter;
//    const eF32*				m_emissionFunc;
//    const eF32*				m_sizeFunc;
//    const eF32*				m_color;
};

#endif // PARTICLE_SYS_HPP