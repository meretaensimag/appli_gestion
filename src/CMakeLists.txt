cmake_minimum_required(VERSION 3.5.1)
project(FXHedging CXX)

# Définir la politique CMP0148
if(POLICY CMP0148)
    cmake_policy(SET CMP0148 NEW)
endif()

# Release or Debug
if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "Setting build type to 'Debug' as none was specified.")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif ()

# Find Pnl
find_package(Pnl REQUIRED)
set(LIBS ${LIBS} ${PNL_LIBRARIES})
include_directories(${PNL_INCLUDE_DIRS})
if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    add_definitions(-DPNL_RANGE_CHECK_OFF)
endif()


# Find nlohmann_json to parse json files
find_package(nlohmann_json REQUIRED)

# Headers and sources
set(fxhedging_hdrs json_reader.hpp
        ChoreliaOption.cpp
        )
add_library(fx_hedging json_reader.cpp ${fxhedging_hdrs}
        ChoreliaOption.cpp
        )
target_link_libraries(fx_hedging PRIVATE ${PNL_LIBRARIES} nlohmann_json::nlohmann_json)

add_executable(hedging_portfolio
        json_reader.cpp
        hedging_portfolio.cpp
        Asset.cpp
        RiskyAsset.hpp
        Currency.cpp
        GlobalModel.cpp
        JsonParser.cpp
        MonteCarlo.cpp
        TimeGrid.cpp
        RiskyAsset.cpp
        FixedTimesOracle.cpp
        Position.cpp
        Portfolio.cpp
        ChoreliaOption.cpp
)
target_link_libraries(hedging_portfolio PRIVATE ${PNL_LIBRARIES} nlohmann_json::nlohmann_json)
pnl_add_postbuild(hedging_portfolio)
