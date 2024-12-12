#pragma once
#include "EntityLookupTable.h"
#include "Containers/ArrayRef.h"
#include "Containers/DynArray.h"

class BucketArray {
    DynArray<ecs::EntityLookupTable::Bucket*> m_buckets_;
public:
    
};
