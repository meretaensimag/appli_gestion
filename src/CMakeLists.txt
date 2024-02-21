cmake_minimum_required(VERSION 3.5.1)

project(FXHedging CXX)

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
# Add nlohmann_json::nlohmann_json to target_link_libraries
find_package(nlohmann_json REQUIRED)

set(fxhedging_hdrs  json_reader.hpp)
add_library(fx_hedging json_reader.cpp ${fxhedging_hdrs})
target_link_libraries(fx_hedging ${PNL_LIBRARIES} nlohmann_json::nlohmann_json)

# add_executable(test_fx_hedging test_fx_hedging.cpp)
# target_link_libraries(test_fx_hedging fx_hedging)
# pnl_add_postbuild(test_fx_hedging)


#add_executable(test_json_reader json_reader.cpp test_json_reader.cpp)
#target_link_libraries(test_json_reader ${PNL_LIBRARIES} nlohmann_json::nlohmann_json)
#pnl_add_postbuild(test_json_reader)

add_executable(hedging_portfolio
        json_reader.cpp
        hedging_portfolio.cpp
        Asset.cpp
        call_currency.cpp
        call_quanto.cpp
        RiskyAsset.hpp
        Currency.cpp
        foreign_asian.cpp
        foreign_perf_basket.cpp
        GlobalModel.cpp
        JsonParser.cpp
        MonteCarlo.cpp
        quanto_exchange.cpp
        TimeGrid.cpp
        Utils.cpp
        RiskyAsset.cpp
        FixedTimesOracle.cpp
        Position.cpp
        Portfolio.cpp
)
target_link_libraries(hedging_portfolio ${PNL_LIBRARIES} nlohmann_json::nlohmann_json)
pnl_add_postbuild(hedging_portfolio)


