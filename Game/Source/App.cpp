#include <utils.hpp>
#include <vuk/Buffer.hpp>
#include <vuk/Pipeline.hpp>

#include "Engine.h"

int main() {
	engine::StealthEngine engine;
    flecs::world& world = engine.get_world();
    engine.run();
}
