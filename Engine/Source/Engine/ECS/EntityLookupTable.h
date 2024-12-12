#pragma once

#include "ECSTypes.h"
#include "Containers/DynArray.h"
#include "Memory/Arena.h"
#include "Memory/Allocators/PoolAllocator.h"

namespace ecs {

// Forward declaration to define an iterator for navigating the buckets
class BucketIterator;

constexpr size_t BUCKET_POOL_CHUNKS = 2 << 12;
constexpr entity_t invalid_entity = std::numeric_limits<entity_t>::max();

class EntityLookupTable {
public:
    struct Bucket {
        entity_t entity;
        component_set enabled_components;
        Bucket* prev;
        Bucket* next;
    };
    class iterator {
        EntityLookupTable& m_lookup_table_;
        Bucket* m_current_bucket_;
        size_t m_index_;
    public:
        iterator(EntityLookupTable& lookup_table, size_t index) : m_lookup_table_(lookup_table), m_index_(index) {}
        
        iterator& operator++() {
            if (m_current_bucket_ && m_current_bucket_->next) {
                m_current_bucket_ = m_current_bucket_->next;
            } else {
                m_current_bucket_ = nullptr;
                while (++m_index_ < m_lookup_table_.m_bucket_count_) {
                    if (m_lookup_table_.m_buckets_[m_index_] != nullptr) {
                        m_current_bucket_ = m_lookup_table_.m_buckets_[m_index_];
                        break;
                    }
                }
            }
            return *this;
        }

        entity_t operator*() const {
            return m_lookup_table_.m_buckets_[m_index_]->entity;
        }

        Bucket* get_bucket() const {
            return m_lookup_table_.m_buckets_[m_index_];
        }

        bool operator==(const iterator& other) const {
            return m_index_ == other.m_index_ && m_current_bucket_ == other.m_current_bucket_;
        }

        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }
    };
private:
    Arena* m_temp_arena_;
    engine::allocators::PoolAllocator m_bucket_pool_;
    DynArray<Bucket*> m_buckets_;
    size_t m_size_;
    size_t m_bucket_count_;
    size_t m_split_pointer_;

    [[nodiscard]] entity_t hash(entity_t entity) const;
    [[nodiscard]] entity_t hash_double(entity_t entity) const;
    [[nodiscard]] entity_t get_bucket_index(entity_t entity) const;
public:
    EntityLookupTable(Arena* temp_arena, Arena* ecs_arena);
    EntityLookupTable(Arena* temp_arena, Arena* ecs_arena, size_t bucket_count);
    ~EntityLookupTable() = default;
    
    EntityLookupTable(const EntityLookupTable&) = delete;
    EntityLookupTable& operator=(const EntityLookupTable&) = delete;
    EntityLookupTable(EntityLookupTable&&) = delete;
    EntityLookupTable& operator=(EntityLookupTable&&) = delete;

    [[nodiscard]] size_t get_size() const;
    void insert(entity_t entity, component_set enabled_components);
    component_set& get_enabled_components(entity_t entity) const;
    void remove(entity_t entity);
    void split();
    
    iterator begin();
    iterator end();
};

class BucketIterator {
    EntityLookupTable::Bucket* m_current_bucket_;
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = EntityLookupTable::Bucket;
    using difference_type = std::ptrdiff_t;
    using pointer = EntityLookupTable::Bucket*;
    using reference = EntityLookupTable::Bucket&;
        
    BucketIterator(EntityLookupTable::Bucket* starting_bucket) : m_current_bucket_(starting_bucket) {}
        
    BucketIterator& operator++() {
        m_current_bucket_ = m_current_bucket_->next;
        return *this;
    }

    BucketIterator& operator--() {
        m_current_bucket_ = m_current_bucket_->prev;
        return *this;
    }

    reference operator*() const {
        return *m_current_bucket_;
    }

    pointer operator->() const {
        return m_current_bucket_;
    }

    pointer get() const {
        return m_current_bucket_;
    }

    bool operator==(const BucketIterator& other) const {
        return m_current_bucket_ == other.m_current_bucket_;
    }

    bool operator!=(const BucketIterator& other) const {
        return m_current_bucket_ != other.m_current_bucket_;
    }
};

}
