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

#ifndef ARRAY_HPP
#define ARRAY_HPP

template<class T> class eArray;

// Some often used default type defintions.
typedef eArray<eChar *> eCharPtrArray;
typedef eArray<eChar> eCharArray;
typedef eArray<ePtr> ePtrArray;
typedef eArray<eU8> eByteArray;
typedef eArray<eID> eIdArray;

// Non-templated functions used to avoid code
// bloat, caused by heavy template instantiation.
void eArrayInit(ePtrArray *a, eU32 typeSize, eU32 size);
void eArrayCopy(ePtrArray *a, const ePtrArray *ta);
void eArrayClear(ePtrArray *a);
void eArrayFree(ePtrArray *a);
void eArrayReserve(ePtrArray *a, eU32 capacity);
void eArrayResize(ePtrArray *a, eU32 size);
void eArrayAppend(ePtrArray *a, const ePtr data);
void eArrayAppend(ePtrArray *a, const ePtrArray &ta);
void eArrayInsert(ePtrArray *a, eU32 index, const ePtr data);
void eArrayRemoveAt(ePtrArray *a, eU32 index);
eInt eArrayExists(const ePtrArray *a, const ePtr data);

// Templated dynamic array. This class is intro-safe,
// because all array functions which are duplicated
// during template instantiation are inlined, using
// non-templated functions. Works for all kind of
// data types, not only pointers.
// One drawback is, that object's (T) constructors
// aren't called when copying or instantiating array.
template<class T> class eArray
{
public:
    eFORCEINLINE eArray(eU32 size=0)
    {
        eArrayInit((ePtrArray *)this, sizeof(T), size);
    }

    eFORCEINLINE eArray(const eArray &a)
    {
        eArrayCopy((ePtrArray *)this, (const ePtrArray *)&a);
    }

    eFORCEINLINE ~eArray()
    {
        free();
    }

    eFORCEINLINE eBool isEmpty() const
    {
        return (m_size == 0);
    }

    eFORCEINLINE void resize(eU32 size)
    {
        eArrayResize((ePtrArray *)this, size);
    }

    eFORCEINLINE void reserve(eU32 capacity)
    {
        eArrayReserve((ePtrArray *)this, capacity);
    }

    eFORCEINLINE void clear()
    {
        eArrayClear((ePtrArray *)this);
    }

    eFORCEINLINE void free()
    {
        eArrayFree((ePtrArray *)this);
    }

    eFORCEINLINE void append(const T &data)
    {
        eArrayAppend((ePtrArray *)this, (ePtr *)&data);
    }

    eFORCEINLINE void append(const eArray &a)
    {
        for (eU32 i=0; i<a.size(); i++)
        {
            append(a[i]);
        }
    }

    eFORCEINLINE void insert(eU32 index, const T &data)
    {
        eArrayInsert((ePtrArray *)this, index, (ePtr *)&data);
    }

    eFORCEINLINE void removeAt(eU32 index)
    {
        eArrayRemoveAt((ePtrArray *)this, index);
    }

    eFORCEINLINE eInt exists(const T &data) const
    {
        return eArrayExists((ePtrArray *)this, (ePtr *)&data);
    }

    eFORCEINLINE eU32 size() const
    {
        return m_size;
    }

    eFORCEINLINE eU32 capacity() const
    {
        return m_capacity;
    }

    eFORCEINLINE void reverse()
    {
        for (eU32 i=0; i<m_size/2; i++)
        {
            eSwap(m_data[i], m_data[m_size-1-i]);
        }
    }

    eFORCEINLINE T & push(const T &data)
    {
        eArrayAppend((ePtrArray *)this, (ePtr *)&data);
		return this->lastElement();
    }

    eFORCEINLINE T & push()
    {
		eArrayReserve((ePtrArray *)this, this->size() + 1);
		this->m_size++;
		return this->lastElement();
    }

    eFORCEINLINE T & pushFast()
    {
        return this->m_data[this->m_size++];
    }

    eFORCEINLINE T pop()
    {
		T val = this->lastElement();
		this->removeLastElement();
		return val;
    }

    eFORCEINLINE void removeLastElement()
    {
		this->m_size--;
    }

    eFORCEINLINE T& lastElement()
    {
		return (*this)[this->size() - 1];
    }


    // Performs insertion sort. Not really fast,
    // but already a lot faster than bubble sort
    // and quite small, easy to implement and easy
    // to get correct.
    eFORCEINLINE void sort(eBool (*predicate)(const T &a, const T &b))
    {
        for (eU32 j=1; j<m_size; j++)
        {
            const T key = m_data[j];
            eInt i = (eInt)j-1;

            while (i >= 0 && predicate(m_data[i], key))
            {
                m_data[i+1] = m_data[i];
                i--;
            }

            m_data[i+1] = key;
        }
    }

    eFORCEINLINE eArray & operator = (const eArray &a)
    {
        if (this != &a)
        {
            eArrayFree((ePtrArray *)this);
            eArrayCopy((ePtrArray *)this, (const ePtrArray *)&a);
        }

        return *this;
    }

    eFORCEINLINE T & operator [] (eInt index)
    {
        eASSERT(index >= 0 && (eU32)index < m_size);
        return m_data[index];
    }

    eFORCEINLINE const T & operator [] (eInt index) const
    {
        eASSERT(index >= 0 && (eU32)index < m_size);
        return m_data[index];
    }

public:
    T *     m_data;
    eU32    m_size;
    eU32    m_capacity;
    eU32    m_typeSize;
};

#endif // ARRAY_HPP