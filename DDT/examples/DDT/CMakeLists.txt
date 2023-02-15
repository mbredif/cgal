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

find_package(LASLIB)
include(CGAL_LASLIB_support)
if(TARGET CGAL::LASLIB_support)
  create_single_source_cgal_program( "ddt_3d_from_las.cpp")
  target_link_libraries(read_las_example PRIVATE ${CGAL_libs} Boost::filesystem CGAL::LASLIB_support)
else()
  message(
    STATUS
      "NOTICE : the LAS reader test requires LASlib and will not be compiled.")
endif()

create_single_source_cgal_program( "ddt_mt_2.cpp" )
create_single_source_cgal_program( "ddt_out_of_core_2.cpp" )
target_link_libraries(ddt_mt_2  PRIVATE Boost::filesystem Boost::program_options)
target_link_libraries(ddt_out_of_core_2 PRIVATE Boost::filesystem Boost::program_options)

option (USE_TBB "Use TBB " ON)
if (USE_TBB)
  find_package(TBB QUIET)
endif()

include(CGAL_TBB_support)
if(TARGET CGAL::TBB_support)
  target_link_libraries(ddt_mt_2 PUBLIC CGAL::TBB_support)
else()
  message(STATUS "ddt_mt_2 will be used without TBB support")
endif()

option (USE_MPI "Use MPI " ON)
if (USE_MPI)
  find_package(MPI)
  if ( MPI_FOUND )
    include_directories(SYSTEM ${MPI_INCLUDE_PATH})
    add_definitions("-DCGAL_LINKED_WITH_MPI=1")
    target_link_libraries(ddt_mt_2 PUBLIC ${MPI_CXX_LIBRARIES})
  else()
    message(STATUS "NOTICE: MPI library not found, MPI support is thus disabled.")
  endif()
endif()

