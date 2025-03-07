#pragma once

#include "Memory/Arena.h"
#include "../Vendor/flecs/flecs.h"

namespace engine {
	class StealthEngine {
	    Arena m_temp_arena_;
	    Arena m_permanent_arena_;
	    flecs::world m_world_;
	public:
	    StealthEngine();
	    StealthEngine(const StealthEngine&) = delete;
	    StealthEngine(StealthEngine&&) = delete;
	    StealthEngine& operator=(const StealthEngine&) = delete;
	    StealthEngine& operator=(StealthEngine&&) = delete;
	    ~StealthEngine() = default;

	    void run();
	    flecs::world& get_world();
	};

}