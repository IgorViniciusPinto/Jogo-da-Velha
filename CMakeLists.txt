cmake_minimum_required(VERSION 3.10)

# Nome do projeto
project(TicTacToeConcurrent)

# Define o padrão C++
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Adiciona o executável
add_executable(tic_tac_toe src/tic_tac_toe.cpp)

# Adiciona suporte a threads
find_package(Threads REQUIRED)
target_link_libraries(tic_tac_toe PRIVATE Threads::Threads)