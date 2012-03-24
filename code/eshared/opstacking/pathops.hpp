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

#ifndef PATH_OPS_HPP
#define PATH_OPS_HPP

// Base class for mesh operators.
class eIPathOp : public eIOperator
{
public:
    struct Result : public eIOperator::Result
    {
        Result(ePath &p) : path(p)
        {
        }

        ePath & path;
    };

public:
    eIPathOp() : m_result(m_path)
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
        m_path.clear();
    }

protected:
    ePath   m_path;
    Result  m_result;
};

#endif // PATH_OPS_HPP