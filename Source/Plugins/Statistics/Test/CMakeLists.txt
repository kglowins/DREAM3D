#--////////////////////////////////////////////////////////////////////////////
#--
#--  Copyright (c) 2015 BlueQuartz Software
#--  All rights reserved.
#--  BSD License: http://www.opensource.org/licenses/bsd-license.html
#--
#-- This code was partly written under US Air Force Contract FA8650-07-D-5800
#--
#--////////////////////////////////////////////////////////////////////////////

# project(StatisticsTest)
set(${PLUGIN_NAME}Test_SOURCE_DIR ${${PLUGIN_NAME}_SOURCE_DIR}/Test)
set(${PLUGIN_NAME}Test_BINARY_DIR ${${PLUGIN_NAME}_BINARY_DIR}/Test)

include(${DREAM3DProj_SOURCE_DIR}/Test/DREAM3DTestFunctions.cmake)


include_directories(${${PROJECT_NAME}_SOURCE_DIR})
include_directories(${${PROJECT_NAME}_BINARY_DIR})


set(TEST_TEMP_DIR ${${PLUGIN_NAME}Test_BINARY_DIR}/Temp)
# Make sure the directory is created during CMake time
file(MAKE_DIRECTORY ${TEST_TEMP_DIR})


configure_file(${${PLUGIN_NAME}_SOURCE_DIR}/Test/TestFileLocations.h.in
               ${${PLUGIN_NAME}_BINARY_DIR}/Test/${PLUGIN_NAME}TestFileLocations.h @ONLY IMMEDIATE)


AddDREAM3DUnitTest(TESTNAME CalculateArrayHistogramTest
                  SOURCES ${${PROJECT_NAME}_SOURCE_DIR}/Test/CalculateArrayHistogramTest.cpp
                  FOLDER "${PLUGIN_NAME}Plugin/Test"
                  LINK_LIBRARIES Qt5::Core H5Support SIMPLib)




AddDREAM3DUnitTest(TESTNAME FindDifferenceMapTest SOURCES ${${PROJECT_NAME}_SOURCE_DIR}/Test/FindDifferenceMapTest.cpp FOLDER "${PLUGIN_NAME}Plugin/Test" LINK_LIBRARIES Qt5::Core H5Support SIMPLib)

