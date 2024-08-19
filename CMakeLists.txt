
cmake_minimum_required(VERSION 3.16)
project("Matsu" CXX)


####
set(CMAKE_CXX_STANDARD 14)
if (MSVC)
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
	set(MATSU_CFLAGS /W4)
endif ()
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
	set(MATSU_CFLAGS -Wall -Wextra -pedantic -Wconversion -Wold-style-cast)
endif ()
if (NOT CMAKE_BUILD_TYPE STREQUAL "Release" AND "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined")
endif ()
####


add_subdirectory("source/thirdparty/lodepng_cmake/")
add_subdirectory("source/thirdparty/pffft_cmake/")
add_subdirectory("source/thirdparty/cargs/")


add_library("matsu" STATIC "source/matsu.cpp")
target_compile_options("matsu" PRIVATE ${MATSU_CFLAGS})
target_link_libraries("matsu" PRIVATE "pffft")


add_executable("606-kick" "source/606-kick.cpp")
target_compile_options("606-kick" PRIVATE ${MATSU_CFLAGS})
target_link_libraries("606-kick" PRIVATE "matsu")

add_executable("606-snare" "source/606-snare.cpp")
target_compile_options("606-snare" PRIVATE ${MATSU_CFLAGS})
target_link_libraries("606-snare" PRIVATE "matsu")

add_executable("606-hats" "source/606-hats.cpp")
target_compile_options("606-hats" PRIVATE ${MATSU_CFLAGS})
target_link_libraries("606-hats" PRIVATE "matsu")

add_executable("606-toms" "source/606-toms.cpp")
target_compile_options("606-toms" PRIVATE ${MATSU_CFLAGS})
target_link_libraries("606-toms" PRIVATE "matsu")


add_executable("analyser" "source/analyser.cpp")
target_compile_options("analyser" PRIVATE ${MATSU_CFLAGS})
target_link_libraries("analyser" PRIVATE "lodepng")
target_link_libraries("analyser" PRIVATE "cargs")
target_link_libraries("analyser" PRIVATE "matsu")


if (CMAKE_BUILD_TYPE STREQUAL "Release")
	set_target_properties("matsu" PROPERTIES INTERPROCEDURAL_OPTIMIZATION True)

	set_target_properties("606-kick"  PROPERTIES INTERPROCEDURAL_OPTIMIZATION True)
	set_target_properties("606-snare" PROPERTIES INTERPROCEDURAL_OPTIMIZATION True)
	set_target_properties("606-hats"  PROPERTIES INTERPROCEDURAL_OPTIMIZATION True)
	set_target_properties("606-toms"  PROPERTIES INTERPROCEDURAL_OPTIMIZATION True)

	set_target_properties("analyser" PROPERTIES INTERPROCEDURAL_OPTIMIZATION True)
endif ()
