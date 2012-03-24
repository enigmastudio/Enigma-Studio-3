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


ApproxNode::ApproxNode(KD_Node* node)
{
	this->m_boundBox = node->m_boundBox;
	this->m_space = new TensorProductSpace();
//	eF32 radius = this->m_boundBox.getSizeVector().length() * 0.5f;

	eF32*				pointBuffer = new eF32[(3 + 1) * (node->m_bucketSize * 3 + 1)];
	eF32*				valueBuffer = new eF32[node->m_bucketSize * 3 + 1];

	eU32 p=0;
	eU32 v=0;

	for(eU32 i = 0; i < node->m_bucketSize; i++)
	{
		for(eS32 d = -1; d <= 1; d++)
		{
			eF32 epsilon = (eF32)d * 0.00001;
			KD_Item* item = node->m_bucket[i];
			pointBuffer[p++] = 1.0f;
//			eF32 dist = (item->pos - this->m_boundBox.getCenter()).length();
//			pointBuffer[p++] = Approximation::Wendland(dist, radius);
			pointBuffer[p++] = item->pos.x + *(((eF32*)item->objectData) + 0) * epsilon;
			pointBuffer[p++] = item->pos.y + *(((eF32*)item->objectData) + 1) * epsilon;
			pointBuffer[p++] = item->pos.z + *(((eF32*)item->objectData) + 2) * epsilon;
			valueBuffer[v++] = epsilon;
		}
	}
	this->m_space->Calculate(v, pointBuffer, valueBuffer);
	this->m_left = 0;
	if(node->m_leftBranch)		this->m_left = new ApproxNode(node->m_leftBranch);
	this->m_right = 0;
	if(node->m_rightBranch)		this->m_right = new ApproxNode(node->m_rightBranch);
}

eF32
ApproxNode::EvaluateFast3DimGrade2(const eVector3* position, eVector3* outNormal) const
{
/*
	if(this->m_boundBox.contains(*position))
	{

	}
*/
/*
	if(this->m_left)
	{
		eF32 leftDistSqr =  (*position - this->m_left->m_boundBox.getCenter()).sqrLength();
		eF32 rightDistSqr = (*position - this->m_right->m_boundBox.getCenter()).sqrLength();
		if(leftDistSqr < rightDistSqr)
			return this->m_left->EvaluateFast3DimGrade2(position, outNormal);
		else
			return this->m_right->EvaluateFast3DimGrade2(position, outNormal);
	}
	else
		return this->m_space->EvaluateFast3DimGrade2(position, outNormal);
*/
	if((this->m_left) && (this->m_left->m_boundBox.contains(*position)))
		return this->m_left->EvaluateFast3DimGrade2(position, outNormal);
	else
	if((this->m_right) && (this->m_right->m_boundBox.contains(*position)))
		return this->m_right->EvaluateFast3DimGrade2(position, outNormal);
	else
		return this->m_space->EvaluateFast3DimGrade2(position, outNormal);
}

Approximation::Approximation(eU32 gridCnt, eU32 sampleCnt, eF32* samples, eF32* normals)
{
//	this->m_gridCnt = gridCnt;

//	m_boundBox.clear();
	eKD_Tree* kdTree = new eKD_Tree();
	KD_Item* sampleItems = new KD_Item[sampleCnt];
	for(eU32 i = 0; i < sampleCnt; i++)
	{
		sampleItems[i].pos = eVector3(samples[i * 3 + 0], samples[i * 3 + 1], samples[i * 3 + 2]);
		sampleItems[i].objectData = &normals[i * 3];
//		m_boundBox.updateExtentsFast(sampleItems[i].pos);
	}
//	m_boundBox.updateExtentsFinalize();
	kdTree->InsertPoints(sampleItems, sampleCnt);
	this->root = new ApproxNode(kdTree->root);

/*
	KD_Lookup_Entry*	queryBuffer = new KD_Lookup_Entry[sampleCnt];
	eF32*				pointBuffer = new eF32[(3 + 1) * (sampleCnt * 3 + 1)];
	eF32*				valueBuffer = new eF32[sampleCnt * 3 + 1];

	this->m_spaces = new TensorProductSpace[gridCnt*gridCnt*gridCnt]; 
	m_step = m_boundBox.getSizeVector() / (eF32)(gridCnt);
	int t = 0;
	for(eU32 z = 0; z < gridCnt; z++)
		for(eU32 y = 0; y < gridCnt; y++)
			for(eU32 x = 0; x < gridCnt; x++)
			{
				eVector3 pos0((eF32)x * m_step.x, (eF32)y * m_step.y, (eF32)z * m_step.z);
				pos0 += m_boundBox.getMinimum();
				eVector3 pos1 = pos0 + m_step;
				
				eU32 p = 0;
				eU32 v = 0;

				eU32 minSamples = 10;
				eU32 cnt = 0;
				eVector3 pivot = (pos0 + pos1) * 0.5;
				eF32 radiusSquared = (m_step * 0.5).sqrLength();
				kdTree->CollectInRadius(&pivot, radiusSquared, queryBuffer, cnt);
				if(cnt < minSamples)
				{
					kdTree->findKNearest(&pivot, minSamples, queryBuffer, cnt);
					radiusSquared = queryBuffer[cnt-1].distanceSquared;

					pointBuffer[p++] = 1;
					pointBuffer[p++] = pivot.x;
					pointBuffer[p++] = pivot.y;
					pointBuffer[p++] = pivot.z;
					eVector3 normal(*(((eF32*)queryBuffer[0].element->objectData) + 0),
									*(((eF32*)queryBuffer[0].element->objectData) + 1),
									*(((eF32*)queryBuffer[0].element->objectData) + 2));
//					valueBuffer[v++] = ((pivot - queryBuffer[0].element->pos).normalized() * normal.normalized()) * eSqrt(queryBuffer[0].distanceSquared);
					valueBuffer[v++] = ((pivot - queryBuffer[0].element->pos) * normal);

				}

				for(eU32 i = 0; i < cnt; i++)
				{
					for(eS32 d = -1; d <= 1; d++)
					{
						eF32 epsilon = (eF32)d * 0.001;
//						pointBuffer[p++] = Wendland(eSqrt(queryBuffer[i].distanceSquared), eSqrt(radiusSquared));
						pointBuffer[p++] = 1.0f;
						pointBuffer[p++] = queryBuffer[i].element->pos.x + *(((eF32*)queryBuffer[i].element->objectData) + 0) * epsilon;
						pointBuffer[p++] = queryBuffer[i].element->pos.y + *(((eF32*)queryBuffer[i].element->objectData) + 1) * epsilon;
						pointBuffer[p++] = queryBuffer[i].element->pos.z + *(((eF32*)queryBuffer[i].element->objectData) + 2) * epsilon;
						valueBuffer[v++] = epsilon;
					}
				}

				this->m_spaces[t].Calculate(v, pointBuffer, valueBuffer);
				t++;
			}
//	this->m_spaces = new TensorProductSpace(grade, dimensionality, sampleC
	this->m_step.x = 1.0 / this->m_step.x;
	this->m_step.y = 1.0 / this->m_step.y;
	this->m_step.z = 1.0 / this->m_step.z;
*/
}

double 
Approximation::EvaluateFast3DimGrade2(const eVector3* position, eVector3* outNormal) const
{
	eF32 result = this->root->EvaluateFast3DimGrade2(position, outNormal);

	eF32 radiusSqr = (this->root->m_boundBox.getSizeVector() * 0.5).sqrLength();
	eF32 distSqr = (this->root->m_boundBox.getCenter() - *position).sqrLength();
	if(distSqr > radiusSqr)
	{
		eVector3 normal2 = (*position - this->root->m_boundBox.getCenter());
		eF32 dist2 = normal2.length();
		normal2 *= 1.0f / dist2;
		eF32 amount = radiusSqr / distSqr;

		result = result * amount + dist2 * (1.0 - amount);
		*outNormal = (*outNormal * amount) + (normal2 * (1.0f - amount));
	}

	return result;
/*
	eVector3 posRel = (*position - this->m_boundBox.getMinimum());
	posRel.x *= this->m_step.x;
	posRel.y *= this->m_step.y;
	posRel.z *= this->m_step.z;
	eU32 x = eClamp((eU32)0, (eU32)posRel.x, this->m_gridCnt - 1);
	eU32 y = eClamp((eU32)0, (eU32)posRel.y, this->m_gridCnt - 1);
	eU32 z = eClamp((eU32)0, (eU32)posRel.z, this->m_gridCnt - 1);
	int t = (z * this->m_gridCnt + y) * this->m_gridCnt + x;
	return this->m_spaces[t].EvaluateFast3DimGrade2(position, outNormal);
*/
}