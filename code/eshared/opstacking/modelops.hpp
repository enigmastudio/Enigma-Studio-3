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

#ifndef MODEL_OPS_HPP
#define MODEL_OPS_HPP

// Base class for model operators.
class eIModelOp : public eIOperator
{
public:
    struct Result : public eIOperator::Result
    {
        Result(eSceneData &sd) : sceneData(sd)
        {
        }

        eSceneData & sceneData;
    };

public:
    eIModelOp() : m_result(m_sceneData)
    {
    }

#ifdef eEDITOR
    virtual ~eIModelOp()
    {
    }
#endif

    virtual const Result & getResult() const
    {
        return m_result;
    }

    virtual Result & getResult()
    {
        return m_result;
    }

private:
    virtual void _preExecute(eGraphicsApiDx9 *gfx)
    {
        m_sceneData.clear();
    }

protected:
    eSceneData  m_sceneData;
    Result      m_result;
};

#endif // MODEL_OPS_HPP