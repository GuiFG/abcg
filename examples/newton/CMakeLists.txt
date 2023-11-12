project(newton)
add_executable(${PROJECT_NAME} main.cpp window.cpp camera.cpp sphere.cpp orbit.cpp)
enable_abcg(${PROJECT_NAME})