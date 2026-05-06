cmake_minimum_required(VERSION 3.21)
project(bakery_backend)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------
# SOURCE FILES
# -----------------------
file(GLOB_RECURSE SOURCES
    backend/*.cpp
)

add_executable(bakery-backend ${SOURCES})

# -----------------------
# INCLUDE DIRECTORIES
# -----------------------
target_include_directories(bakery-backend PRIVATE
    ${CMAKE_SOURCE_DIR}/backend
    ${CMAKE_SOURCE_DIR}/backend/bakery-backend/bakery-backend
)

# -----------------------
# PACKAGES (vcpkg)
# -----------------------
find_package(Crow CONFIG REQUIRED)
find_package(jwt-cpp CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# -----------------------
# LINK LIBRARIES
# -----------------------
target_link_libraries(bakery-backend PRIVATE
    Crow::Crow
    jwt-cpp::jwt-cpp
    OpenSSL::SSL
    OpenSSL::Crypto
    nlohmann_json::nlohmann_json
)

# -----------------------
# WINDOWS SPECIFIC
# -----------------------
if (WIN32)
    target_link_libraries(bakery-backend PRIVATE
        odbc32
        odbccp32
    )
    target_compile_definitions(bakery-backend PRIVATE
        _WIN32_WINNT=0x0A00   # Windows 10+
    )
endif()

# -----------------------
# DEBUG DEFINES
# -----------------------
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(bakery-backend PRIVATE _DEBUG)
endif()