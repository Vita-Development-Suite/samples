#
#   Vita Development Suite Samples
#

# Build library

add_library(library-emd MODULE
  library.c
  library.emd
)

target_link_options(library-emd
  PRIVATE
    --prx-no-runtime-support
)

target_link_libraries(library-emd
  c_stub
)

VDSuiteCreateStubs(stubs library.emd
  LIBRARIES
    library_emd_stub
)

# Build app

add_executable(prx-emd
  app.c
)

target_link_libraries(prx-emd
  library_emd_stub
  SceDbg_stub
)

# Build VPK

VDSuiteCreateSfo(prx-emd-param.sfo param.yml)

VDSuitePackage(prx-emd.vpk
  FILES
    $<TARGET_FILE:prx-emd> eboot.bin
    $<TARGET_FILE:library-emd> prx.suprx
    prx-emd-param.sfo sce_sys/param.sfo
)
