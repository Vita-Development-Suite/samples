#
#   Vita Development Suite Samples
#

# Build library

add_library(library-declspec SHARED
  library.cpp
)

# Build app

add_executable(prx-declspec
  app.cpp
)

target_link_libraries(prx-declspec
  library-declspec
  SceDbg_stub
)

# Build VPK

VDSuiteCreateSfo(prx-declspec-param.sfo param.yml)

VDSuitePackage(prx-declspec.vpk
  FILES
    $<TARGET_FILE:prx-declspec> eboot.bin
    $<TARGET_FILE:library-declspec> prx.suprx
    prx-declspec-param.sfo sce_sys/param.sfo
)
