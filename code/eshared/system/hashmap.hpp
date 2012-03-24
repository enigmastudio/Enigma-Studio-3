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

#ifndef HASH_MAP_HPP
#define HASH_MAP_HPP

template<class KEY, class VALUE, eU32 (* hashFunc)(const KEY &)=eHashPtr> class eHashMap
{
public:
    struct Pair
    {
        KEY     key;
        VALUE   value;
        eBool   used;
    };

private:
    typedef eArray<Pair> PairArray;

private:
    static const eU32 DEFAULT_SIZE = 128;

public:
    eHashMap(eU32 size=0)
    {
        reserve(size);
    }

    // Reserves 2x more space than requested (reduces
    // number of collisions and is needed in order
    // to make quadratic probing working).
    void reserve(eU32 size)
    {
        m_reqSize = size;
        m_pairCount = 0;

        const eU32 tableSize = _getNextPrimeNumber((!size ? DEFAULT_SIZE : size)*2);

        m_table.resize(tableSize);
        eMemSet(&m_table[0], 0, tableSize*sizeof(Pair));
    }

    void insert(const KEY &key, const VALUE &value)
    {
        if (m_reqSize == m_pairCount)
        {
            _growTable();
        }

        const eU32 index = hashFunc(key)%m_table.size();
        eU32 probe = index;
        eU32 k = 1;

        do
        {
            Pair &pair = m_table[probe];

            if (!pair.used)
            {
                pair.key = key;
                pair.value = value;
                pair.used = eTRUE;

                m_idxPairMap.append(probe);
                m_pairCount++;
                return;
            }
            else if (pair.key == key)
            {
                pair.value = value;
                return;
            }
            else
            {
                probe = (index+k*k)%m_table.size();
                k++;
            }
        }
        while (index != probe);

        eASSERT(eFALSE);
    }

    void clear()
    {
        m_idxPairMap.clear();
        m_pairCount = 0;

        for (eU32 i=0; i<m_table.size(); i++)
        {
            m_table[i].used = eFALSE;
        }
    }

    eU32 getCount() const
    {
        return m_pairCount;
    }

    const Pair & getPair(eU32 index) const
    {
        const eU32 realIndex = m_idxPairMap[index];

        eASSERT(realIndex < m_table.size());
        eASSERT(m_table[realIndex].used == eTRUE);

        return m_table[realIndex];
    }

    VALUE & getAt(eU32 index)
    {
        const eU32 realIndex = m_idxPairMap[index];

        eASSERT(realIndex < m_table.size());
        eASSERT(m_table[realIndex].used == eTRUE);

        return m_table[realIndex].value;
    }

    const VALUE & getAt(eU32 index) const
    {
        return getPair(index).value;
    }

    eBool exists(const KEY &key) const
    {
        return (_findKey(key) != -1);
    }

    VALUE & operator [] (const KEY &key)
    {
        eInt index = _findKey(key);

        if (index == -1)
        {
            insert(key, VALUE());
            index = _findKey(key);
        }

        eASSERT(index != -1);
        return m_table[index].value;
    }

private:
    eInt _findKey(const KEY &key) const
    {
        const eU32 index = hashFunc(key)%m_table.size();
        eU32 probe = index;
        eU32 k = 1;

        do
        {
            const Pair &pair = m_table[probe];

            if (!pair.used)
            {
                return -1;
            }
            else if (pair.used && pair.key == key)
            {
                return probe;
            }
            else
            {
                probe = (index+k*k)%m_table.size();
                k++;
            }
        }
        while (index != probe);

        return -1;
    }

    void _growTable()
    {
        m_reqSize = m_table.size()*2;

        const eU32 newSize = _getNextPrimeNumber(m_reqSize*2);
        PairArray oldTable(m_table);

        m_pairCount = 0;
        m_table.resize(newSize);
        m_idxPairMap.clear();

        for (eU32 i=0; i<m_table.size(); i++)
        {
            m_table[i].used = eFALSE;
        }

        for (eU32 i=0; i<oldTable.size(); i++)
        {
            const Pair &pair = oldTable[i];

            if (pair.used)
            {
                insert(pair.key, pair.value);
            }
        }
    }

    // Used for creating a hashmap which size is
    // a prime number. This is required in order
    // to make sure that quadratic probing will
    // always find a free slot in the table.
    eU32 _getNextPrimeNumber(eU32 num) const
    {
        num += (num%2 == 0 ? 1 : 0);

        for (eU32 i=num; ; i+=2)
        {
            eBool found = eTRUE;

            for (eU32 j=2; j<i; j++)
            {
                if (i%j == 0)
                {
                    found = eFALSE;
                    break;
                }
            }

            if (found)
            {
                return i;
            }
        }

        eASSERT(eFALSE);
        return 0;
    }

private:
    PairArray       m_table;
    eArray<eU32>    m_idxPairMap;
    eU32            m_pairCount;
    eU32            m_reqSize;
};

#endif