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

#ifndef FACTORY_HPP
#define FACTORY_HPP

// Helper function to create new class object.
template <class BASE_CLASS, class CLASS> static BASE_CLASS * eFactory_newInstance()
{
    BASE_CLASS *instance = new CLASS;
    eASSERT(instance != eNULL);
    return instance;
}

// Factory class for creating class objects based on IDs.
template<class BASE_CLASS, class UNIQUE_ID> class eFactory : public eSingleton<eFactory<BASE_CLASS, UNIQUE_ID>>
{
public:
    template<class CLASS> eBool registerClass(UNIQUE_ID id)
    {
        // Class already registered?
        const eU32 idHash = eHashStr(id);
        const eBool exists = m_idHash2func.exists(idHash);
        eASSERT(exists == eFALSE);

        if (exists)
        {
            return eFALSE;
        }

        m_idHash2func.insert(idHash, &eFactory_newInstance<BASE_CLASS, CLASS>);
        return eTRUE;
    }

    eU32 getRegisteredCount() const
    {
        return m_idHash2func.getCount();
    }

    eU32 toHashID(UNIQUE_ID id) const
    {
        return eHashStr(id);
    }

    BASE_CLASS * newInstance(UNIQUE_ID id) const
    {
        return newInstanceByHashID(toHashID(id));
    }

    BASE_CLASS * newInstanceByHashID(eU32 idHash) const
    {
        eASSERT(m_idHash2func.exists(idHash) == eTRUE);

        return m_idHash2func[idHash]();
    }

    BASE_CLASS * newInstanceByIndex(eU32 index) const
    {
        eASSERT(index < m_idHash2func.getCount());
        return m_idHash2func.getPair(index).value();
    }

private:
    typedef BASE_CLASS *(* NewObjectFunc )();
    typedef eHashMap<eInt, NewObjectFunc, eHashInt> IdHashNewFuncMap;

private:
    mutable IdHashNewFuncMap m_idHash2func;
};

// Helper class for class registering in factory.
class eFactoryRegisterer
{
public:
    // Parameter ist result from factory's
    // registering function.
    eFactoryRegisterer(eBool regResult)
    {
        eASSERT(regResult != eFALSE);
    }
};

#ifdef eEDITOR
#define eFACTORY_REGISTER(factory, className, id) \
    eFactoryRegisterer regClass##className(factory::get().registerClass<className>(id));
#else
#define eFACTORY_REGISTER(factory, className, id)
#endif

#endif // FACTORY_HPP