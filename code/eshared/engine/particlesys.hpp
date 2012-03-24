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

#define PSYS_MAX_PARTICLES 100000
#define PSYS_MAX_TIME_STEP (1.0f/30.0f)
//#define PSYS_USE_EMITTERMODES
//#define PSYS_USE_LIQUID_INFLUENCE

class eParticleSystem
{
public:
    // Instance of a particle system for scene graph.
    class Instance : public eIRenderable
    { 
    public:
        Instance(eParticleSystem &psys);

        virtual void            update(eF32 time);
        virtual void            getRenderJobs(const eMatrix4x4 &mtx, const eMatrix4x4 &normalMtx, eRenderJobPtrArray &jobs, eU32 passID) const;
        virtual const eAABB &   getBoundingBox() const;
        virtual Type            getType() const;

    private:
        eParticleSystem &       m_psys;
        mutable eMaterial       m_mat;
    };

#if defined(PSYS_USE_EMITTERMODES)
    enum EmitterMode {
        EMITTERMODE_FACES,
        EMITTERMODE_EDGES,
        EMITTERMODE_VERTICES,
        EMITTERMODE_FLUID_INFLUENCED,
    };
#endif

	enum PATHSAMPLERS {
		SIZE = 0,
		COLOR = 1,
		ROTATION = 2,
		__PATHSAMPLERSIZE__ = 3,
	};

    struct DynamicEntity {
        eVector3            position;   // [m]
        eVector3            velocity;   // [m/s]
    };

    struct Particle {
        DynamicEntity       dynamicEntity;
        eF32                timeToLive;     // (0..1]
        eF32                timeConstant;   // 1/ttl_max
        eF32                rotation;
		eF32				mass;
		eF32				size;
    };

    typedef eArray<Particle> ParticleArray;

public:
    eParticleSystem();
    ~eParticleSystem();

    void                    update(eF32 time);
	void					init(eGraphicsApiDx9 * gfx);
#if defined(PSYS_USE_EMITTERMODES)
	void                    setEmitter(const eEditMesh *mesh, EmitterMode mode);
    EmitterMode             m_emitterMode;
#else
	void                    setEmitter(const eEditMesh *mesh);
#endif
    eITexture2d *           m_tex;
    eF32                    m_emissionFreq;
    eF32                    m_emissionVel;
    eF32                    m_lifeTime;
    eF32                    m_randomization;
    eF32                    m_stretchAmount;
    eF32                    m_gravity;
	eVector3*				m_gravityConst;

//    static const eF32       MAX_TIME_STEP;
    eGraphicsApiDx9 *         m_gfx;
    eGeometry *             m_geometry;
    ParticleArray           m_particles;
    eAABB                   m_boundingBox;

    eF32                    m_lastTime;
    eF32                    m_timer;
    eF32                    m_emitTime;
    eU32                    m_count;

	ePathSampler*			m_pathSampler[__PATHSAMPLERSIZE__];

    const eEditMesh *       m_emitterMesh;
    eF32                    m_emitterEmitSurfaceArea;
    eArray<eF32>            m_emitterEntities;


    eBlendMode              m_blendSrc;
    eBlendMode              m_blendDst;
    eBlendOp                m_blendOp;

#if defined(PSYS_USE_LIQUID_INFLUENCE)
	const eStableFluids*    m_fluids;
#endif

private:
    __forceinline __m128 _calcAcceleration(eF32 mass, const __m128 pos) const {
		__m128 result = _mm_load_ps(&this->m_gravityConst->x);
#if defined(PSYS_USE_LIQUID_INFLUENCE)
		if(this->m_fluids != eNULL) {
			const __m128 fluidForce = this->m_fluids->evaluateAt(pos);
			result = _mm_add_ps(result, _mm_mul_ps(fluidForce, _mm_set1_ps(mass)));
		}
#endif
		return result;
	}

    void                    _moveParticles(Particle *particles, eU32 count, eF32 nowTime, eF32 deltaTime);
    static void             _fillDynamicBuffers(ePtr param, eGeometry *geo);
};

#endif // PARTICLE_SYS_HPP