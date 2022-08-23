cmake_minimum_required(VERSION 3.1...3.23)
project(pyDDT)

FIND_PACKAGE(PythonInterp REQUIRED)
FIND_PACKAGE(PythonLibs REQUIRED)
FIND_PACKAGE(CGAL REQUIRED)
FIND_PACKAGE(Eigen3 REQUIRED NO_MODULE)
FIND_PACKAGE(Boost COMPONENTS python system filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS} ${PYTHON_INCLUDE_DIRS} ${CGAL_INCLUDE_DIRS})
add_library( pyddt SHARED pyddt.cpp )
target_link_libraries( pyddt
  ${Boost_LIBRARIES}
  ${PYTHON_LIBRARIES}
  Eigen3::Eigen
  CGAL::CGAL
  )

set_target_properties(pyddt PROPERTIES PREFIX "" )
#set_target_properties(pyddt PROPERTIES SUFFIX ".pyd")
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ddt.py
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/ddt.py ${CMAKE_CURRENT_BINARY_DIR}/ddt.py
  MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/ddt.py
  )

add_custom_command(
  OUTPUT $ENV{HOME}/.qgis2/processing/scripts/ddt_qgis.py
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/ddt_qgs.py $ENV{HOME}/.qgis2/processing/scripts/
  MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/ddt_qgs.py)

add_custom_target(
  python ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/ddt.py $ENV{HOME}/.qgis2/processing/scripts/ddt_qgis.py
  )
