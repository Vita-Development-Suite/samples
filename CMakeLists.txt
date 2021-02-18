#
#   Vita Development Suite Samples
#

cmake_minimum_required(VERSION 3.19)

set(CMAKE_TOOLCHAIN_FILE "$ENV{SCE_PSP2_SDK_DIR}/host_tools/build/cmake/psp2-snc-toolchain.cmake")

project(vdsuite-samples LANGUAGES C CXX)

include(VitaDevelopmentSuite)
enable_language(EMD)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

add_compile_options(
  -Xdiag=2 -Xquit=2
)

# VDSS00001
add_subdirectory(hello-world)
# VDSS00002
add_subdirectory(prx-declspec)
# VDSS00003
add_subdirectory(prx-emd)
# VDSS00004
add_subdirectory(right-plugin)
