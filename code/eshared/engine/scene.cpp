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

eScene::eScene()
{
}

eScene::eScene(eSceneData &sceneData)
{
    setSceneData(sceneData);
}

void eScene::update(eF32 time)
{
    eASSERT(time >= 0.0f);
}

static eU32 collectPassCounter = 0;

void eScene::collectRenderJobs(const eCamera &cam, eRenderJobPtrArray &jobs)
{
    jobs.clear();

//    eRenderJob::resetInstancing();
    collectPassCounter++;

	// NOTE: eKDTree::cull is NOT a const function
    m_kdTree.cull(collectPassCounter, cam, jobs);
}

void eScene::setSceneData(eSceneData &sceneData)
{
    m_sceneData.clear();
    m_sceneData.merge(sceneData);

    m_kdTree.reconstruct(sceneData);
}

eU32 eScene::getLightCount() const
{
    return m_sceneData.getLightCount();
}

const eLight & eScene::getLight(eU32 index) const
{
    return m_sceneData.getLight(index);
}