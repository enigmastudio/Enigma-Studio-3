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

#ifndef DYNAMICS_HPP
#define DYNAMICS_HPP

class ApproxNode
{
public:
	ApproxNode(KD_Node* node);
	eF32		EvaluateFast3DimGrade2(const eVector3* position, eVector3* outNormal) const;
	eAABB				m_boundBox;
	ApproxNode*			m_left;
	ApproxNode*			m_right;
	TensorProductSpace*	m_space;
};

class Approximation
{
public:
	Approximation(eU32 gridCnt, eU32 sampleCnt, eF32* samples, eF32* normals);
	double EvaluateFast3DimGrade2(const eVector3* position, eVector3* outNormal) const;
//private:
	static eF32 Wendland(eF32 d, eF32 h)
	{
		if(d < 0.0)
			d = 0.0;
		if(d > h)
			d = h;
		eF32 a = (eF32)(1.0 - d / h);
		return (eF32)((a * a * a * a) * (4.0 * d / h + 1.0));
	}

	ApproxNode	*root;
//	TensorProductSpace*		m_spaces;
//	eU32					m_gridCnt;
//	eAABB					m_boundBox;
//	eVector3				m_step;
};

#endif // DYNAMICS_HPP