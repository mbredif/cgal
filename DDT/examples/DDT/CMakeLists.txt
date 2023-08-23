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

option (USE_TBB "Use TBB " ON)
if (USE_TBB)
  find_package(TBB QUIET)
endif()
include(CGAL_TBB_support)

find_package(LASLIB)
include(CGAL_LASLIB_support)

create_single_source_cgal_program( "ddt_out_of_core_2.cpp" )
target_link_libraries(ddt_out_of_core_2 PRIVATE Boost::filesystem Boost::program_options)

if(TARGET CGAL::TBB_support)

  create_single_source_cgal_program( "ddt_tbb_2.cpp" )
  target_link_libraries(ddt_tbb_2 PUBLIC Boost::filesystem Boost::program_options CGAL::TBB_support)

  if(TARGET CGAL::LASLIB_support)

	create_single_source_cgal_program( "ddt_from_las_3.cpp")
	target_link_libraries(ddt_from_las_3 PRIVATE ${CGAL_libs} Boost::filesystem Boost::program_options CGAL::LASLIB_support CGAL::TBB_support)

  else()
	message(STATUS "NOTICE : ddt_from_las_3 requires LASlib and will not be compiled.")
  endif()

else()
  message(STATUS "ddt_tbb_2 and ddt_from_las_3 require TBB and will not be compiled.")
endif()

option (USE_MPI "Use MPI " OFF)
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

