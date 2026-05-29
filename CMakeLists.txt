cmake_minimum_required(VERSION 3.10)
project(rtt_typelib)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FindPkgConfig)

set(NO_ROS_PACKAGE TRUE)

# Source the env.sh script of the Orocos Toolchain in order to find RTT:
find_package(OROCOS-RTT)
include( ${OROCOS-RTT_USE_FILE} )

if (NOT OROCOS-RTT_FOUND)
  message(FATAL_ERROR "Could not Find Orocos-RTT. Please use 'source env.sh' in your Orocos Toolchain directory and try to rebuild this package.")
endif()

if (OROCOS-RTT_MQUEUE_FOUND)
    message(STATUS "MQ transport found, MQ typelib marshaller enabled")
    set(mqsupport_sources MQTypelibMarshaller.cpp)
else()
    message(STATUS "MQ transport not found, MQ typelib marshaller disabled")
endif()

# Typelib specific:
pkg_check_modules(Typelib REQUIRED typelib)
include_directories(${Typelib_INCLUDE_DIRS})
link_directories(${Typelib_LIBRARY_DIRS})
add_definitions(${Typelib_CFLAGS})
find_library(Typelib_LIBRARY typeLib HINTS ${Typelib_LIBRARY_DIRS})
set(Typelib_LIBRARIES ${Typelib_LIBRARY})

# This takes care of proper install locations:
# We do our linking ourselves...
set(OROCOS_NO_AUTO_LINKING TRUE)
orocos_library(rtt-typelib TypelibMarshallerBase.cpp TypelibTransportPlugin.cpp ${mqsupport_sources})

target_link_libraries(rtt-typelib
    ${Typelib_LIBRARIES})
if (OROCOS-RTT_MQUEUE_FOUND)
    target_link_libraries(rtt-typelib
        ${OROCOS-RTT_MQUEUE_LIBRARIES})
endif()

option(BUILD_TESTING "Build rtt_typelib tests" OFF)
if(BUILD_TESTING)
    enable_testing()

    add_executable(get_marshaller_for_test tests/get_marshaller_for_test.cpp)
    target_include_directories(get_marshaller_for_test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${OROCOS-RTT_INCLUDE_DIRS})
    target_link_libraries(get_marshaller_for_test rtt-typelib)
    add_test(NAME get_marshaller_for_test COMMAND get_marshaller_for_test)
    set_tests_properties(get_marshaller_for_test PROPERTIES
        ENVIRONMENT "LD_LIBRARY_PATH=$<TARGET_FILE_DIR:rtt-typelib>:$ENV{LD_LIBRARY_PATH}")
endif()

orocos_install_headers( TypelibMarshallerBase.hpp TypelibMarshaller.hpp
    TypelibMarshallerHandle.hpp
    MQTypelibMarshaller.hpp OpaqueTypelibMarshaller.hpp
    TypelibTransportPlugin.hpp
    INSTALL include/rtt/typelib)

find_package(Doxygen)
if (DOXYGEN_FOUND)
    if (DOXYGEN_DOT_EXECUTABLE)
        SET(DOXYGEN_DOT_FOUND YES)
    elSE(DOXYGEN_DOT_EXECUTABLE)
        SET(DOXYGEN_DOT_FOUND NO)
        SET(DOXYGEN_DOT_EXECUTABLE "")
    endif(DOXYGEN_DOT_EXECUTABLE)
    configure_file(Doxyfile.in Doxyfile @ONLY)
    add_custom_target(doc doxygen Doxyfile)
endif(DOXYGEN_FOUND)

# custom written .pc file, so don't use orocos_generated_package():
#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/rtt-typelib.pc.in
#    ${CMAKE_CURRENT_BINARY_DIR}/rtt-typelib.pc @ONLY)
#install(FILES ${CMAKE_CURRENT_BINARY_DIR}/rtt-typelib.pc
#    DESTINATION lib/pkgconfig)
INSTALL(FILES package.xml DESTINATION share/rtt_typelib)
orocos_generate_package(DEPENDS typelib)
file(READ ${CMAKE_CURRENT_BINARY_DIR}/rtt_typelib-${OROCOS_TARGET}.pc INSTALLED_PC_CONTENTS)
string(REPLACE "include/orocos" "include" INSTALLED_PC_CONTENTS
    "${INSTALLED_PC_CONTENTS}")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/rtt_typelib-${OROCOS_TARGET}.pc
    ${INSTALLED_PC_CONTENTS})
