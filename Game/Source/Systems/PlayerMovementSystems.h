#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "../../Vendor/flecs/flecs.h"

struct Velocity {
    glm::vec3 direction;
    float speed;
};


struct KeyboardMovement {
    int forward;
    int right;
    int up;
    int look_up;
    int look_right;
};

void setup_input_keyboard_system(flecs::world& world);
void setup_keyboard_movement(flecs::world& world);
