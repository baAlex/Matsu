
cmake_minimum_required(VERSION 3.16)
project("Pffft Library" C)


####
set(CMAKE_CXX_STANDARD 14)
if (MSVC)
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
	set(PFFFT_CFLAGS /W4)
endif ()
####


add_library("pffft"            OBJECT "../pffft/pffft.c")
target_compile_options("pffft" PRIVATE ${PFFFT_CFLAGS})


if (CMAKE_BUILD_TYPE STREQUAL "Release")
	set_target_properties("pffft" PROPERTIES INTERPROCEDURAL_OPTIMIZATION True)
endif ()
