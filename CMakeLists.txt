#-------------------------------------------------------------------------------
# Project description
#-------------------------------------------------------------------------------
SET( VERSION_MAJOR "0" )
SET( VERSION_MINOR "1" )
SET( VERSION_PATCH "0")
cmake_minimum_required( VERSION 2.6 )
project( RedirPlugin )

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/cmake )
SET(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
SET(CMAKE_BUILD_TYPE Release)
ADD_SUBDIRECTORY( src )

