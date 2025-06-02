# Overview

SteathGame (yet to be named) is a game with a custom made engine that was made as a learning experience to learn more about how 3D rendering works from the graphics API up to the application. The libraries I used you can find below.

# Libraries Used

This project uses several third-party libraries to handle various aspects of game development:

## Graphics and Rendering
- **Vulkan** - Low-level graphics API for high-performance 3D rendering
- **vuk** - High-level C++ wrapper around Vulkan for easier development
- **vk-bootstrap** - Simplifies Vulkan initialization and setup
- **VMA (Vulkan Memory Allocator)** - Handles efficient memory management for Vulkan resources
- **SPIRV-Cross** - Cross-compilation and reflection of SPIR-V shaders
- **GLFW** - Cross-platform library for window creation, input handling, and OpenGL/Vulkan context management

## Mathematics and Utilities
- **GLM** - Mathematics library optimized for graphics programming (vectors, matrices, etc.)
- **STB** - Collection of single-header libraries, primarily used for image loading

## User Interface
- **ImGui** - Immediate mode GUI library for editor interfaces and debugging tools
- **ImGuizmo** - 3D manipulation gizmos integrated with ImGui for object transformation in the editor

## Asset Loading and Processing
- **Assimp** - Comprehensive 3D model loading library supporting various formats

## Game Architecture
- **Flecs** - Fast and flexible Entity Component System (ECS) for game object management

## Utility Libraries
- **spdlog** - Fast C++ logging library for debugging and runtime information
- **fmt** - Modern C++ string formatting library
- **concurrentqueue** - Lock-free queue for multi-threaded programming
- **robin-hood-hashing** - Fast and memory-efficient hash map implementation
- **plf_colony** - Optimized container for frequent insertion/deletion scenarios

## Development and Testing
- **doctest** - Lightweight C++ testing framework

Each library was chosen to provide robust, well-tested functionality while maintaining good performance and ease of integration with the custom engine architecture.

# Build

To build this project, run the appropriate script `Scripts/Setup-<platform>` to generate either a Makefile or Visual Studio Solution. Once one of those is setup, either run `make` or build the solution to run the project.

### Note about Platform
The game is made for 64-bit Windows. The project is made with this in mind, and I don't plan on making this game for Linux or Mac until I figure out the Windows game.