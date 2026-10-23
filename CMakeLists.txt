cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(AutoDecor VERSION 1.0.0)

# Проверка на наличие SDK (нужна для GitHub Actions)
if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Set GEODE_SDK env variable.")
endif()

# Подключение Geode
add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

# Создание библиотеки мода
# ВАЖНО: Если у тебя есть другие .cpp файлы (например, src/Hook.cpp), их нужно добавить сюда через пробел!
add_library(${PROJECT_NAME} SHARED 
    src/main.cpp
)

# Применение настроек Geode к твоему моду
setup_geode_mod(${PROJECT_NAME})
