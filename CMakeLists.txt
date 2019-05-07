cmake_minimum_required(VERSION 3.0)

project(openlwm2m)

################################################################################
# Options
################################################################################

OPTION(WITH_TEST "build with test" OFF)
OPTION(WITH_DOC "build with documenation" OFF)

if(NOT LWM2M_PLATFORM)
	set(LWM2M_PLATFORM "linux")
endif()

message(STATUS)
message(STATUS "${PROJECT_NAME} Configuration:")
message(STATUS "CMAKE_BUILD_TYPE              = ${CMAKE_BUILD_TYPE}")
message(STATUS)
message(STATUS "LWM2M_PLATFORM                = ${LWM2M_PLATFORM}")
message(STATUS)
message(STATUS "WITH_DOC                      = ${WITH_DOC}")
message(STATUS "WITH_TEST                     = ${WITH_TEST}")
message(STATUS)

################################################################################
# Compiler flags
################################################################################

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=gnu++11 -Wall")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

################################################################################
# Sources
################################################################################

add_subdirectory(src)
add_subdirectory(example/${LWM2M_PLATFORM})

if(WITH_TEST)
	enable_testing()
	add_subdirectory(tests)
	if (CMAKE_BUILD_TYPE STREQUAL "Coverage")
		set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)
		include(CodeCoverage)
		setup_target_for_coverage(coverage unitTests coverage)
	endif()
endif()

################################################################################
# Doc
################################################################################

find_package(Doxygen)

if(WITH_DOC AND NOT DOXYGEN_FOUND)
	message(FATAL_ERROR "Doxygen not found. Required to generate docs.")
else()
	configure_file(
		${CMAKE_CURRENT_SOURCE_DIR}/doxygen.cfg
		${CMAKE_CURRENT_BINARY_DIR}/doxygen.cfg @ONLY
	)
	add_custom_target(
		doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doxygen.cfg
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Generating API documentation with Doxygen" VERBATIM
	)
endif()
