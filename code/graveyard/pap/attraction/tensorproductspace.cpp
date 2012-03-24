/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *   This file is part of
 *       _______   ______________  ______     _____
 *      / ____/ | / /  _/ ____/  |/  /   |   |__  /
 *     / __/ /  |/ // // / __/ /|_/ / /| |    /_ <
 *    / /___/ /|  // // /_/ / /  / / ___ |  ___/ /
 *   /_____/_/ |_/___/\____/_/  /_/_/  |_| /____/.
 *
 *   Copyright © 2003-2008 Brain Control, all rights reserved.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "../eshared.hpp"

#define TENSORDIM 4

void
TensorProductSpace::Calculate(eU32 sampleCount, eF32* samples, eF32* sampleValues)
{

	// now make sure that we have at least tensorBasesCount samples
	eASSERT(sampleCount >= TENSORDIM);

	// initialize matrices
	Matrix sum_Bi(TENSORDIM,TENSORDIM);
	ColumnVector sum_b(TENSORDIM);
	for(eU32 i = 1; i <= TENSORDIM; i++)
	{
		sum_b(i) = 0.0;
		for(eU32 j = 1; j <= TENSORDIM; j++)
			sum_Bi(i,j) = 0.0;
	};


	// now solve the linear system
	ColumnVector b(TENSORDIM);
	for(eU32 i = 0; i < sampleCount; i++)
	{
		eU32 offset = i * (3 + 1);

		b(1) = 1.0;
		b(2) = samples[offset + 1];
		b(3) = samples[offset + 2];
		b(4) = samples[offset + 3];
/*		b(5) = samples[offset + 1] * samples[offset + 2];
		b(6) = samples[offset + 1] * samples[offset + 3];
		b(7) = samples[offset + 2] * samples[offset + 3];
		b(8) = samples[offset + 1] * samples[offset + 1];
		b(9) = samples[offset + 2] * samples[offset + 2];
		b(10) = samples[offset + 3] * samples[offset + 3];
*/
		Matrix Bi = b * b.t();
		sum_Bi += Bi * samples[offset];
		sum_b += b * (sampleValues[i] * samples[offset]);

	};

	double det = 0.0;
	try
	{
		det = sum_Bi.Determinant();
	}
		catch(Exception E)
		{
			int k = 4;
			det = 0.0;
		};

	if(det != 0.0)
	{
		ColumnVector approximation(TENSORDIM);
		approximation = sum_Bi.i() * sum_b;
		this->m_approxValues = new double[TENSORDIM];
		for(eU32 i = 0; i < TENSORDIM; i++)
			this->m_approxValues[i] = approximation(i+1);

		this->m_isDefined = true;
	}
	else
	{
		this->m_isDefined = false;
	};

}

TensorProductSpace::~TensorProductSpace()
{
	delete this->m_approxValues;
}

double 
TensorProductSpace::EvaluateFast3DimGrade2(const eVector3* position, eVector3* outNormal) const
{
	if(this->m_isDefined)
	{
		outNormal->set(this->m_approxValues[1],
			           this->m_approxValues[2],
					   this->m_approxValues[3]);
/*		outNormal->set(this->m_approxValues[1] + this->m_approxValues[4] * position->y + this->m_approxValues[5] * position->z + 2.0 * this->m_approxValues[7] * position->x,
			           this->m_approxValues[2] + this->m_approxValues[4] * position->x + this->m_approxValues[6] * position->z + 2.0 * this->m_approxValues[8] * position->y,
					   this->m_approxValues[3] + this->m_approxValues[5] * position->x + this->m_approxValues[6] * position->y + 2.0 * this->m_approxValues[9] * position->z);
					   */
		*outNormal *= 1.0f / outNormal->length();
		return this->m_approxValues[0] +
						this->m_approxValues[1] * position->x +
						this->m_approxValues[2] * position->y +
						this->m_approxValues[3] * position->z;
/*						this->m_approxValues[4] * position->x * position->y +
						this->m_approxValues[5] * position->x * position->z +
						this->m_approxValues[6] * position->y * position->z +
						this->m_approxValues[7] * position->x * position->x +
						this->m_approxValues[8] * position->y * position->y +
						this->m_approxValues[9] * position->z * position->z;
*/
	}
	outNormal->x = 0;
	outNormal->y = 0;
	outNormal->z = 0;
	return 0;
}