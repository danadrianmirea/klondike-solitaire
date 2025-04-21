#!/bin/bash

# Activate Emscripten environment
. "c:\raylib\emsdk\emsdk_env.sh"

# Create build directory and source directory structure
mkdir -p web-build/src

# Copy source files to build directory
cp src/*.cpp src/*.h web-build/src/

# Build with debugging support
emcc src/*.cpp -o web-build/index.html \
  -IC:/raylib/raylib/src \
  libraylib.web.a \
  -DPLATFORM_WEB \
  -DEMSCRIPTEN_BUILD \
  -DDEBUG=1 \
  -gsource-map \
  --source-map-base http://localhost:8000/ \
  -s USE_GLFW=3 \
  -s ASYNCIFY \
  -s TOTAL_MEMORY=67108864 \
  -s STACK_SIZE=5242880 \
  -s EXPORTED_FUNCTIONS="['_main']" \
  -s EXPORTED_RUNTIME_METHODS="['ccall', 'cwrap']" \
  -s ALLOW_MEMORY_GROWTH=1 \
  --preload-file assets@/assets \
  --shell-file minshell.html

# Check if the emcc build was successful
if [ $? -eq 0 ]; then
  echo "Debug build succeeded."
  echo "To debug:"
  echo "1. Start a local server in the web-build directory (e.g., python -m http.server 8000)"
  echo "2. Open Chrome DevTools (F12)"
  echo "3. Go to Sources tab"
  echo "4. Look for your source files under 'localhost:8000/src/'"
  echo "5. Set breakpoints and debug as needed"
  
  # Start local server automatically (optional)
  echo "Starting local server..."
  cd web-build
  python -m http.server 8000
else
  echo "Debug build failed."
fi 