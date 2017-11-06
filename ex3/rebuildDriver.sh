#!/bin/bash

# Do minimal driver rebuild:
cd ~/Desktop/support_files/OSELAS.BSP-EnergyMicro-Gecko
ptxdist clean driver-gamepad
ptxdist compile driver-gamepad
ptxdist targetinstall driver-gamepad
ptxdist targetinstall kernel
ptxdist image root.romfs
ptxdist test flash-rootfs
exit 0
