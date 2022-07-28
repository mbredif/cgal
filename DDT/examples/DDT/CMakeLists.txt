# Created by the script cgal_create_cmake_script
# This is the CMake script for compiling a CGAL application.

cmake_minimum_required(VERSION 3.1...3.23)
project(DDT_Examples)

find_package(CGAL REQUIRED)

find_package(Boost QUIET COMPONENTS filesystem program_options)
if(NOT TARGET Boost::filesystem)
  message(STATUS  "NOTICE: Tests requires boost filesystem and will not be compiled.")
  return()
endif()

create_single_source_cgal_program( "ddt_mt_2.cpp" )
target_link_libraries(ddt_mt_2  PRIVATE Boost::filesystem Boost::program_options)

