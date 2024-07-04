
cmake_minimum_required(VERSION 3.16)
project("Lodepng Library" CXX)


####
set(CMAKE_CXX_STANDARD 14)
if (MSVC)
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
	set(LODEPNG_CFLAGS /W4)
endif ()
####


add_library("lodepng"            OBJECT "../lodepng/lodepng.cpp")
target_compile_options("lodepng" PRIVATE ${LODEPNG_CFLAGS})


if (CMAKE_BUILD_TYPE STREQUAL "Release")
	set_target_properties("lodepng" PROPERTIES INTERPROCEDURAL_OPTIMIZATION True)
endif ()
