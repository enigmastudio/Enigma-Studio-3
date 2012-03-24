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

#ifndef EFFECT_OPS_HPP
#define EFFECT_OPS_HPP

// Base class for mesh operators.
class eIEffectOp : public eIOperator
{
public:
    struct Result : public eIOperator::Result
    {
        Result(eIEffect *&fx) : effect(fx)
        {
        }

        eIEffect *&         effect;
    };

public:
    eIEffectOp() :
        m_effect(eNULL),
        m_result(m_effect)
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
        if (getInputCount() > 0)
        {
            if (TEST_CATEGORY(getInputOperator(0), "Effect", Effect_CID))
            {
                m_effect = ((eIEffectOp *)getInputOperator(0))->getResult().effect;
            }
        }
    }

protected:
    void _appendEffect(eIEffect *fx)
    {
        eASSERT(fx != eNULL);

        fx->addInput(m_effect);
        m_effect = fx;
    }

protected:
    Result      m_result;
    eIEffect *  m_effect;
};

#endif // EFFECT_OPS_HPP