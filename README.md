-> 2D Action Metroidvania

A 2D action-metroidvania built from scratch using **C++20, SFML 2.6.1, CMake, and tmxlite**.

The project started as a learning project for understanding how the different systems of a 2D game work together. It currently has a playable level-based structure with platforming, combat, enemies, hazards, checkpoints, saving, and level transitions.

-> Current Features

* 2D player movement and platforming
* Jumping with coyote time and jump buffering
* Player attack system
* Player health and damage system
* Damage invincibility and hit flash
* Enemy AI and movement
* Multiple enemy types
* Enemy collision and knockback
* Enemy death
* Environmental damage tiles
* Checkpoints
* Save/load system
* Multiple levels
* Level transitions and spawn points
* TMX/Tiled level loading
* Camera movement and look-ahead
* Camera shake
* Player and enemy animations
* Health bar
* Death and respawn transitions
* Shader-based visual effects
* Custom platform rendering

-> Requirements

* C++20 compatible compiler
* CMake
* SFML 2.6.1
* tmxlite

The project is currently developed and tested on Linux.

->Build and Run

Clone the repository:

git clone https://github.com/rishabhj2004/metroidvania_game/
cd platformer


Configure the project:

Run the following in the terminal:

cmake -S . -B build
cmake --build build
./build/platformer

->Controls

| Key             | Action     |
| --------------- | ---------- |
| A / Left Arrow  | Move Left  |
| D / Right Arrow | Move Right |
| Space           | Jump       |
| Attack Key      | Attack     |
| Escape          | Quit       |



->Development

The game is being developed incrementally, with the different systems being built and tested individually before being integrated into the main game.

Current development is focused on expanding the gameplay systems and polishing the existing mechanics.
