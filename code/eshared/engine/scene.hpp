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

#ifndef SCENE_HPP
#define SCENE_HPP

class eScene
{
public:
    eScene();
    eScene(eSceneData &sceneData);

    void            update(eF32 time);
    void            collectRenderJobs(const eCamera &cam, eRenderJobPtrArray &jobs);
    void            setSceneData(eSceneData &sceneData);

    eU32            getLightCount() const;
    const eLight &  getLight(eU32 index) const;

private:
    eKDTree         m_kdTree;
    eSceneData      m_sceneData;
};

#endif // SCENE_HPP