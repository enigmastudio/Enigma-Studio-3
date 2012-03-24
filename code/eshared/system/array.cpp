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

#include "system.hpp"

void eArrayInit(ePtrArray *a, eU32 typeSize, eU32 size)
{
    eASSERT(a != eNULL);
    eASSERT(typeSize > 0);

    a->m_data = eNULL;
    a->m_size = 0;
    a->m_capacity = 0;
    a->m_typeSize = typeSize;

    if (size > 0)
    {
        eArrayResize(a, size);
    }
}

void eArrayCopy(ePtrArray *a, const ePtrArray *ta)
{
    eASSERT(a != eNULL);
    eASSERT(ta != eNULL);

    eArrayInit(a, ta->m_typeSize, 0);
    eArrayReserve(a, ta->m_capacity);
    a->m_size = ta->m_size;

    if (ta->m_size)
    {
        eMemCopy(a->m_data, ta->m_data, ta->m_size*ta->m_typeSize);
    }
}

void eArrayClear(ePtrArray *a)
{
    eASSERT(a != eNULL);
    a->m_size = 0;
}

void eArrayFree(ePtrArray *a)
{
    eASSERT(a != eNULL);

    eSAFE_DELETE_ALIGNED_ARRAY(a->m_data);
    a->m_data = eNULL; //introhack
    a->m_size = 0;
    a->m_capacity = 0;
}

void eArrayReserve(ePtrArray *a, eU32 capacity)
{
    eASSERT(a != eNULL);

    if (capacity == 0)
    {
        eArrayClear(a);
    }
    else if (a->m_capacity < capacity)
    {
		ePROFILER_ZONE("Array Capacity Resize");

//        eU8 *temp = new eU8[capacity*a->m_typeSize];
        eU8 *temp = (eU8*) eMemAllocAlignedAndZero(capacity*a->m_typeSize, 16);
        eASSERT(temp != eNULL);
        eU32 newSize = 0;

        if (a->m_data)
        {
            newSize = eMin(a->m_size, capacity);
            eMemCopy(temp, a->m_data, newSize*a->m_typeSize);
            eSAFE_DELETE_ALIGNED_ARRAY(a->m_data);
        }

        a->m_data = (ePtr *)temp;
        a->m_size = newSize;
        a->m_capacity = capacity;
    }
}

void eArrayResize(ePtrArray *a, eU32 size)
{
    eASSERT(a != eNULL);

    if (size > a->m_capacity)
    {
        eArrayReserve(a, size);
    }

    a->m_size = size;
}

void eArrayAppend(ePtrArray *a, const ePtr data)
{
    eASSERT(a != eNULL);
    eASSERT(data != eNULL);

    if (a->m_size >= a->m_capacity)
    {
        const eU32 newCapacity = (a->m_capacity > 0 ? a->m_capacity*2 : 32);
        eArrayReserve(a, newCapacity);
    }

    const eU8 *src = (eU8 *)data;
    eU8 *dst = ((eU8 *)a->m_data)+a->m_size*a->m_typeSize;

    for (eU32 i=0; i<a->m_typeSize; i++)
    {
        dst[i] = src[i];
    }

    a->m_size++;
}

void eArrayInsert(ePtrArray *a, eU32 index, const ePtr data)
{
    eASSERT(a != eNULL);
    eASSERT(index <= a->m_size);
    eASSERT(data != eNULL);

    if (a->m_size >= a->m_capacity)
    {
        const eU32 newCapacity = (a->m_capacity > 0 ? a->m_capacity*2 : 32);
        eArrayReserve(a, newCapacity);
    }

    eMemMove(((eU8 *)a->m_data)+(index+1)*a->m_typeSize,
             ((eU8 *)a->m_data)+index*a->m_typeSize,
             (a->m_size-index)*a->m_typeSize);

    eMemCopy(((eU8 *)a->m_data)+index*a->m_typeSize, data, a->m_typeSize);
    a->m_size++;
}

void eArrayRemoveAt(ePtrArray *a, eU32 index)
{
    eASSERT(a != eNULL);
    eASSERT(index < a->m_size);

    eMemMove(((eU8 *)a->m_data)+index*a->m_typeSize,
             ((eU8 *)a->m_data)+(index+1)*a->m_typeSize,
             (a->m_size-index-1)*a->m_typeSize);

    a->m_size--;
}

eInt eArrayExists(const ePtrArray *a, const ePtr data)
{
    eASSERT(a != eNULL);
    eASSERT(data != eNULL);

    for (eU32 i=0, index=0; i<a->size(); i++, index+=a->m_typeSize)
    {
        if (eMemEqual(((eU8 *)a->m_data)+index, data, a->m_typeSize))
        {
            return i;
        }
    }

    return -1;
}