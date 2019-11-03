project(unitTests)

################################################################################
# Requires
################################################################################

find_package(Catch2 REQUIRED)

################################################################################
# Compiler flags
################################################################################

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++11 -Wall")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

################################################################################
# Includes
################################################################################

include_directories(
	.
	platform
	../src
)

################################################################################
# Sources
################################################################################

set(TEST_SOURCES
	main.cpp
	testbootstraphandler.cpp
	testconverter.cpp
	testobject.cpp
	testobjectinstance.cpp
	testobjectmanager.cpp
	testresource.cpp
	testserverhandler.cpp
	testtimer.cpp
)

add_subdirectory(platform)

################################################################################
# Targets
################################################################################

add_executable(unitTests ${TEST_SOURCES})

################################################################################
# Libraries
################################################################################

target_link_libraries(
	unitTests
	Catch2::Catch2
	lwm2mclient
	testPlatform
)

include(CTest)
include(ParseAndAddCatchTests)

ParseAndAddCatchTests(unitTests)