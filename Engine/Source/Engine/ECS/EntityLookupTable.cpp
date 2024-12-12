#include "EntityLookupTable.h"

namespace ecs {

entity_t EntityLookupTable::hash(entity_t entity) const {
    return entity % m_bucket_count_;
}

entity_t EntityLookupTable::hash_double(entity_t entity) const {
    return entity % (2 * m_bucket_count_);
}

entity_t EntityLookupTable::get_bucket_index(entity_t entity) const {
    size_t bucket_index = hash(entity);
    if (bucket_index < m_bucket_count_) {
        bucket_index = hash_double(entity);
    }
    return bucket_index;
}

EntityLookupTable::EntityLookupTable(Arena* temp_arena, Arena* ecs_arena) : EntityLookupTable(temp_arena, ecs_arena, 16) {
    
}

EntityLookupTable::EntityLookupTable(Arena* temp_arena, Arena* ecs_arena, size_t bucket_count) :
    m_temp_arena_(temp_arena),
    m_bucket_pool_(ecs_arena, 1024, sizeof(Bucket)),
    m_buckets_(*ecs_arena, bucket_count),
    m_size_(0),
    m_bucket_count_(bucket_count),
    m_split_pointer_(0) {
    for (size_t i = 0; i < m_bucket_count_; i++) {
        const auto bucket = static_cast<Bucket*>(m_bucket_pool_.allocate());
        bucket->next = nullptr;
        bucket->prev = nullptr;
        bucket->entity = invalid_entity;
        bucket->enabled_components = 0;
        m_buckets_[i] = bucket;
    }
}


size_t EntityLookupTable::get_size() const {
    return m_size_;
}

void EntityLookupTable::insert(entity_t entity, component_set enabled_components) {
    size_t bucket = get_bucket_index(entity);
    if (bucket >= m_bucket_count_) {
        Bucket* new_bucket = static_cast<Bucket*>(m_bucket_pool_.allocate());
        new_bucket->next = nullptr;
        new_bucket->prev = nullptr;
        new_bucket->entity = entity;
        new_bucket->enabled_components = enabled_components;
        m_buckets_.push_back(new_bucket);
    } else {
        Bucket* bucket_to_add_to = m_buckets_[bucket];
        while (bucket_to_add_to->next != nullptr) {
            bucket_to_add_to = bucket_to_add_to->next;
        }
        bucket_to_add_to->next = static_cast<Bucket*>(m_bucket_pool_.allocate());
        Bucket* new_bucket = bucket_to_add_to->next;
        new_bucket->entity = entity;
        new_bucket->enabled_components = enabled_components;
        new_bucket->prev = bucket_to_add_to;
    }
    m_size_++;
}

component_set& EntityLookupTable::get_enabled_components(entity_t entity) const {
    size_t bucket_index = get_bucket_index(entity);
    Bucket* bucket = m_buckets_[bucket_index];
    for (Bucket* it = bucket; it != nullptr; ++it) {
        if (it->entity == entity) {
            return it->enabled_components;
        }
    }
    // Should never reach here unless giving an expired entity
    // Undefined behavior
    return bucket->enabled_components;
}

void EntityLookupTable::remove(entity_t entity) {
    const size_t bucket_index = get_bucket_index(entity);
    Bucket* bucket = m_buckets_[bucket_index];
    for (Bucket* it = bucket; it != nullptr; ++it) {
        if (it->entity == entity) {
            Bucket* prev = it->prev;
            Bucket* next = it->next;
            if (prev != nullptr) {
                prev->next = next;
            }
            if (next != nullptr) {
                next->prev = prev;
            }
            if (prev == nullptr) {
                // This is the head. Just invalidate the entity
                it->entity = invalid_entity;
                it->enabled_components = 0;
            } else {
                m_bucket_pool_.deallocate(it);
            }
            m_size_--;
            break;
        }
    }
}

void EntityLookupTable::split() {
    if (const double load_factor = static_cast<double>(m_size_) / static_cast<double>(m_bucket_count_); load_factor > 1.5) {
        m_buckets_.push_back(static_cast<Bucket*>(m_bucket_pool_.allocate()));
        Bucket* bucket_to_split = m_buckets_[m_split_pointer_];
        for (Bucket* it = bucket_to_split; it != nullptr; ++it) {
            if (it->prev != nullptr && it->prev->entity == invalid_entity) {
                it->prev = it->prev->prev;
                m_bucket_pool_.deallocate(it->prev);
            }
            entity_t new_bucket_index = hash_double(it->entity);
            Bucket* bucket_to_add_to = m_buckets_[new_bucket_index];
            if (bucket_to_add_to != bucket_to_split) {
                while (bucket_to_add_to->next != nullptr) {
                    bucket_to_add_to = bucket_to_add_to->next;
                }
                bucket_to_add_to->next = static_cast<Bucket*>(m_bucket_pool_.allocate());
                Bucket* new_bucket = bucket_to_add_to->next;
                new_bucket->entity = it->entity;
                new_bucket->enabled_components = it->enabled_components;
                new_bucket->prev = bucket_to_add_to;
                new_bucket->next = nullptr;

                it->entity = invalid_entity;
                if (it->prev != nullptr) {
                    it->prev->next = it->next;
                }
            }
        }
        m_split_pointer_++;

        if (m_split_pointer_ >= m_bucket_count_) {
            m_split_pointer_ = 0;
            for (size_t i = 0; i < m_bucket_count_; i++) {
                m_buckets_.push_back(static_cast<Bucket*>(m_bucket_pool_.allocate()));
            }
            m_bucket_count_ *= 2;
        }
    }
}

EntityLookupTable::iterator EntityLookupTable::begin() {
    return iterator{*this, 0};
}

EntityLookupTable::iterator EntityLookupTable::end() {
    return iterator{*this, m_bucket_count_};
}

}