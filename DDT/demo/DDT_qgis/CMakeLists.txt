cmake_minimum_required(VERSION 3.1...3.23)
project(DDT_qgis_Demo)

file(GLOB MY_PUBLIC_HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/*.qml
  )
file(COPY ${MY_PUBLIC_HEADERS}  DESTINATION $ENV{HOME}/.qgis2/processing/scripts/)
  




