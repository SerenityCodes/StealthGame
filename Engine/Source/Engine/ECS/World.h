#pragma once

#include "EntityLookupTable.h"
#include "Components/Components.h"

namespace ecs {
    using namespace components;
class World {
    Arena* m_temp_arena_;
    Arena m_ecs_arena_;
    EntityLookupTable m_entity_lookup_table_;
    entity_t m_new_entity_id_;
public:
    World(Arena& temp_arena);
    ~World() = default;

    entity_t create_entity();
    void destroy_entity(entity_t entity);

    Transform3D create_transform(entity_t entity);
    //Transform3D create_transform(entity_t entity, Transform3D&& transform);
    Renderable create_renderable(entity_t entity, engine::vulkan::VulkanModel* model);

    EntityLookupTable::iterator entity_iterator_begin();
    EntityLookupTable::iterator entity_iterator_end();
};

}
