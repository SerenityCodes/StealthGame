#include <random>

#include "Systems/PlayerMovementSystems.h"
#include "Engine.h"
#include "Components/Components.h"
#include "Systems/CoreEngineSystems.h"
#include "Vulkan/Camera.h"

int main() {
	engine::StealthEngine engine;
    flecs::world& world = engine.get_world();
    engine.run();
}
