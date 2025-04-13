# Raylib Solitaire

A classic Solitaire card game implementation using the raylib graphics library.

## Features

- Classic Klondike Solitaire gameplay
- Smooth card animations
- Modern UI with raylib graphics
- Efficient texture caching system
- Cross-platform support (Windows, Linux, macOS)

## Requirements

- C++14 compatible compiler
- CMake 3.10 or higher
- raylib library
- nlohmann/json library (automatically fetched by CMake)

## Building the Game

### Windows

1. Install raylib:
   - Download raylib from [raylib's website](https://www.raylib.com/)
   - Extract it to `C:/raylib` (or update the path in CMakeLists.txt)

2. Build the project:
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

The executable will be created in `build/bin/Release/`

### Linux/macOS

1. Install raylib:
```bash
# Ubuntu/Debian
sudo apt-get install libraylib-dev

# macOS
brew install raylib
```

2. Build the project:
```bash
mkdir build
cd build
cmake ..
make
```

## Game Controls

- Left-click and drag to move cards
- Double-click to automatically move cards to foundation piles
- Right-click to flip through the stock pile

## Project Structure

```
raylib-solitaire/
├── assets/         # Game assets (card images, etc.)
├── src/            # Source code
│   ├── Card.cpp    # Card class implementation
│   ├── Card.h      # Card class header
│   ├── Solitaire.cpp # Game logic
│   └── main.cpp    # Main game loop
├── CMakeLists.txt  # Build configuration
└── README.md       # This file
```

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- [raylib](https://www.raylib.com/) - The graphics library used
- [nlohmann/json](https://github.com/nlohmann/json) - JSON library for C++ 
