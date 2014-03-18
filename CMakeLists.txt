project(finnapomodorod)

cmake_minimum_required(VERSION 2.8)
find_package(PkgConfig REQUIRED)

pkg_check_modules(GLIBMM REQUIRED glibmm-2.4)
pkg_check_modules(GIOMM REQUIRED giomm-2.4)

add_subdirectory(src)
