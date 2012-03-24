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
//#include <stdio.h>

#ifdef eUSE_SSE
#include <intrin.h>
#endif

#define LQ_B_XYZ	0
#define LQ_B_X		1
#define LQ_B_Y		2
#define LQ_B_Z		3
#define LQ_B_CLEAR	4
#define LQ_B_DENS   5
#define LQ_B_DENS_MAX (LQ_B_DENS + LQ_MAX_EMITTERS - 1)

eStableFluids::eStableFluids()
{
	this->m_numCellsVPlusHalf = (eF32*)eMemAllocAlignedAndZero(4 * sizeof(eF32), 16);
	this->m_ixconst = (eF32*)eMemAllocAlignedAndZero(4 * sizeof(eF32), 16);
	eInt initCnt[] = {1,1,1};
	this->init((eIXYZ&)initCnt[0]);
}

eStableFluids::~eStableFluids()
{
	this->deinit();
	eFreeAligned(this->m_numCellsVPlusHalf);
	eFreeAligned(this->m_ixconst);
}

void	
eStableFluids::setGrid(const eIXYZ& cellSize) {
	this->deinit();
	// make x and y-cnt aligned to 4
	eIXYZ cellCnt;
	cellCnt.x = (((cellSize.x - 1) >> 2) << 2) + 4;
	cellCnt.y = (((cellSize.y - 1) >> 2) << 2) + 4;
//	cellCnt.z = (((cellSize.z - 1) >> 1) << 1) + 2; // align to 2
	cellCnt.z = cellSize.z;
	this->init(cellCnt);
}

void	
eStableFluids::deinit() {
	for(eU32 i = 0; i < LQ_FIELDS; i++)
		eFreeAligned(this->m_field[i]);
}

void	
eStableFluids::init(const eIXYZ& numCells) {
	for(eU32 i = 0; i < 3; i++) {
		this->m_numCells[i] = numCells[i];
		this->m_numCellsF[i] = (eF32)numCells[i];
		this->m_numCellsVPlusHalf[i] = this->m_numCellsF[i] + 0.5f;
	}
	this->m_numCellsVPlusHalf[3] = 1.5f;
	this->m_gcells = 1;
	for(eU32 i = 0; i < 3; i++) {
		eF32 h = (eF32)(this->m_numCells[i]+2);
		this->m_h[i] = 1.0f / h;
		this->m_minushalfh[i] = - 0.5f * m_h[i];
		this->m_halfhinv[i] = (0.5f) / m_h[i];

		this->m_offsetstep[i] = this->m_gcells;
		// additional 2 for boundaries, or 4 in case of the x-axis (for alignment reasons)
		eU32 boundOffset = (i == 0) ? 4 : 2;
		this->m_gcells *= (numCells[i] + boundOffset); 
	}
	// we add (16 - 4) bytes, so that x-values of 1 are always at aligned positions
	m_gcells += 3;

	for(eU32 i = 0; i < LQ_FIELDS; i++)
		this->m_field[i] = (eF32*)eMemAllocAlignedAndZero(this->m_gcells * sizeof(eF32), 16);

	this->m_ixconst[0] = 1;
	this->m_ixconst[1] = (eF32)this->m_offsetstep[1];
	this->m_ixconst[2] = (eF32)this->m_offsetstep[2];
	this->m_ixconst[3] = 0;
}

void    
eStableFluids::setup(eVector3 scale) {
	this->m_sampleScale = scale;
	m_cellCntDivSampleScale = eVector3((eF32)this->m_numCells[0], (eF32)this->m_numCells[1], (eF32)this->m_numCells[2]);
	m_cellCntDivSampleScale.scale(1.0f/scale);
}

void	
eStableFluids::diffuse(eU32 b, eF32* x1, const eF32* x0, eF32 diff, eF32 dt) {
	ePROFILER_ZONE("Fluid Sim - diffuse");
	this->set_bnd(b, x1, dt);

	if(diff == 0.0f) {
		eMemCopy(x1, x0, this->m_gcells * sizeof(eF32));
		this->set_bnd(b, x1, dt);
		return;
	}

	eF32 a = dt * diff;
	for(eU32 dim = 0; dim < 3; dim++) 
		a *= (eF32)this->m_numCells[dim];
    eF32 adiv = 1.0f / (1.0f + a * 6.0f);

	// TODO: implement 2D variant
	for(eU32 k = 0; k < 20; k++) {
#ifdef eUSE_SSE
		const __m128 a0 = _mm_set1_ps(a);
		const __m128 adiv0 = _mm_set1_ps(adiv);

		for(eU32 z = 1; z <= this->m_numCells[2]; z++) {
			for(eU32 y = 1; y <= this->m_numCells[1]; y++) {
				// cell x cnt is a multiple of 4
				eU32 pos = IX(1, y, z);
				__m128 vx0 = _mm_load_ps(&x1[pos - 4]);
				__m128 vx1 = _mm_load_ps(&x1[pos]);
				__m128 vx2 = _mm_set1_ps(0.0f);
				__m128* vcur = &vx0;
				__m128* vnext = &vx1;
				__m128* vlast = &vx2;
				for(eU32 x = 1; x <= this->m_numCells[0]; x+=4) {

					__m128* temp = vlast;
					vlast = vcur;
					vcur = vnext;
					vnext = temp;
					*vnext = _mm_load_ps(&x1[pos + 4]);
					const __m128 dy0 = _mm_load_ps(&x1[pos - this->m_offsetstep[1]]);
					const __m128 dy1 = _mm_load_ps(&x1[pos + this->m_offsetstep[1]]);
					const __m128 dz0 = _mm_load_ps(&x1[pos - this->m_offsetstep[2]]);
					const __m128 dz1 = _mm_load_ps(&x1[pos + this->m_offsetstep[2]]);

					const __m128 la1 = _mm_shuffle_ps(*vlast, *vcur, _MM_SHUFFLE(1,0,3,3)); // |cur.y|cur.x|last.w|last.w|
					const __m128 la2 = _mm_shuffle_ps(*vcur, *vnext, _MM_SHUFFLE(0,0,3,2)); // |next.x|next.x|cur.w|cur.z|
//					const __m128 dx0 = _mm_shuffle_ps(la1, *vcur, _MM_SHUFFLE(2,1,2,0));	// |cur.z|cur.y|cur.x|last.w|
//					const __m128 dx1 = _mm_shuffle_ps(*vcur, la2, _MM_SHUFFLE(2,1,2,1));	// |next.x|cur.w|cur.z|cur.y|

					const __m128 dx0 = _mm_loadu_ps(&x1[pos - 1]);	// |cur.z|cur.y|cur.x|last.w|
					const __m128 dx1 = _mm_loadu_ps(&x1[pos + 1]);	// |cur.z|cur.y|cur.x|last.w|
				    const __m128 sx = _mm_add_ps(dx0, dx1);
				    const __m128 sy = _mm_add_ps(dy0, dy1);
				    const __m128 sz = _mm_add_ps(dz0, dz1);
				    
					const __m128 sum = _mm_add_ps(sx, sy);
					const __m128 sumg = _mm_add_ps(sum, sz);
					
					const __m128 am = _mm_mul_ps(sumg, a0);
//					const __m128 insum = _mm_add_ps(am, *vcur);
					const __m128 insum = _mm_add_ps(am, _mm_load_ps(&x0[pos]));
					const __m128 om = _mm_mul_ps(insum, adiv0);

					_mm_store_ps(&x1[pos], om);
//					*vcur = om;
					pos += 4;
				}
			}
		}
#else
		eU32 xStep = 1;
		eU32 yStep = this->m_offsetstep[1];
		eU32 zStep = this->m_offsetstep[2];
		eU32 pos = zStep + yStep;
		for(eU32 z = 1; z <= this->m_numCells[2]; z++) {
			for(eU32 y = 1; y <= this->m_numCells[1]; y++) {

				for(eU32 x = 1; x <= this->m_numCells[0]; x++) {
					pos++;
					x1[pos] = (x0[pos] + a*(x1[pos - xStep] + x1[pos + xStep] +
											  x1[pos - yStep] + x1[pos + yStep] +
											  x1[pos - zStep] + x1[pos + zStep])) * adiv;
				}
				pos += 2;
			}
			pos += yStep + yStep;
		}
#endif
		this->set_bnd(b, x1, dt);
	}
}

void	
eStableFluids::advect(eU32 b, eF32* d, eF32* d0, eF32* pos[3], eF32 dt) {
	ePROFILER_ZONE("Fluid Sim - advect");

	const __m128 ncellsdt = _mm_mul_ps(_mm_set1_ps(dt), _mm_loadu_ps(&this->m_numCellsF[0]));
	const __m128 dtx = _mm_shuffle_ps(ncellsdt, ncellsdt, _MM_SHUFFLE(0,0,0,0));
	const __m128 dty = _mm_shuffle_ps(ncellsdt, ncellsdt, _MM_SHUFFLE(1,1,1,1));
	const __m128 dtz = _mm_shuffle_ps(ncellsdt, ncellsdt, _MM_SHUFFLE(2,2,2,2));


	__m128 offset;
	offset.m128_f32[0] = 0;
	offset.m128_f32[1] = 1;
	offset.m128_f32[2] = 2;
	offset.m128_f32[3] = 3;

//	if(this->m_numCells[2] > 1) 
	{
		for(eU32 zn = 1; zn <= this->m_numCells[2]; zn++)
			for(eU32 yn = 1; yn <= this->m_numCells[1]; yn++) {
				eU32 pos_w = IX(1, yn, zn);
				for(eU32 xn = 1; xn <= this->m_numCells[0]; xn+=4, pos_w += 4) {

					const __m128 posx = _mm_sub_ps(_mm_add_ps(_mm_set1_ps((eF32)xn), offset), _mm_mul_ps(dtx, _mm_load_ps(&pos[0][pos_w])));
					const __m128 posy = _mm_sub_ps(_mm_set1_ps((eF32)yn),                     _mm_mul_ps(dty, _mm_load_ps(&pos[1][pos_w])));
					const __m128 posz = _mm_sub_ps(_mm_set1_ps((eF32)zn),                     _mm_mul_ps(dtz, _mm_load_ps(&pos[2][pos_w])));
				
					const __m128 xy_0 = _mm_shuffle_ps(posx,posy,_MM_SHUFFLE(1,0,1,0));
					const __m128 xy_1 = _mm_shuffle_ps(posx,posy,_MM_SHUFFLE(3,2,3,2));

					const __m128 val0 = this->get_linear_interpolated(_mm_shuffle_ps(xy_0,posz,_MM_SHUFFLE(0,0,2,0)), d0);
					const __m128 val1 = this->get_linear_interpolated(_mm_shuffle_ps(xy_0,posz,_MM_SHUFFLE(0,1,3,1)), d0);
					const __m128 val2 = this->get_linear_interpolated(_mm_shuffle_ps(xy_1,posz,_MM_SHUFFLE(0,2,2,0)), d0);
					const __m128 val3 = this->get_linear_interpolated(_mm_shuffle_ps(xy_1,posz,_MM_SHUFFLE(0,3,3,1)), d0);

					const __m128 result = _mm_shuffle_ps(_mm_shuffle_ps(val0, val1, _MM_SHUFFLE(0,0,0,0)), _mm_shuffle_ps(val2, val3, _MM_SHUFFLE(0,0,0,0)), _MM_SHUFFLE(2,0,2,0));
					_mm_store_ps(&d[pos_w], result);
				}
			}
	};
/**/

		this->set_bnd(b, d, dt);
}

void	
eStableFluids::dens_step(eU32 b, eF32* x, eF32* x0, eF32* pos[3], eF32 diff, eF32 dt) {
	for(eU32 k = 0; k < this->m_gcells; k++) 
		x[k] += x0[k] * dt;

	this->add_source(b, x, dt);
	this->set_bnd(b, x, dt);
	eSwap(x0, x);
	this->diffuse(b, x, x0, diff, dt);
	eSwap(x0, x);
	this->advect(b, x, x0, pos, dt);	
}

void	
eStableFluids::vel_step(eF32* pos[3], eF32* pos0[3], eF32 visc, eF32 dt) {

	{
		ePROFILER_ZONE("Fluid Sim - 1");
		for(eU32 dim = 0; dim < 3; dim++) 
			this->add_source(dim + LQ_B_X, pos[dim], dt);
	}

	{
		ePROFILER_ZONE("Fluid Sim - 2");
		for(eU32 dim = 0; dim < 3; dim++) {
			eSwap(pos0[dim], pos[dim]);
			this->diffuse(dim + LQ_B_X, pos[dim], pos0[dim], visc, dt);
		}
	}


	this->project(pos, pos0[0], pos0[1], dt);

	for(eU32 dim = 0; dim < 3; dim++)
		eSwap(pos0[dim], pos[dim]);
	{
		ePROFILER_ZONE("Fluid Sim - 5");
		for(eU32 dim = 0; dim < 3; dim++) 
			this->advect(LQ_B_X + dim, pos[dim], pos0[dim], pos0, dt);
	} 
	{
		ePROFILER_ZONE("Fluid Sim - 6");
		this->project(pos, pos0[0], pos0[1], dt);
	}

}

void	
eStableFluids::project(eF32* pos[3], eF32* p, eF32* div, eF32 dt) {
	{
		ePROFILER_ZONE("Fluid Sim - Project 1");
		__m128 vx0, vx1, vx2;
		__m128* vcur = &vx0;
		__m128* vnext = &vx1;
		__m128* vlast = &vx2;
		eMemSet(p,0,this->m_gcells * sizeof(eF32));
		for(eU32 z = 1; z <= this->m_numCells[2]; z++) {
			for(eU32 y = 1; y <= this->m_numCells[1]; y++) {
				eU32 pos_w = IX(1, y, z);
				*vcur = _mm_load_ps(&pos[0][pos_w - 4]);
				*vnext = _mm_load_ps(&pos[0][pos_w]);
				for(eU32 x = 1; x <= this->m_numCells[0]; x += 4, pos_w += 4) {
					__m128* temp = vlast;
					vlast = vcur;
					vcur = vnext;
					vnext = temp;
					*vnext = _mm_load_ps(&pos[0][pos_w + 4]);
					const __m128 la1 = _mm_shuffle_ps(*vlast, *vcur, _MM_SHUFFLE(1,0,3,3)); // |cur.y|cur.x|last.w|last.w|
					const __m128 la2 = _mm_shuffle_ps(*vcur, *vnext, _MM_SHUFFLE(0,0,3,2)); // |next.x|next.x|cur.w|cur.z|

					__m128 val0 = _mm_shuffle_ps(la1, *vcur, _MM_SHUFFLE(2,1,2,0));	// |cur.z|cur.y|cur.x|last.w|
					__m128 val1 = _mm_shuffle_ps(*vcur, la2, _MM_SHUFFLE(2,1,2,1));	// |next.x|cur.w|cur.z|cur.y|
					__m128 sum = _mm_setzero_ps();
					for(eU32 i = 0; i < 3; i++) {
						if(i != 0) {
							val0 = _mm_load_ps(&pos[i][pos_w - this->m_offsetstep[i]]);
							val1 = _mm_load_ps(&pos[i][pos_w + this->m_offsetstep[i]]);
						}
						const __m128 xs = _mm_sub_ps(val1,val0);
						const __m128 xm = _mm_set1_ps(this->m_minushalfh[i]);
						const __m128 xx = _mm_mul_ps(xs,xm);
						sum = _mm_add_ps(sum, xx);
					}
					_mm_store_ps(&div[pos_w], sum);
				}
			}
		}
		this->set_bnd(LQ_B_XYZ, div, dt);
		this->set_bnd(LQ_B_XYZ, p, dt); // we can save this
	}

	{
		__m128 vx0, vx1, vx2;
		__m128* vcur = &vx0;
		__m128* vnext = &vx1;
		__m128* vlast = &vx2;
//		ePROFILER_ZONE("Fluid Sim - Project 2");
		for(eU32 k = 0; k < 20; k++) {
			for(eU32 z = 1; z <= this->m_numCells[2]; z++) {
				for(eU32 y = 1; y <= this->m_numCells[1]; y++) {
					eU32 pos_w = IX(1, y, z);
					eF32* divPtr = &div[pos_w];
					eF32* dataPtr = &p[pos_w];
					eF32* target = dataPtr + this->m_numCells[0];
					__m128 last;
					__m128 cur = _mm_load_ps(&p[pos_w - 4]);
					__m128 next = _mm_load_ps(&p[pos_w]);
					__m128 sum;
					__m128 a = _mm_set1_ps(1.0f / 6.0f);
					for(; dataPtr < target; dataPtr += 4, divPtr += 4) {
						last = cur;
						cur = next;
						sum = _mm_load_ps(divPtr);

						next = _mm_load_ps(dataPtr + 4);
						sum = _mm_add_ps(sum, _mm_shuffle_ps(_mm_shuffle_ps(last, cur, _MM_SHUFFLE(1,0,3,3)), cur, _MM_SHUFFLE(2,1,2,0)));
						sum = _mm_add_ps(sum, _mm_shuffle_ps(cur, _mm_shuffle_ps(cur, next, _MM_SHUFFLE(0,0,3,2)), _MM_SHUFFLE(2,1,2,1)));
						sum = _mm_add_ps(sum, _mm_load_ps(dataPtr - this->m_offsetstep[1]));
						sum = _mm_add_ps(sum, _mm_load_ps(dataPtr + this->m_offsetstep[1]));
						sum = _mm_add_ps(sum, _mm_load_ps(dataPtr - this->m_offsetstep[2]));
						sum = _mm_add_ps(sum, _mm_load_ps(dataPtr + this->m_offsetstep[2]));
						sum = _mm_mul_ps(sum, a);

						_mm_store_ps(dataPtr , sum);
					}
				}
			}
			this->set_bnd(LQ_B_XYZ, p, dt);
		}
	}
	{
		__m128 vx0, vx1, vx2;
		__m128* vcur = &vx0;
		__m128* vnext = &vx1;
		__m128* vlast = &vx2;
//		ePROFILER_ZONE("Fluid Sim - Project 3");
		for(eU32 z = 1; z <= this->m_numCells[2]; z++) 
			for(eU32 y = 1; y <= this->m_numCells[1]; y++) {
				eU32 pos_w = IX(1, y, z);
				*vcur = _mm_load_ps(&p[pos_w - 4]);
				*vnext = _mm_load_ps(&p[pos_w]);
				for(eU32 x = 1; x <= this->m_numCells[0]; x += 4, pos_w += 4) {
					__m128* temp = vlast;
					vlast = vcur;
					vcur = vnext;
					vnext = temp;
					*vnext = _mm_load_ps(&p[pos_w + 4]);
					const __m128 la1 = _mm_shuffle_ps(*vlast, *vcur, _MM_SHUFFLE(1,0,3,3)); // |cur.y|cur.x|last.w|last.w|
					const __m128 la2 = _mm_shuffle_ps(*vcur, *vnext, _MM_SHUFFLE(0,0,3,2)); // |next.x|next.x|cur.w|cur.z|

					__m128 val0 = _mm_shuffle_ps(la1, *vcur, _MM_SHUFFLE(2,1,2,0));	// |cur.z|cur.y|cur.x|last.w|
					__m128 val1 = _mm_shuffle_ps(*vcur, la2, _MM_SHUFFLE(2,1,2,1));	// |next.x|cur.w|cur.z|cur.y|
					for(eU32 i = 0; i < 3; i++) {
						if(i != 0) {
							val0 = _mm_load_ps(&p[pos_w - this->m_offsetstep[i]]);
							val1 = _mm_load_ps(&p[pos_w + this->m_offsetstep[i]]);
						}
						const __m128 xs = _mm_sub_ps(val1,val0);
						const __m128 xx = _mm_mul_ps(xs,_mm_set1_ps(this->m_halfhinv[i]));
						const __m128 xo = _mm_load_ps(&pos[i][pos_w]);
						const __m128 xe = _mm_sub_ps(xo,xx);
						_mm_store_ps(&pos[i][pos_w], xe);
					}
				}
			}
		for(eU32 i = 0; i < 3; i++)
			this->set_bnd(LQ_B_X + i,pos[i], dt);
	}
}

void	
eStableFluids::set_bnd(eU32 b, eF32* x0, eF32 dt) {
	__m128 cellmask;
	cellmask.m128_f32[0] = (this->m_numCells[0] > 1) ? 1.0f : 0.0f; 
	cellmask.m128_f32[1] = (this->m_numCells[1] > 1) ? 1.0f : 0.0f; 
	cellmask.m128_f32[2] = (this->m_numCells[2] > 1) ? 1.0f : 0.0f; 
	cellmask.m128_f32[3] = 0.0f; 
	cellmask = _mm_cmpgt_ps(cellmask, _mm_setzero_ps());

	ePROFILER_ZONE("Fluid Sim - Set Boundary");
	{
		ePROFILER_ZONE("Fluid Sim - Set Boundary Static");
		// MODE: 0 = Faces 1 = Edges 2 = Corners
		for(eU32 mode = 0; mode < 3; mode++) {
			// set boundaries with flat z axis
			for(eU32 flatDim = 0; flatDim < 3; flatDim++) {
				eF32 reflect = (b - LQ_B_X == flatDim) ? (this->m_hasBorder ? -1.0f : 1.0f) : 
													((b >= LQ_B_DENS) && (b <= LQ_B_DENS_MAX) ? (m_hasBorder ? 1.0f : 1.0f) : 
															(b == LQ_B_CLEAR) ? 0.0f : 1.0f);
				eU32 coDim0 = (flatDim == 0) ? 1 : 0;
				eU32 coDim1 = (flatDim == 0) ? 2 : 3 - flatDim;
		
				const __m128 reflectreg = _mm_set1_ps(reflect);

				eU32 coords[4];
				eS32 offset = this->m_offsetstep[flatDim];
				switch(mode) {
					case 0: // FACES
						if(flatDim == 0) {
							for(eU32 z = 1; z <= this->m_numCells[2]; z++) {
								for(eU32 y = 1; y <= this->m_numCells[1] - 1; y += 2) {
									eU32 w0 = IX(0,y,z);
									const __m128 val0 = _mm_load_ss(&x0[w0 + 1]);
									const __m128 val1 = _mm_load_ss(&x0[w0 + this->m_numCells[0]]);
									const __m128 val2 = _mm_load_ss(&x0[w0 + this->m_offsetstep[1] + 1]);
									const __m128 val3 = _mm_load_ss(&x0[w0 + this->m_offsetstep[1] + this->m_numCells[0]]);

									const __m128 part0 = _mm_shuffle_ps(val0,val1,_MM_SHUFFLE(0,0,0,0));
									const __m128 part1 = _mm_shuffle_ps(val2,val3,_MM_SHUFFLE(0,0,0,0));
									const __m128 gval = _mm_shuffle_ps(part0, part1, _MM_SHUFFLE(2,0,2,0));
									const __m128 result = _mm_mul_ps(gval, reflectreg);
									eASSERT(w0 + this->m_numCells[0] + 1 == IX(this->m_numCells[0] + 1, y, z));
									eASSERT(w0 + this->m_offsetstep[1] == IX(0, y+1, z));
									_mm_store_ss(&x0[w0], result);
									_mm_store_ss(&x0[w0 + this->m_numCells[0] + 1], _mm_shuffle_ps(result, result, _MM_SHUFFLE(0,0,0,1)));
									_mm_store_ss(&x0[w0 + this->m_offsetstep[1]], _mm_shuffle_ps(result, result, _MM_SHUFFLE(0,0,0,2)));
									_mm_store_ss(&x0[w0 + this->m_offsetstep[1] + this->m_numCells[0] + 1], _mm_shuffle_ps(result, result, _MM_SHUFFLE(0,0,0,3)));
								}
							} 
						} else {
//							if(this->m_numCells[flatDim] > 1) 
							{
								for(coords[flatDim] = 0; coords[flatDim] <= this->m_numCells[flatDim] + 1; coords[flatDim] += this->m_numCells[flatDim] + 1) {
									for(coords[coDim1] = 0; coords[coDim1] <= this->m_numCells[coDim1] + 1; coords[coDim1]++) {
										eF32* dataPtr = &x0[IX(1, coords[1], coords[2])];
										const eF32* target = dataPtr + this->m_numCells[0];
										for(; dataPtr < target; dataPtr += 4) {
											const __m128 val = _mm_load_ps(dataPtr + offset);
											_mm_store_ps(dataPtr, _mm_mul_ps(val, reflectreg));
										}
									}
									offset = (eU32)-(eInt)this->m_offsetstep[flatDim];
								}
							}
						}
						break;
/*
					case 1: // EDGES

						coords[coDim0] = 0;
						coords[coDim1] = 0;
						for(coords[flatDim] = 0; coords[flatDim] <= this->m_numCells[flatDim] + 1; coords[flatDim]++) {
							eU32 w = IX(coords[0], coords[1], coords[2]);
							x0[w] = (x0[w + this->m_offsetstep[coDim0]] + x0[w + this->m_offsetstep[coDim1]]) * 0.5f;

							eU32 wd0 = w + (this->m_numCells[coDim0] + 1) * this->m_offsetstep[coDim0];
							x0[wd0] = (x0[wd0 - this->m_offsetstep[coDim0]] + x0[wd0 + this->m_offsetstep[coDim1]]) * 0.5f;

							eU32 wd1 = w + (this->m_numCells[coDim1] + 1) * this->m_offsetstep[coDim1];
							x0[wd1] = (x0[wd1 + this->m_offsetstep[coDim0]] + x0[wd1 - this->m_offsetstep[coDim1]]) * 0.5f;

							eU32 wd2 = w + (this->m_numCells[coDim0] + 1) * this->m_offsetstep[coDim0] + (this->m_numCells[coDim1] + 1) * this->m_offsetstep[coDim1];
							x0[wd2] = (x0[wd2 - this->m_offsetstep[coDim0]] - x0[wd2 - this->m_offsetstep[coDim1]]) * 0.5f;

							eASSERT((w >= 0) && (w < this->m_gcells));
							eASSERT((wd0 >= 0) && (wd0 < this->m_gcells));
							eASSERT((wd1 >= 0) && (wd1 < this->m_gcells));
							eASSERT((wd2 >= 0) && (wd2 < this->m_gcells));
						}
						break;
					case 2: // CORNERS

						for(eU32 z = 0; z < 2; z++)
							for(eU32 y = 0; y < 2; y++)
								for(eU32 x = 0; x < 2; x++) {
									eU32 w = IX(x * (this->m_numCells[0] + 1),y * (this->m_numCells[1] + 1),z * (this->m_numCells[2] + 1));
									x0[w] = (x0[w + this->m_offsetstep[0] * (1 - x * 2)] + x0[w + this->m_offsetstep[1] * (1 - y * 2)] + x0[w + this->m_offsetstep[2] * (1 - z * 2)]) * (1.0f / 3.0f);
								}

						break;
/**/
				}
			}
		}

	}
/**/
	if(this->m_boundMesh) {
		if((b >= LQ_B_DENS) && (b <= LQ_B_DENS_MAX)) {

  			ePROFILER_ZONE("Fluid Sim - Set Boundary Bound Mesh B4");
			const __m128 reflectConst = _mm_set1_ps(-1.0f);

			__m128 craw;
			for(eU32 v = 0; v < this->m_boundMesh->getVertexCount(); v++) {
				if(this->convertGlobalPosToGridPos_AndTestIfOutside(_mm_loadu_ps(&this->m_boundMesh->getVertex(v)->position.x), craw))
					continue;

				// lookup target point
				const __m128 targetPos = this->clampGridPos(_mm_add_ps(craw, _mm_and_ps(cellmask, _mm_loadu_ps(&this->m_boundMesh->getVertex(v)->normal.x))));
				const __m128 localVal = this->get_linear_interpolated(craw, x0);
				const __m128 targetVal = this->get_linear_interpolated(targetPos, x0);
				const __m128 newval = _mm_add_ps(targetVal, localVal);
//				this->set_linear_interpolated(targetPos, newval, x0);
//				this->set_linear_interpolated(craw, _mm_setzero_ps(), x0);
//				this->set_linear_interpolated(targetPos, _mm_add_ps(targetVal, localVal), x0);
//				const __m128 newval = _mm_mul_ps(this->get_linear_interpolated(targetPos, x0), reflectConst);

			}

		} else if((b >= LQ_B_X) && (b <= LQ_B_Z) && (this->m_numCells[b - LQ_B_X] > 1)) {

  			ePROFILER_ZONE("Fluid Sim - Set Boundary Bound Mesh B1-3");
			__m128 reflectConst = _mm_set1_ps(1.0f);
			__m128 evNormal = _mm_setzero_ps();
			evNormal.m128_f32[b - LQ_B_X] = -1.0f;
			reflectConst = _mm_setzero_ps();
			__m128 craw;
			for(eU32 v = 0; v < this->m_boundMesh->getVertexCount(); v++) {
				if(this->convertGlobalPosToGridPos_AndTestIfOutside(_mm_loadu_ps(&this->m_boundMesh->getVertex(v)->position.x), craw))
					continue;

				const __m128 m = _mm_and_ps(cellmask, _mm_loadu_ps(&this->m_boundMesh->getVertex(v)->normal.x));
				const __m128 normal = eStableFluids::normalize(m);

				const __m128 dotp = _mm_mul_ps(normal, evNormal);
				const __m128 dotpbidim = _mm_hadd_ps(dotp, dotp);
				const __m128 dotpscalar = _mm_hadd_ps(dotpbidim, dotpbidim);
				const __m128 reflect = _mm_sub_ps(reflectConst, dotpscalar);

				const __m128 targetPos = clampGridPos(_mm_add_ps(craw, normal));
				const __m128 targetval = this->get_linear_interpolated(targetPos, x0);
				const __m128 newval = _mm_mul_ps(targetval, reflect);
				this->set_linear_interpolated(craw, newval, x0);
			}

		}
	}
/**/
}

void	
eStableFluids::add_source(eU32 b, eF32* x, eF32 dt) {
//	return;
	if(this->m_forceMesh && (b >= LQ_B_X) && (b <= LQ_B_Z) && (this->m_numCells[b - LQ_B_X] > 1)) {

		ePROFILER_ZONE("Fluid Sim - Add Source Force");
		__m128 forceScale = _mm_set1_ps(this->m_forceAmount * dt);
		__m128 craw;
		for(eU32 v = 0; v < this->m_forceMesh->getVertexCount(); v++) {
			if(this->convertGlobalPosToGridPos_AndTestIfOutside(_mm_loadu_ps(&this->m_forceMesh->getVertex(v)->position.x), craw))
				continue;

			const __m128 newval = _mm_mul_ps(_mm_set1_ps(this->m_forceMesh->getVertex(v)->normal[b - LQ_B_X]), forceScale);
			this->set_linear_interpolated(craw, newval, x);
		}
	}
	if((b >= LQ_B_DENS) && (b <= LQ_B_DENS_MAX)) {

		ePROFILER_ZONE("Fluid Sim - Add Source Dens");
		eU32 m = b - LQ_B_DENS;
		__m128 amount = _mm_set1_ps(this->m_emissionMeshes[m]->getVertex(this->m_emissionMeshes[m]->getVertexCount() - 1)->normal.x * dt);
		__m128 craw;
		eU32 vcnt = this->m_emissionMeshes[m]->getVertexCount() - 1;
		for(eU32 v = 0; v < vcnt; v++) {
			if(this->convertGlobalPosToGridPos_AndTestIfOutside(_mm_loadu_ps(&this->m_emissionMeshes[m]->getVertex(v)->position.x), craw))
				continue;

			const __m128 newval = amount;
//			this->set_linear_interpolated(craw, newval, x);
			this->set_linear_interpolated(craw, _mm_add_ps(newval, this->get_linear_interpolated(craw, x)), x);
		}

		const eF32 vanish = this->m_emissionMeshes[m]->getVertex(this->m_emissionMeshes[m]->getVertexCount() - 1)->normal.z;
		const eF32 vanishMul = 1.0f - eMin(vanish * dt, 1.0f);
		
		// clamp [0,1]
		eU32 wi = IX(1,0,0);
		eU32 w = wi;
		for(eU32 i = 0; i < this->m_gcells - wi; i += 4, w += 4) {
			const __m128 src = _mm_mul_ps(_mm_set1_ps(vanishMul), _mm_load_ps(&x[w]));
			const __m128 result = _mm_min_ps(_mm_set1_ps(1.0), _mm_max_ps(_mm_set1_ps(-1.0f), src));
			_mm_store_ps(&x[w], result);
		}
	}
}


void	
eStableFluids::simulate(eF32 dt) {

	ePROFILER_ZONE("Fluid Sim");
	{
		ePROFILER_ZONE("Fluid Sim - Vel Step");
		this->vel_step(&m_field[LQ_FIELD_U], &m_field[LQ_FIELD_UPREV], this->m_viscosity, dt);
	}
	{
		ePROFILER_ZONE("Fluid Sim - Dens Step");
		if(this->m_emissionMeshes != eNULL)  {
			for(eU32 i = 0; i < this->m_emitterCnt; i++)
				this->dens_step(LQ_B_DENS + i, m_field[LQ_FIELD_DENSITY + i], m_field[LQ_FIELD_DENSITYPREV + i], &m_field[LQ_FIELD_U], 
					            this->m_emissionMeshes[i]->getVertex(this->m_emissionMeshes[i]->getVertexCount()-1)->normal.y, 
								dt);
		}
	}
	{
		ePROFILER_ZONE("Fluid Sim - Post");
		// clean buffers for next iteration
		for(eU32 i = 0; i < this->m_emitterCnt; i++) 
			eMemSet(m_field[LQ_FIELD_DENSITYPREV + i],0, sizeof(eF32) * this->m_gcells);
	}
}

void	
eStableFluids::evaluateDensGrid(eColor* bitmap, eU32 width, eU32 height) const {
	ePROFILER_ZONE("Fluid Sim - RenderBitmap");

	__declspec(align(16)) eVector3 colorArray[16];
	for(eU32 d = 0; d < this->m_emitterCnt; d++) 
		colorArray[d] = this->m_emissionMeshes[d]->getVertex(this->m_emissionMeshes[d]->getVertexCount() - 1)->position;

	eU32 rwidth = eMin(width, this->m_numCells[0]);
	eU32 rheight = eMin(height, this->m_numCells[1]);
	eColor* bm = (eColor*)bitmap;
	for(eU32 y = 0; y < rheight; y++) {
		eU32 tp = IX(1, rheight - y + 1, 1);
		for(eU32 x = 0; x < rwidth; x++, bm++, tp++) {
			//dens
			__m128 colorsum = _mm_setzero_ps();
			for(eU32 d = 0; d < this->m_emitterCnt; d++) {
				const __m128 psumg = _mm_load_ss(&m_field[LQ_FIELD_DENSITY + d][tp]);
				const __m128 color = _mm_load_ps(&colorArray[d].x);

				const __m128 densclamped = _mm_max_ps(_mm_setzero_ps(), _mm_min_ps(_mm_set1_ps(1.0f), _mm_set1_ps(psumg.m128_f32[0])));
				colorsum = _mm_add_ps(colorsum, _mm_mul_ps(color, densclamped));
			}

			const __m128 sumclamped = _mm_max_ps(_mm_setzero_ps(), _mm_min_ps(_mm_set1_ps(1.0f), colorsum));
			bm->setRedF(sumclamped.m128_f32[0]);
			bm->setGreenF(sumclamped.m128_f32[1]);
			bm->setBlueF(sumclamped.m128_f32[2]);
			bm->setAlphaF(sumclamped.m128_f32[3]);
/*
			const __m128 sumsat = _mm_mul_ps(sumclamped, _mm_set1_ps(eU16_MAX));
			const __m128i sumi = _mm_cvtps_epi32(sumsat);
			const __m128i sumsati = _mm_packus_epi32(sumi, sumi);
			_mm_storel_epi64(((__m128i*)bm), sumsati);
*/
		}
		bm += width - rwidth;
	}
}
