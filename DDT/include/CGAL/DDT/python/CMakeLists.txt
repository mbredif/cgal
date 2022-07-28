FIND_PACKAGE(PythonInterp)
FIND_PACKAGE(PythonLibs)
FIND_PACKAGE(Boost COMPONENTS python REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS} ${PYTHON_INCLUDE_DIRS})
add_library( pyddt SHARED pyddt.cpp )
target_link_libraries( pyddt libddt
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_PYTHON_LIBRARY}
  ${PYTHON_LIBRARIES}
  )
Message(STATUS
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_PYTHON_LIBRARY}
  ${PYTHON_LIBRARIES}
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




