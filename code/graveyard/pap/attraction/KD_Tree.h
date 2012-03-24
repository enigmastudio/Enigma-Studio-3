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

#ifndef KD_Tree_H
#define KD_Tree_H

struct KD_Lookup_Entry;
class KD_Node;

typedef struct KD_Item
{
	eVector3	pos;
	void*		objectData;
} KD_Item;

//------------------------------------------------------------------------------
class eKD_Tree
{
public:
	/// Constructor
	eKD_Tree();
	/// Destructor
	~eKD_Tree();

	void InsertPoints(KD_Item* points, int nrOfPoints);
	void findKNearest(const eVector3* pivot, eU32 k, KD_Lookup_Entry* result, eU32 &resultCnt) const;
	void CollectInRadius(const eVector3* pivot, eF32 radiusSquared, KD_Lookup_Entry* result, eU32& resultCnt) const;

	KD_Node* root;
	KD_Item*		m_pointsRaw;
	eU32			m_nrOfPoints;
protected:
private:
};


struct KD_Lookup_Entry
{
	eF32			distanceSquared;
	KD_Item*		element;
};

//------------------------------------------------------------------------------
class KD_Node// : public KD_Item
{
public:
	KD_Node()
	{
		this->m_leftBranch = 0;
		this->m_rightBranch = 0;
		this->m_bucket = 0;
		this->m_bucketSize = 0;
	};
	
	void	insertPoints(KD_Item** points, eU32 nrOfPoints);
	void	findKNearest(const eVector3* pivot, eU32 k, KD_Lookup_Entry* result, eU32 &k_now) const;
	void    collectInRadius(const eVector3* pivot, eF32 radiusSquared, KD_Lookup_Entry* result, eU32& resultCnt) const;

	eAABB					m_boundBox;

	eF32					m_radiusSquared;
	KD_Node*				m_leftBranch;
	KD_Node*				m_rightBranch;
	KD_Item**				m_bucket;
	eU32					m_bucketSize;
};

//------------------------------------------------------------------------------
#endif
