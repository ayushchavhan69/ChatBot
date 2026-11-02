cmake_minimum_required(VERSION 3.14)
project(AIBuddyBackend VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

set(SOURCES
    src/StorageManager.cpp
    src/AIModel.cpp
    src/ConversationManager.cpp
    src/ChatBot.cpp
    src/HttpServer.cpp
    src/main.cpp
)

add_executable(server ${SOURCES})

if (WIN32)
    target_link_libraries(server ws2_32 crypt32 winhttp)
endif()

