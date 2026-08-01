cmake_minimum_required(VERSION 3.16)
project(minpdf VERSION 0.1.0 LANGUAGES C)

add_library(minpdf STATIC core.c)
target_include_directories(minpdf PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(minpdf PUBLIC c_std_11)
set_target_properties(minpdf PROPERTIES POSITION_INDEPENDENT_CODE ON)

include(GNUInstallDirs)
install(TARGETS minpdf EXPORT minpdf-targets ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES include/minpdf.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT minpdf-targets NAMESPACE minpdf:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/minpdf)
configure_file(minpdf.pc.in minpdf.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/minpdf.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
