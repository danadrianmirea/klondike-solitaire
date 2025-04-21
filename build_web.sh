. "c:\raylib\emsdk\emsdk_env.sh"
mkdir -p web-build
emcc src/*.cpp -o web-build/index.html \
  -IC:/raylib/raylib/src \
  libraylib.web.a \
  -DPLATFORM_WEB \
  -DEMSCRIPTEN_BUILD \
  -DDEBUG=1 \
  -s ASYNCIFY \
  -s TOTAL_MEMORY=67108864 \
  -s STACK_SIZE=5242880 \
  -s USE_GLFW=3 \
  -s EXPORTED_FUNCTIONS="['_main', '_glfwInit', '_glfwTerminate', '_glfwWindowHint']" \
  -s EXPORTED_RUNTIME_METHODS="['ccall', 'cwrap']" \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s USE_WEBGL2=1 \
  -s FULL_ES3=1 \
  -s MIN_WEBGL_VERSION=2 \
  -s MAX_WEBGL_VERSION=2 \
  -s ASSERTIONS=1 \
  -s SAFE_HEAP=1 \
  -s STACK_OVERFLOW_CHECK=1 \
  -s DEMANGLE_SUPPORT=1 \
  -O1 \
  --preload-file assets@/assets \
  --shell-file minshell.html

# Check if the emcc build was successful
if [ $? -eq 0 ]; then
  echo "Build succeeded, creating web-build.zip..."
  powershell -Command "Compress-Archive -Path web-build\* -DestinationPath web-build.zip -Force"
else
  echo "Build failed, not creating zip."
fi
