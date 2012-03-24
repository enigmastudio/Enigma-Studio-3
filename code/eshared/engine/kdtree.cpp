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

#include "kdtree.hpp"

#define KD_MAX_ITEMS_PER_NODE 1
//#define KD_MAX_ITEMS_PER_NODE 128
#define KDTREE_CULL_CELLS
#define KDTREE_CULL_LEAFS

void eKDTree::expandItem(eSceneData::Entry** S) {
//	ePROFILER_ZONE("kdTree - Expand");
    
    eSceneData::Entry& parentItem = **S;
	const eSceneData& parentList = *parentItem.renderableList;
    eSceneData::Entry* itemPtr = (*S) - (parentItem.renderableCount - 1);
    eU32 pp = 0;

    eMatrix4x4 parentMatrix(parentItem.matrix);
    for(eU32 i = 0; i < parentList.getEntryCount(); i++) {
		const eSceneData::Entry& e = parentList.getEntry(i);
        if(e.renderableCount > 0) {
            itemPtr += e.renderableCount;
            eSceneData::Entry& item = *(itemPtr - 1);
		    item = e;
            item.matrix *= parentMatrix; 
            eASSERT((item.aabb.getMaximum() - item.aabb.getMinimum()).length() > 0.0f); // empty bound boxes should not occur
            item.aabb.transform(parentMatrix);
            eASSERT((item.aabb.getMaximum() - item.aabb.getMinimum()).length() > 0.0f); // empty bound boxes should not occur
		    S[pp] = &item;
		    pp += item.renderableCount;
		    S[pp - 1] = &item;
        }
	}
}

eU32 eKDTree::selectMedianAndSort(eU32 axis, eU32 k, eSceneData::Entry** S, eU32 renderableCount) 
{
	ePROFILER_ZONE("kdTree - Select");

	if(S[0]->renderableCount == renderableCount)
		return 0;

#ifdef eDEBUG
	// check integrity
	eU32 d_c = 0;
	while(d_c < renderableCount) 
		d_c += S[d_c]->renderableCount;
	eASSERT(d_c == renderableCount);
#endif

	eS32 renderableCountHalf = renderableCount / 2;

    // swap first and second element to avoid taking the same pivot element over and over again
	eU32 idx0 = renderableCount - 1;
    eSceneData::Entry* tmpE = S[idx0];
    S[idx0] = S[idx0 - tmpE->renderableCount];
    eU32 idx1 = idx0 - S[idx0]->renderableCount;
    S[idx1 + 1] = S[idx0];
	S[idx1] = tmpE;
	S[idx1 - tmpE->renderableCount + 1] = tmpE;
    const eF32& medVal = S[idx0]->aabb.getCenter()[axis];

    // TODO: bring back old (smaller) code here
    eS32 lEmptyStart = 0;
    eS32 rEmptyStart = renderableCount - 1 - S[idx0]->renderableCount;

	eSceneData::Entry* curItemL = eNULL;
	eSceneData::Entry* curItemR = S[rEmptyStart];

	eS32 lPos = lEmptyStart;
    eS32 rPos = rEmptyStart - curItemR->renderableCount;

	eS32 dir = 1;
	while((rPos - lPos) * dir >= 0) {
		curItemL = S[lPos];
		lPos += (eS32)curItemL->renderableCount * dir;

        const eF32& cval = curItemL->aabb.getCenter()[axis];
		if((cval > medVal) || (cval == medVal)) {
			const eS32 free = (rEmptyStart - rPos) * dir;
			eASSERT(free >= 0);
			if((eS32)curItemL->renderableCount <= free) {
				S[rEmptyStart] = curItemL;
				rEmptyStart -= curItemL->renderableCount * dir;
				S[rEmptyStart + dir] = curItemL;
			} else {
				S[lEmptyStart] = curItemR;
				lEmptyStart += curItemR->renderableCount * dir;
				S[lEmptyStart - dir] = curItemR;

				dir *= -1;
				eSwap(lPos, rPos);
				eSwap(lEmptyStart, rEmptyStart);
				eSwap(curItemL, curItemR);			
            }
		} else {
			S[lEmptyStart] = curItemL;
			lEmptyStart += curItemL->renderableCount * dir;
			S[lEmptyStart - dir] = curItemL;
		}

	}
	S[lEmptyStart] = curItemR;
	lEmptyStart += curItemR->renderableCount * dir;
	S[lEmptyStart - dir] = curItemR;

	if(dir == -1) {
		eSwap(lPos, rPos);
		eSwap(lEmptyStart, rEmptyStart);
		eSwap(curItemL, curItemR);			
	}

	eU32 l = lEmptyStart;
	eU32 r = renderableCount - lEmptyStart;

	eASSERT(l + r == renderableCount);

	if(l-1 >= k) {
		return selectMedianAndSort(axis, k, S, l);
	}
	else
		if(l == k)
			return l;
		else
			return l + selectMedianAndSort(axis, k - l, &S[l], r);
}

void eKDTree::finalizeLeaf(eSceneData::Entry** itemPtr, eKDTree::Node& node) {
	// expand all items and calculate correct boundbox
	node.m_bbox.clear();
	for(eU32 i = 0; i < node.renderableCnt; i++) {
		eSceneData::Entry*& e = itemPtr[i];
		if(e->renderableList != eNULL) {
			expandItem(&e);
			i--;
		} else 
            node.m_bbox.mergeFast(e->aabb);
	}
	node.m_bbox.merge(node.m_bbox);
}

/*
void checkIntegrity(eSceneData::Entry** itemPtr, eU32 renderableCount) {
	if(itemPtr == eNULL)
		return;
	// check integrity
    eSceneData::Entry* lastItem = eNULL;
	eU32 d_c = 0;
	while(d_c < renderableCount) {
		eU32 d_idx = 0;
		while(d_idx < d_c) {
			eASSERT(itemPtr[d_idx] != itemPtr[d_c]);
			d_idx += itemPtr[d_idx]->renderableCount;
		}
        eU32 rcnt = itemPtr[d_c]->renderableCount;
        lastItem = itemPtr[d_c];
		d_c += rcnt;
	}
	eASSERT(d_c == renderableCount);
}
*/

void eKDTree::expandAndCalculateBBox(eSceneData::Entry** itemPtr, eU32 count, eAABB& bbox) {
	bbox.clear();
	eU32 expandThresh = (count <= 1) ? 1 : count / 2;
	eU32 idx = 0;
	while(idx < count) {
        eSceneData::Entry*& item = itemPtr[idx];
        while(item->renderableCount > expandThresh) 
			expandItem(&item);

        bbox.mergeFast(item->aabb);
		idx += item->renderableCount;
	}
	bbox.merge(bbox);
}


const eAABB& eKDTree::cull(Node& node, eSceneData::Entry** itemPtr, const eCamera& cam, eRenderJobPtrArray& jobs)
{
    eASSERT(!eIsNAN(0.0f * node.m_bbox.getSize().sqrLength())); // SANITY CHECK for valid bound boxes

#if defined(KDTREE_CULL_CELLS)
    eU32 numHits;
	if(!cam.intersectsFrustumCountHits(node.m_bbox, numHits))
        return node.m_bbox;
#else
    eU32 numIntersects = 0;
#endif

	// check if node needs to be expanded
	if((node.renderableCnt <= KD_MAX_ITEMS_PER_NODE) || (numHits >= 3)/**/) {
        if(!node.m_isFinal) {
    	    ePROFILER_ZONE("Finalize Leaf");
			finalizeLeaf(itemPtr, node);
            node.m_isFinal = eTRUE;
        }
	    ePROFILER_ZONE("Get Render Jobs");
		for(eU32 i = 0; i < node.renderableCnt; i++) {
			eSceneData::Entry*& e = itemPtr[i];

            eASSERT(!eIsNAN(0.0f * e->aabb.getSize().sqrLength())); // SANITY CHECK for valid bound boxes
#if defined(KDTREE_CULL_LEAFS)
            if(cam.intersectsFrustumCountHits(e->aabb, numHits)) 
#endif
            {
                eMatrix4x4 eMtx = (e->matrix);
				eMatrix4x4 mtx((eMtx * cam.getViewMatrix()).inverse());
				mtx.transpose();
        	    ePROFILER_ZONE("Get Render Jobs internal");
                e->renderableObject->getRenderJobs(eMtx, mtx, jobs, this->m_passID);
			}
		}
	} else {
        if(node.children[0] == eNULL) {
			
            // calculate split axis
            const eVector3& boxSize = node.m_bbox.getSize();
			eU32 axis = (boxSize[1] > boxSize[0]) ? 1 : 0;
			if(boxSize[2] > boxSize[axis])
				axis = 2;

            // sort and split
			eU32 median = selectMedianAndSort(axis, node.renderableCnt / 2, itemPtr, node.renderableCnt);

            for(eU32 c = 0; c < 2; c++) {
				node.children[c] = &this->m_nodes.push();
                node.children[c]->children[0] = eNULL;

                eSceneData::Entry** childItemPtr = (c == 0) ? itemPtr : (itemPtr + median);
                eU32 childRenderCnt              = (c == 0) ? median : (node.renderableCnt - median);

                node.children[c]->renderableCnt = childRenderCnt;
                node.children[c]->m_isFinal = eFALSE;

                // calculate child bound box
                expandAndCalculateBBox(childItemPtr, childRenderCnt, node.children[c]->m_bbox);
                eASSERT((node.children[c]->m_bbox.getMaximum() - node.children[c]->m_bbox.getMinimum()).length() > 0.0f); // empty bound boxes should not occur

                eASSERT(childRenderCnt > 0);
                eASSERT(!eIsNAN(0.0f * node.children[c]->m_bbox.getSize().sqrLength())); // SANITY CHECK for valid bound boxes
            }
	    }

		node.m_bbox.clear();
		for(eU32 c = 0; c < 2; c++) {
            if(node.children[c]->renderableCnt > 0)
                node.m_bbox.merge(cull(*node.children[c], itemPtr, cam, jobs));
            itemPtr += node.children[c]->renderableCnt;
        }
        eASSERT(!eIsNAN(0.0f * node.m_bbox.getSize().sqrLength())); // SANITY CHECK for valid bound boxes
	}

	return node.m_bbox;
}

void eKDTree::reconstruct(eSceneData& sceneList) {
    this->m_nodes.clear();

	eU32 renderableCnt = sceneList.getRenderableTotal();
    this->m_root.renderableCnt = renderableCnt;

    if(renderableCnt == 0)
        return;

    this->m_nodes.reserve(renderableCnt * 2);
	this->m_itemsRaw.resize(renderableCnt);
	this->m_items.resize(renderableCnt);

	eSceneData::Entry& item = this->m_itemsRaw[renderableCnt - 1];
    item.matrix = eMatrix4x4();
 	item.renderableCount = renderableCnt;
    item.renderableList = &sceneList;
    item.aabb = sceneList.getBoundingBox();
    eASSERT((sceneList.getBoundingBox().getMaximum() - sceneList.getBoundingBox().getMinimum()).length() > 0.0f); // empty bound boxes should not occur
    item.renderableObject = eNULL;

    this->m_items[0] = &item;
	this->m_items[item.renderableCount - 1] = &item;

    this->m_root.children[0] = eNULL;
    this->m_root.m_isFinal = eFALSE;

    expandAndCalculateBBox(m_items.m_data, renderableCnt, this->m_root.m_bbox);
    eASSERT((m_root.m_bbox.getMaximum() - m_root.m_bbox.getMinimum()).length() > 0.0f); // empty bound boxes should not occur
}

void eKDTree::cull(eU32 passID, const eCamera& cam, eRenderJobPtrArray& jobs) {
	ePROFILER_ZONE("kdTree - Cull");

    this->m_passID = passID;
    if(m_root.renderableCnt == 0)
        return;

    this->cull(m_root, this->m_items.m_data, cam, jobs);

	eRenderJob::sortJobs(jobs);

}
