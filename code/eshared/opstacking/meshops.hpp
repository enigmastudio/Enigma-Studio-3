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

#ifndef MESH_OPS_HPP
#define MESH_OPS_HPP

// Base class for mesh operators.
class eIMeshOp : public eIOperator
{
public:
    struct Result : public eIOperator::Result
    {
        Result(eEditMesh &m) : mesh(m)
        {
        }

        eEditMesh & mesh;
    };

public:
    eIMeshOp() : m_result(m_mesh)
    {
    }

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
        m_mesh.clear();
    }

protected:
    void _copyFirstInputMesh()
    {
        const eEditMesh &mesh = ((eIMeshOp *)getInputOperator(0))->getResult().mesh;
        m_mesh.merge(mesh);
    }

protected:
    eEditMesh   m_mesh;
    Result      m_result;
};

#endif // MESH_OPS_HPP