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
#include "../eshared.hpp"

#define KD_MAX_POINTS_IN_BUCKET 20

//------------------------------------------------------------------------------
/**
*/
eKD_Tree::eKD_Tree()
{
	root = 0;
}

//------------------------------------------------------------------------------
/**
*/
eKD_Tree::~eKD_Tree()
{
}


//------------------------------------------------------------------------------
/**
*/
void
eKD_Tree::InsertPoints(KD_Item* points, int nrOfPoints)
{
	this->m_pointsRaw = points;
	this->m_nrOfPoints = nrOfPoints;
	KD_Item** pointList = new KD_Item*[this->m_nrOfPoints];
	for(eU32 i = 0; i < this->m_nrOfPoints; i++)
		pointList[i] = &this->m_pointsRaw[i];
	if(this->root == 0)
	{
		this->root = new KD_Node();
		this->root->insertPoints(pointList, this->m_nrOfPoints);
	};
}

//------------------------------------------------------------------------------
/**
*/
void
eKD_Tree::findKNearest(const eVector3* pivot, eU32 k, KD_Lookup_Entry* result, eU32 &resultCnt) const
{
	resultCnt = 0;
	this->root->findKNearest(pivot, k, result, resultCnt);
/*
	for(eU32 t = 0; t < k; t++)
	{
		KD_Item* best = 0;
		eF32 bestDist = 0;
		for(eU32 i = 0; i < this->m_nrOfPoints; i++)
		{
			bool found = false;
			for(eU32 a = 0; a < t; a++)
				if(&this->m_pointsRaw[i] == result[a].element)
					found = true;
			if(!found)
			{
				eF32 dist = (this->m_pointsRaw[i].pos - *pivot).sqrLength();
				if((best == 0) || (dist < bestDist))
				{
					best = &this->m_pointsRaw[i];
					bestDist = dist;
				}
			}
		}
		if(best != result[t].element)
		{
			eVector3 bla(0,0,0);
			eF32 l = (result[t].element->pos - *pivot).sqrLength();
			if(l != bestDist)
			{
				eVector3 blub(0,0,0);
			}
		}
	}
*/
};



//------------------------------------------------------------------------------
/**
*/
void
eKD_Tree::CollectInRadius(const eVector3* pivot, eF32 radiusSquared, KD_Lookup_Entry* result, eU32& resultCnt) const
{
	resultCnt = 0;
	this->root->collectInRadius(pivot, radiusSquared, result, resultCnt);
};

//------------------------------------------------------------------------------
/**
*/
void 
KD_Node::insertPoints(KD_Item** points, eU32 nrOfPoints)
{
	this->m_bucket = new KD_Item*[nrOfPoints];
	this->m_bucketSize = nrOfPoints;
	// create Boundbox
	for(eU32 i = 0; i < nrOfPoints; i++)
	{
		this->m_bucket[i] = points[i];
		this->m_boundBox.updateExtentsFast(points[i]->pos);
	}
	this->m_boundBox.updateExtentsFinalize();
	eU32 splitAxis = this->m_boundBox.getLargestDimension();

	if(nrOfPoints <= KD_MAX_POINTS_IN_BUCKET)
	{
		delete [] points;
//		this->m_bucket = points;
//		this->m_bucketSize = nrOfPoints;
	}
	else
	{
		eU32 rank = (nrOfPoints - 1) >> 1;
		KD_Item** left = new KD_Item*[nrOfPoints];
		KD_Item** right = new KD_Item*[nrOfPoints];
		eU32 pLeft = 0;
		eU32 pRight = 0;

		KD_Item** smaller = new KD_Item*[nrOfPoints];
		KD_Item** bigger = new KD_Item*[nrOfPoints];
		KD_Item** mid = new KD_Item*[nrOfPoints];

		KD_Item** remaining = points;
		eU32 cntRemaining = nrOfPoints;
		while(cntRemaining)
		{
			eU32 pSmaller = 0;
			eU32 pMid = 0;
			eU32 pBigger = 0;

//			eF32 splitVal = remaining[eRandom(0,cntRemaining)]->pos[splitAxis];
			eF32 splitVal = remaining[0]->pos[splitAxis];
			for(eU32 i = 0; i < cntRemaining; i++)
			{
				eF32 val = remaining[i]->pos[splitAxis];
				if(val < splitVal)
					smaller[pSmaller++] = remaining[i];
				else if (val > splitVal)
					bigger[pBigger++] = remaining[i];
				else
					mid[pMid++] = remaining[i];
			};
			
			if((pSmaller < rank + 1) && (rank < pSmaller + pMid))
			{
				// mid
				memcpy(&left[pLeft], smaller, pSmaller * sizeof(KD_Item*));
				pLeft += pSmaller;
				memcpy(&right[pRight], bigger, pBigger * sizeof(KD_Item*));
				pRight += pBigger;
				
				eU32 midToLeft = rank - pSmaller + 1;
				eU32 midToRight = pMid - midToLeft;
				memcpy(&left[pLeft], mid, midToLeft * sizeof(KD_Item*));
				pLeft += midToLeft;
				memcpy(&right[pRight], &mid[midToLeft], midToRight * sizeof(KD_Item*));
				pRight += midToRight;
				cntRemaining = 0;
			}
			else
			{
				KD_Item** temp = remaining;
				if(rank >= pSmaller + pMid)
				{
					// bigger
					memcpy(&left[pLeft], smaller, pSmaller * sizeof(KD_Item*));
					pLeft += pSmaller;
					memcpy(&left[pLeft], mid, pMid * sizeof(KD_Item*));
					pLeft += pMid;
					rank -= (pSmaller + pMid);
					remaining = bigger;
					bigger = temp;
					cntRemaining = pBigger;
				}
				else
				{
					//smaller
					memcpy(&right[pRight], bigger, pBigger * sizeof(KD_Item*));
					pRight += pBigger;
					memcpy(&right[pRight], mid, pMid * sizeof(KD_Item*));
					pRight += pMid;
					remaining = smaller;
					smaller = temp;
					cntRemaining = pSmaller;
				};
			};
		};

		delete [] smaller;
		delete [] mid;
		delete [] bigger;
		delete [] remaining;

		this->m_leftBranch = new KD_Node();
		this->m_leftBranch->insertPoints(left,pLeft);

		this->m_rightBranch = new KD_Node();
		this->m_rightBranch->insertPoints(right,pRight);
	};

	this->m_radiusSquared = this->m_boundBox.getSizeVector().sqrLength();
};



//------------------------------------------------------------------------------
/**
*/
void    
KD_Node::collectInRadius(const eVector3* pivot, eF32 radiusSquared, KD_Lookup_Entry* result, eU32& resultCnt) const
{
	if(this->m_leftBranch)
	{
		if(this->m_leftBranch->m_boundBox.getDistanceSquared(pivot) <= radiusSquared)
			this->m_leftBranch->collectInRadius(pivot, radiusSquared, result, resultCnt);
		if(this->m_rightBranch->m_boundBox.getDistanceSquared(pivot) <= radiusSquared)
			this->m_rightBranch->collectInRadius(pivot, radiusSquared, result, resultCnt);
	}
	else
	{
		for(eU32 i = 0; i < this->m_bucketSize; i++)
		{
			eF32 dist = (this->m_bucket[i]->pos - *pivot).sqrLength();
			if(dist <= radiusSquared)
			{
				result[resultCnt].element = this->m_bucket[i];
				result[resultCnt].distanceSquared = dist;
				resultCnt++;
			}
		}
	}
}

//------------------------------------------------------------------------------
/**
*/
void
KD_Node::findKNearest(const eVector3* pivot, eU32 k, KD_Lookup_Entry* result, eU32 &k_now) const
{
	if(this->m_bucketSize != 0)
	{
		for(eU32 i = 0; i < this->m_bucketSize; i++)
		{
			eF32 dist = (this->m_bucket[i]->pos - *pivot).sqrLength();
			eU32 pos = k_now;
			while((pos > 0) && (dist < result[pos - 1].distanceSquared))
				pos--;
			if(pos < k)
			{
				eU32 copyPos = k - 1;
				if(copyPos > k_now)
					copyPos = k_now;
				while(copyPos > pos)
				{
					result[copyPos] = result[copyPos - 1];
					copyPos--;
				}

				result[pos].element = this->m_bucket[i];
				result[pos].distanceSquared = dist;

				if(k_now < k)
					k_now++;
			}
		};
	}
	else
	if(this->m_leftBranch != 0)
	{
		KD_Node *first = this->m_leftBranch;
		KD_Node *second = this->m_rightBranch;
		eF32 firstDist = first->m_boundBox.getDistanceSquared(pivot);
		eF32 secondDist = second->m_boundBox.getDistanceSquared(pivot);
		if(firstDist > secondDist)
		{
			first = this->m_rightBranch;
			second = this->m_leftBranch;
			eF32 temp = firstDist;
			firstDist = secondDist;
			secondDist = temp;
		};
		if((k_now < k) || (firstDist < result[k - 1].distanceSquared))
			first->findKNearest(pivot,k,result,k_now);
		if((k_now < k) || (secondDist < result[k - 1].distanceSquared))
			second->findKNearest(pivot,k,result,k_now);
	};
};

//------------------------------------------------------------------------------
