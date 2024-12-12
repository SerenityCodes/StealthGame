#include <Engine/ECS/World.h>

namespace ecs {

World::World(Arena& temp_arena) : m_temp_arena_(&temp_arena), m_ecs_arena_(2 << 25), m_entity_lookup_table_(&temp_arena, &m_ecs_arena_), m_new_entity_id_(0) {
    
}

entity_t World::create_entity() {
    entity_t new_entity = m_new_entity_id_++;
    m_entity_lookup_table_.insert(new_entity, 0);
    return new_entity;
}

void World::destroy_entity(entity_t entity) {
    m_entity_lookup_table_.remove(entity);
}

Transform3D World::create_transform(entity_t entity) {
    return {{0.f, 0.f, 2.5f}, {0.f, 0.f, 0.f}, {.5f, .5f, .5f}};
}

Renderable World::create_renderable(entity_t entity, engine::vulkan::VulkanModel* model) {
    return {.model = model};
}

EntityLookupTable::iterator World::entity_iterator_begin() {
    return m_entity_lookup_table_.begin();
}

EntityLookupTable::iterator World::entity_iterator_end() {
    return m_entity_lookup_table_.end();
}

}