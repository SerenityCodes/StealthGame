# Overview

SteathGame (yet to be named) is a game with a custom made engine that was made as a learning experience to learn more about how 3D rendering works from the graphics API up to the application.

## Components

There are two main projects that come together to make the game.

1. Engine (/Engine)
2. Game (/Game)

Engine contains all of the necessary logic to interact with Vulkan, and provides an interface to make it easy to make render passes, and submit commands. The engine also defines systems for the ECS to run, but does not impede on the games systems. The game defines the systems before the engine runs the main game loop.

Game contains all of the game logic, and contains all of the models needed to run the game. This game is a very simple game, with the goal in mind of making it custom.

## Why make Engine a static lib?

The goal was to seperate the "engine" from the "game". However, the farther I got into the project, the more I realized that the benefit of having a custom engine, is to be able to tune it specifically to the game. Seperating the two requires me to think about how to make the engine the best it can be, and how to generalize without taking a hit to performance.

# Build

To build this project, run the appropriate script `Scripts/Setup-<platform>` to generate either a Makefile or Visual Studio Solution. Once one of those is setup, either run `make` or build the solution to run the project. It's that simple!
