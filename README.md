# Overview
Zap is a data-oriented, game-focused programming language designed to deliver near “bare-metal” performance while providing modern conveniences:
* Built-in ECS (Entity-Component-System) for game logic.
* Arena-based memory for predictable allocations, plus an optional reference-counted heap for dynamic objects.
* Job system for concurrency, letting you easily spawn parallel tasks.
* Compile-time execution using aot {} blocks for precomputing values or asset sizes.
* Minimal runtime overhead—the compiler uses LLVM for native AOT compilation.
* No mandatory memory safety (unlike Rust), but with optional leak detection in debug builds.

Philosophy:
* Keep the language simple and explicit.
* Embrace data-oriented design: direct control over how data is laid out and processed.
* Provide high-performance by default, with optional dynamic features.
* Offer a “batteries-included” engine framework—Zap is not just a language, but a full game toolset.

## Project Structure

`Engine/` holds the engine library used to initialize the Vulkan instance for ease of use in the compiler/interepter. `Compiler/` holds all of the compiler code that compiles down to bytecode and/or LLVM IR.

# Build Instructions
1. Clone the repository `git clone https://github.com/SerenityCodes/ZapLang`
2. Open the `Scripts/` directory and run the appropriate `Setup` script to generate projects files. You can edit the setup scripts to change the type of project that is generated - out of the box they are set to Visual Studio 2022 for Windows and gmake2 for Linux.
