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

#ifndef TENSORPRODSPACE_HPP
#define TENSORPRODSPACE_HPP

/**************************************
/* The NewMat10 include files         */
#include "newmat/include.h"
#include "newmat/newmat.h"
#include "newmat/newmatap.h"
#include "newmat/newmatio.h"
/***************************************/

class TensorProductSpace
{
public:
	void Calculate(eU32 sampleCount, eF32* samples, eF32* sampleValues);
	double EvaluateFast3DimGrade2(const eVector3* position, eVector3* outNormal) const;
	~TensorProductSpace();
private:
	double*			m_approxValues;
	eBool			m_isDefined;
};


#endif // TENSORPRODSPACE_HPP