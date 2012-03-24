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

#ifndef SEQUENCER_OPS_HPP
#define SEQUENCER_OPS_HPP

// Base class for sequencer operators.
class eISequencerOp : public eIOperator
{
public:
    struct Result : public eIOperator::Result
    {
        Result(eSequencer &seq) : sequencer(seq)
        {
        }

        eSequencer & sequencer;
    };

public:
    eISequencerOp() : m_result(m_sequencer)
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
        m_sequencer.clear();
    }

protected:
    eSequencer  m_sequencer;
    Result      m_result;
};

#endif // SEQUENCER_OPS_HPP