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

#ifndef KDTREE_HPP
#define KDTREE_HPP

class eKDTree 
{
public:
	eKDTree() {};

	void reconstruct(eSceneData& sceneList);
	void cull(eU32 passID, const eCamera& cam, eRenderJobPtrArray& jobs);
    
	struct Node {
        Node*                       children[2];
		eAABB						m_bbox;
		eU32						renderableCnt;
        eBool                       m_isFinal;
	};
	const eAABB& cull(Node& node, eSceneData::Entry** itemPtr, const eCamera& cam, eRenderJobPtrArray& jobs);

	eArray<Node>	                m_nodes;
	eArray<eSceneData::Entry*>		m_items;
	eArray<eSceneData::Entry>	    m_itemsRaw;
	Node	                        m_root;
    eU32                            m_passID; // set on each culling

private:
    void    expandItem(eSceneData::Entry** S);
    eU32    selectMedianAndSort(eU32 axis, eU32 k, eSceneData::Entry** S, eU32 renderableCount);
    void    finalizeLeaf(eSceneData::Entry** itemPtr, eKDTree::Node& node);
    void    expandAndCalculateBBox(eSceneData::Entry** itemPtr, eU32 count, eAABB& bbox);


};

#endif // KDTREE_HPP