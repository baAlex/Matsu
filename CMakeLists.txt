
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


add_executable("606-kick"       "source/606-kick.cpp")
add_executable("606-snare"      "source/606-snare.cpp")
add_executable("606-hat-closed" "source/606-hat-closed.cpp")
add_executable("606-hat-open"   "source/606-hat-open.cpp")
add_executable("606-tom-low"    "source/606-tom-low.cpp")
add_executable("606-tom-high"   "source/606-tom-high.cpp")

target_compile_options("606-kick"       PRIVATE ${MATSU_CFLAGS})
target_compile_options("606-snare"      PRIVATE ${MATSU_CFLAGS})
target_compile_options("606-hat-closed" PRIVATE ${MATSU_CFLAGS})
target_compile_options("606-hat-open"   PRIVATE ${MATSU_CFLAGS})
target_compile_options("606-tom-low"    PRIVATE ${MATSU_CFLAGS})
target_compile_options("606-tom-high"   PRIVATE ${MATSU_CFLAGS})

add_executable("analyser" "source/analyser.cpp")
target_link_libraries("analyser" PRIVATE "lodepng")
target_link_libraries("analyser" PRIVATE "pffft")
target_link_libraries("analyser" PRIVATE "cargs")

target_link_libraries("606-hat-open"   PRIVATE "pffft")
target_link_libraries("606-hat-closed" PRIVATE "pffft")

if (CMAKE_BUILD_TYPE STREQUAL "Release")
	set_target_properties("606-kick"       PROPERTIES INTERPROCEDURAL_OPTIMIZATION True)
	set_target_properties("606-snare"      PROPERTIES INTERPROCEDURAL_OPTIMIZATION True)
	set_target_properties("606-hat-closed" PROPERTIES INTERPROCEDURAL_OPTIMIZATION True)
	set_target_properties("606-hat-open"   PROPERTIES INTERPROCEDURAL_OPTIMIZATION True)
	set_target_properties("606-tom-low"    PROPERTIES INTERPROCEDURAL_OPTIMIZATION True)
	set_target_properties("606-tom-high"   PROPERTIES INTERPROCEDURAL_OPTIMIZATION True)
	set_target_properties("analyser"       PROPERTIES INTERPROCEDURAL_OPTIMIZATION True)
endif ()
