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

#ifndef MATHFUNCTION_OPS_HPP
#define MATHFUNCTION_OPS_HPP

// Base class for mesh operators.
class eIMathFunctionOp : public eIOperator
{
public:
    struct Result : public eIOperator::Result
    {
        Result(eMathFunction *&mf) : mathFunc(mf)
        {
        }

        eMathFunction *&    mathFunc;
    };

public:
    eIMathFunctionOp();
    virtual ~eIMathFunctionOp();

    virtual const Result &  getResult() const;
    virtual Result &        getResult();
    virtual Category        getCategory() const;


protected:
    eMathFunction *         m_mathFunc;
    Result                  m_result;
};

#endif // MATHFUNCTION_OPS_HPP