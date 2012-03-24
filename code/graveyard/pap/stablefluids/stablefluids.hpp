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

#ifndef STABLEFLUIDS_HPP
#define STABLEFLUIDS_HPP

//#include <xmmintrin.h>
//#include <emmintrin.h>
#include <intrin.h>

#define LQ_MAX_EMITTERS 16
#define LQ_FIELD_U				0
#define LQ_FIELD_UPREV			(LQ_FIELD_U + 3)
#define LQ_FIELD_DENSITY		(LQ_FIELD_UPREV + 3)
#define LQ_FIELD_DENSITYPREV	(LQ_FIELD_DENSITY + LQ_MAX_EMITTERS)
#define LQ_FIELDS				(LQ_FIELD_DENSITYPREV + LQ_MAX_EMITTERS)
 
class eEditMesh;
class eParticleSystem;

class eStableFluids
{
public:
	eStableFluids();
    ~eStableFluids();  

	void	setGrid(const eIXYZ& cellSize);

	void	simulate(eF32 dt);

	void	evaluateDensGrid(eColor* bitmap, eU32 width, eU32 height) const;  
//    void    setup(eVector3 scale, eBool hasBorder, const eEditMesh* boundMesh, const eEditMesh* forceMesh, const eF32 forceAmount, const eEditMesh** emissionMeshes, eU32 emitterCnt);
	void	setup(eVector3 scale);
    eF32        m_viscosity;

	eVector3	m_sampleScale;
	eVector3	m_cellCntDivSampleScale;

	eU32		m_offsetstep[4];

//private:
	eU32		m_numCells[4];
	eF32		m_numCellsF[4];
	eF32*		m_numCellsVPlusHalf;
	eF32		m_h[4];
    eF32		m_minushalfh[4];
    eF32		m_halfhinv[4];

	eF32*		m_field[LQ_FIELDS];

	eBool	m_hasBorder;
	
	const	eEditMesh*	m_boundMesh;
	const	eEditMesh*	m_forceMesh;
	eF32	m_forceAmount;
	const	eEditMesh*	m_emissionMeshes[LQ_MAX_EMITTERS];
	eU32	m_emitterCnt;
	//	eF32	m_emissionAmount;

	void	add_source(eU32 b, eF32* x, eF32 dt);
	void	diffuse(eU32 b, eF32* x, const eF32* x0, eF32 diff, eF32 dt);
	void	advect(eU32 b, eF32* d, eF32* d0, eF32* pos[3], eF32 dt);
	void	dens_step(eU32 b, eF32* x, eF32* x0, eF32* pos[3], eF32 diff, eF32 dt);
	void	vel_step(eF32* pos[3], eF32* pos0[3], eF32 visc, eF32 dt);
	void	project(eF32* pos[3], eF32* p, eF32* div, eF32 dt);
	void	set_bnd(eU32 b, eF32* x, eF32 dt);

	inline eBool	convertGlobalPosToGridPos_AndTestIfOutside(const __m128 globalPos, __m128 &result) const {
		__m128 half = _mm_set1_ps(0.5f);

		__m128 staticadd;
		staticadd.m128_f32[0] = (this->m_numCells[0] > 1) ? 0.0f : 0.5f; 
		staticadd.m128_f32[1] = (this->m_numCells[1] > 1) ? 0.0f : 0.5f; 
		staticadd.m128_f32[2] = (this->m_numCells[2] > 1) ? 0.0f : 0.5f; 
		staticadd.m128_f32[3] = 0.0f; 
		const __m128 vpos = _mm_mul_ps(globalPos, _mm_loadu_ps(&this->m_cellCntDivSampleScale.x));
		result = _mm_add_ps(half, _mm_add_ps(staticadd, _mm_and_ps(_mm_cmple_ps(staticadd, _mm_setzero_ps()), vpos)));
		__m128 test = _mm_cmplt_ps(result, half);
		test = _mm_or_ps(test, _mm_cmpgt_ps(result, _mm_load_ps(&this->m_numCellsVPlusHalf[0])));
		test = _mm_hadd_ps(test,test);
		test = _mm_hadd_ps(test,test);
		return (test.m128_f32[0] != 0.0f);
	}

	inline const __m128	clampGridPos(const __m128 gridPos) const {
		return _mm_max_ps(_mm_set1_ps(0.5f), _mm_min_ps(_mm_load_ps(&this->m_numCellsVPlusHalf[0]), gridPos));
	}

	// pos is assumed to be in grid space, with unit cell-length
	// returns 4 x interpolated value
	inline const __m128	get_linear_interpolated(const __m128 pos, const eF32* data) const {
		const __m128 craw = _mm_min_ps(_mm_load_ps(&this->m_numCellsVPlusHalf[0]), _mm_max_ps(pos, _mm_set1_ps(0.5f)));
		eU32 tp;
		const __m128 ctraw = this->IX_SSE(craw, tp);

		const __m128 s1 = _mm_sub_ps(craw,ctraw);
		const __m128 s0 = _mm_sub_ps(_mm_set1_ps(1.0f), s1);
		const __m128 slow = _mm_unpacklo_ps(s0,s1);
		const __m128 sxreg = _mm_shuffle_ps(slow, slow, _MM_SHUFFLE(1,0,1,0));
		const __m128 syreg = _mm_mul_ps(sxreg,_mm_shuffle_ps(slow, slow, _MM_SHUFFLE(3,3,2,2)));
		const __m128 sz0reg = _mm_mul_ps(syreg, _mm_shuffle_ps(s0, s0, _MM_SHUFFLE(2,2,2,2)));
		const __m128 sz1reg = _mm_mul_ps(syreg, _mm_shuffle_ps(s1, s1, _MM_SHUFFLE(2,2,2,2)));

		const __m128  data3 = _mm_add_ps(_mm_mul_ps(_mm_loadh_pi(_mm_loadl_pi(sxreg, (__m64*)&data[tp]), (__m64*)&data[tp + this->m_offsetstep[1]]),sz0reg), 
						   			     _mm_mul_ps(_mm_loadh_pi(_mm_loadl_pi(sxreg, (__m64*)&data[tp + this->m_offsetstep[2]]), (__m64*)&data[tp + this->m_offsetstep[1] + this->m_offsetstep[2]]), sz1reg));

		const __m128 psum33 = _mm_hadd_ps(data3, data3);
		const __m128 psumg = _mm_hadd_ps(psum33, psum33);

		return psumg;
	}

	// pos is assumed to be in world space
	inline void	set_linear_interpolated(const __m128 pos, const __m128 newval, const eF32* data) const {
		eU32 tp;
		const __m128 ctraw = this->IX_SSE(pos, tp);

		const __m128 s1 = _mm_sub_ps(pos,ctraw);
		const __m128 one = _mm_set1_ps(1.0f);
		const __m128 s0 = _mm_sub_ps(one, s1);
		const __m128 slow = _mm_unpacklo_ps(s0,s1);

		const __m128 ymul = _mm_mul_ps(_mm_shuffle_ps(slow, slow, _MM_SHUFFLE(1,0,1,0)), _mm_shuffle_ps(slow,slow,_MM_SHUFFLE(3,3,2,2)));
		const __m128 z0mul = _mm_mul_ps(ymul, _mm_shuffle_ps(s0, s0, _MM_SHUFFLE(2,2,2,2)));
		const __m128 z1mul = _mm_mul_ps(ymul, _mm_shuffle_ps(s1, s1, _MM_SHUFFLE(2,2,2,2)));
		const __m128 invz0mul = _mm_sub_ps(_mm_set1_ps(1.0f), z0mul);
		const __m128 invz1mul = _mm_sub_ps(_mm_set1_ps(1.0f), z1mul);

		const __m128 dataz0 = _mm_loadh_pi(_mm_loadl_pi(_mm_setzero_ps(), (__m64*)&data[tp]), (__m64*)&data[tp + this->m_offsetstep[1]]);
		const __m128 newdataz0 = _mm_add_ps(_mm_mul_ps(newval, z0mul), _mm_mul_ps(dataz0, invz0mul));
		_mm_storel_pi((__m64*)&data[tp], newdataz0);
		_mm_storeh_pi((__m64*)&data[tp + this->m_offsetstep[1]], newdataz0);

		const __m128 dataz1 = _mm_loadh_pi(_mm_loadl_pi(_mm_setzero_ps(), (__m64*)&data[tp + this->m_offsetstep[2]]), (__m64*)&data[tp + this->m_offsetstep[1] + this->m_offsetstep[2]]);
		const __m128 newdataz1 = _mm_add_ps(_mm_mul_ps(newval, z1mul), _mm_mul_ps(dataz1, invz1mul));
		_mm_storel_pi((__m64*)&data[tp + this->m_offsetstep[2]], newdataz1);
		_mm_storeh_pi((__m64*)&data[tp + this->m_offsetstep[1] + this->m_offsetstep[2]], newdataz1);
	}

	eU32								m_gcells;
	eF32*								m_ixconst;

	inline eU32 IX(eU32 x, eU32 y, eU32 z) const {
//		return (z * m_offsetstep[2] + y * m_offsetstep[1] + x);
		eASSERT((x >= 0) && (x < m_numCells[0] + 2));
		eASSERT((y >= 0) && (y < m_numCells[1] + 2));
		eASSERT((z >= 0) && (z < m_numCells[2] + 2));
		eASSERT((z * m_offsetstep[2] + y * m_offsetstep[1] + x + 3) < this->m_gcells);
	
		// version with alignment offset
		return (z * m_offsetstep[2] + y * m_offsetstep[1] + x + 3);
	}

	// returns truncated value and stores calculated IX offset
	inline const __m128 IX_SSE(const __m128 val, eU32& result) const {
		const __m128 ixTrunced = _mm_cvtepi32_ps(_mm_cvttps_epi32(val));
		const __m128 ixScaled = _mm_mul_ps(ixTrunced, _mm_load_ps(this->m_ixconst));
		const __m128 ixResult0 = _mm_add_ps(ixScaled, _mm_shuffle_ps(ixScaled,ixScaled,_MM_SHUFFLE(1,0,3,2)));
		const __m128 ixResult = _mm_add_ps(ixResult0, _mm_shuffle_ps(ixResult0, ixResult0, _MM_SHUFFLE(0,1,0,1)));
		result = _mm_cvttss_si32(ixResult) + 3;
		return ixTrunced;
	}

	static inline __m128 normalize(__m128 m)
	{
		__m128 l = _mm_mul_ps(m, m);
		l = _mm_add_ps(l, _mm_shuffle_ps(l, l, 0x4E));
		return _mm_div_ps(m, _mm_sqrt_ps(_mm_add_ps(l,
										   _mm_shuffle_ps(l, l, 0x11))));
	}

	void	init(const eIXYZ& cellSize);
	void	deinit();

public:

	__m128	evaluateAt(const __m128 posreg) const {
		__declspec(align(16)) eU32 posInt[4];

		const __m128 half = _mm_set1_ps(0.5f);
		__m128 pos = _mm_add_ps(_mm_mul_ps(posreg, _mm_loadu_ps(&this->m_cellCntDivSampleScale.x)), half);

		const __m128 craw = _mm_min_ps(_mm_load_ps(&this->m_numCellsVPlusHalf[0]), _mm_max_ps(pos, half));
		const __m128i truncated = _mm_cvttps_epi32(craw);
		_mm_store_si128((__m128i*)&posInt[0], truncated);
		const eU32 tp = IX(posInt[0], posInt[1], posInt[2]);

		const __m128 ctraw = _mm_cvtepi32_ps(truncated);

		const __m128 s1 = _mm_sub_ps(craw,ctraw);
		const __m128 one = _mm_set1_ps(1.0f);
		const __m128 s0 = _mm_sub_ps(one, s1);
		const __m128 slow = _mm_unpacklo_ps(s0,s1);
		const __m128 sxreg = _mm_shuffle_ps(slow, slow, _MM_SHUFFLE(1,0,1,0));
		const __m128 syreg = _mm_mul_ps(sxreg,_mm_shuffle_ps(slow, slow, _MM_SHUFFLE(3,3,2,2)));
		const __m128 sz0reg = _mm_mul_ps(syreg, _mm_shuffle_ps(s0, s0, _MM_SHUFFLE(2,2,2,2)));
		const __m128 sz1reg = _mm_mul_ps(syreg, _mm_shuffle_ps(s1, s1, _MM_SHUFFLE(2,2,2,2)));

		//u0
		const __m128  data0 = _mm_add_ps(_mm_mul_ps(_mm_loadh_pi(_mm_loadl_pi(sxreg, (__m64*)&m_field[LQ_FIELD_U + 0][tp]), (__m64*)&m_field[LQ_FIELD_U + 0][tp + this->m_offsetstep[1]]),sz0reg), 
						   				 _mm_mul_ps(_mm_loadh_pi(_mm_loadl_pi(sxreg, (__m64*)&m_field[LQ_FIELD_U + 0][tp + this->m_offsetstep[2]]), (__m64*)&m_field[LQ_FIELD_U + 0][tp + this->m_offsetstep[1] + this->m_offsetstep[2]]), sz1reg));
		//u1
		const __m128  data1 = _mm_add_ps(_mm_mul_ps(_mm_loadh_pi(_mm_loadl_pi(sxreg, (__m64*)&m_field[LQ_FIELD_U + 1][tp]), (__m64*)&m_field[LQ_FIELD_U + 1][tp + this->m_offsetstep[1]]),sz0reg), 
						   				 _mm_mul_ps(_mm_loadh_pi(_mm_loadl_pi(sxreg, (__m64*)&m_field[LQ_FIELD_U + 1][tp + this->m_offsetstep[2]]), (__m64*)&m_field[LQ_FIELD_U + 1][tp + this->m_offsetstep[1] + this->m_offsetstep[2]]), sz1reg));

		const __m128 psum1100 = _mm_add_ps(_mm_shuffle_ps(data0, data1, _MM_SHUFFLE(3,2,1,0)), _mm_shuffle_ps(data0, data1, _MM_SHUFFLE(1,0,3,2)));

		//u2
		const __m128  data2 = _mm_add_ps(_mm_mul_ps(_mm_loadh_pi(_mm_loadl_pi(sxreg, (__m64*)&m_field[LQ_FIELD_U + 2][tp]), (__m64*)&m_field[LQ_FIELD_U + 2][tp + this->m_offsetstep[1]]),sz0reg), 
						   				 _mm_mul_ps(_mm_loadh_pi(_mm_loadl_pi(sxreg, (__m64*)&m_field[LQ_FIELD_U + 2][tp + this->m_offsetstep[2]]), (__m64*)&m_field[LQ_FIELD_U + 2][tp + this->m_offsetstep[1] + this->m_offsetstep[2]]), sz1reg));
		//dens
		const __m128  data3 = _mm_set1_ps(1.0f);

		const __m128 psum3322 = _mm_add_ps(_mm_shuffle_ps(data2, data3, _MM_SHUFFLE(3,2,1,0)), _mm_shuffle_ps(data2, data3, _MM_SHUFFLE(1,0,3,2)));
		const __m128 psumg = _mm_add_ps(_mm_shuffle_ps(psum1100, psum3322, _MM_SHUFFLE(2,0,2,0)), _mm_shuffle_ps(psum1100, psum3322, _MM_SHUFFLE(3,1,3,1)));
		return psumg;
	}

};

#endif // STABLEFLUIDS_HPP