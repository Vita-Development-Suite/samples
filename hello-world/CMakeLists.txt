#
#   Vita Development Suite Samples
#

# Build self

add_executable(hello-world
  basic.c
)

# Build VPK

VDSuiteCreateSfo(hello-world-param.sfo param.yml)

VDSuitePackage(hello-world.vpk
  FILES
    hello-world eboot.bin
    hello-world-param.sfo sce_sys/param.sfo
)
