project(newton)
add_executable(${PROJECT_NAME} main.cpp window.cpp camera.cpp model.cpp skybox.cpp)
enable_abcg(${PROJECT_NAME})