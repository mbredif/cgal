add_executable(test_cgal_traits_2 test_cgal_traits_2.cpp test_traits.hpp)
add_executable(test_cgal_traits_3 test_cgal_traits_3.cpp test_traits.hpp)
add_executable(test_cgal_traits_d2 test_cgal_traits_d2.cpp test_traits.hpp)
add_executable(test_cgal_traits_d3 test_cgal_traits_d3.cpp test_traits.hpp)
add_executable(test_cgal_traits_d4 test_cgal_traits_d4.cpp test_traits.hpp)
add_executable(test_DDT test_DDT.cpp)



target_link_libraries(test_cgal_traits_2 libddt ${ddt_EXE_LIB_DEP})
target_link_libraries(test_cgal_traits_3 libddt ${ddt_EXE_LIB_DEP})
target_link_libraries(test_cgal_traits_d2 libddt ${ddt_EXE_LIB_DEP})
target_link_libraries(test_cgal_traits_d3 libddt ${ddt_EXE_LIB_DEP})
target_link_libraries(test_cgal_traits_d4 libddt ${ddt_EXE_LIB_DEP})
target_link_libraries(test_DDT libddt ${ddt_EXE_LIB_DEP})


add_test(cgal_traits_2 test_cgal_traits_2)
add_test(cgal_traits_3 test_cgal_traits_3)
add_test(cgal_traits_d2 test_cgal_traits_d2)
add_test(cgal_traits_d3 test_cgal_traits_d3)
add_test(cgal_traits_d4 test_cgal_traits_d4)
add_test(DDT test_DDT)
