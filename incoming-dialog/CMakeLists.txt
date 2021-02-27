#
#   Vita Development Suite Samples
#

include_directories(SYSTEM
  ${VDSUITE_USER_INCLUDE_DIRECTORIES}
)

link_directories(
  ${VDSUITE_LIBRARY_DIRECTORIES}
)

# Build primary app

add_executable(incoming-dialog
  main.c
)

target_link_libraries(incoming-dialog
  SceDbg_stub
  SceIncomingDialog_stub
  SceSysmodule_stub
)

VDSuiteSignElf(incoming-dialog.self $<TARGET_FILE:incoming-dialog>
  BOOT_PARAM bootparam.yml
)

VDSuiteCreateSfo(incoming-dialog-param.sfo param.yml)

VDSuitePackage(incoming-dialog.vpk
  FILES
    incoming-dialog.self eboot.bin
    incoming-dialog-param.sfo sce_sys/param.sfo
)

# Build secondary app

add_executable(incoming-dialog-target
  main-target.c
)

target_link_libraries(incoming-dialog-target
  SceAppMgrUser_stub
  SceDbg_stub
)

VDSuiteSignElf(incoming-dialog-target.self $<TARGET_FILE:incoming-dialog-target>
  BOOT_PARAM bootparam.yml
)

VDSuiteCreateSfo(incoming-dialog-target-param.sfo param-target.yml)

VDSuitePackage(incoming-dialog-target.vpk
  FILES
    incoming-dialog-target.self eboot.bin
    incoming-dialog-target-param.sfo sce_sys/param.sfo
)
