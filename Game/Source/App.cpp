#include <random>

#include "Systems/PlayerMovementSystems.h"
#include "Engine.h"
#include "Components/Components.h"
#include "Containers/ArrayRef.h"
#include "Systems/CoreEngineSystems.h"
#include "Vulkan/Camera.h"

int main() {
	engine::StealthEngine engine;
    flecs::world& world = engine.get_world();
    // For more info on import flags, go here
    // https://github.com/assimp/assimp/blob/master/include/assimp/postprocess.h
    //engine::vulkan::VulkanModel vase_model = engine.load_model("C:/Users/LyftDriver/Projects/StealthGame/Game/Models/smooth_vase");
    //engine::vulkan::VulkanModel cube_model = engine.load_model("C:/Users/LyftDriver/Projects/StealthGame/Game/Models/flat_vase");
    //engine::vulkan::VulkanModel* models[2] = {&vase_model, &cube_model};
    //initialize_world(world, ArrayRef{models, 2}, engine.get_aspect_ratio());
    //setup_input_keyboard_system(world);
    //setup_keyboard_movement(world);
    engine.run();
}
