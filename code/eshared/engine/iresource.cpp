#include "../eshared.hpp"
#include "iresource.hpp"

eIResource::eIResource()
{
    eResourceManager::addResource(this);
}

eIResource::~eIResource()
{
    eResourceManager::removeResource(this);
}

eIResource::Type eIVolatileResource::getType()
{
    return TYPE_VOLATILE;
}

eIResource::Type eINonVolatileResource::getType()
{
    return TYPE_NONVOLATILE;
}