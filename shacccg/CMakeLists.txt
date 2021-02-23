#
#   Vita Development Suite Samples
#

include_directories(SYSTEM
  ${VDSUITE_USER_INCLUDE_DIRECTORIES}
)

link_directories(
  ${VDSUITE_LIBRARY_DIRECTORIES}
)

# Build self

add_executable(shacccg
  main.c
)

target_link_libraries(shacccg
  SceCtrl_stub
  SceDisplay_stub
  SceDisplayUser_stub
  SceGxm_stub
  SceShaccCg_stub
)

# Build VPK

VDSuiteCreateSfo(shacccg-param.sfo param.sfx)

VDSuitePackage(shacccg.vpk
  FILES
    $<TARGET_FILE:shacccg> eboot.bin
    shacccg-param.sfo sce_sys/param.sfo
    ${CMAKE_SYSROOT}/module/libshacccg.suprx module/libshacccg.suprx
    shader shader
)
