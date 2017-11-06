#!/bin/bash

# Do minimal game rebuild:
cd ~/Desktop/support_files/OSELAS.BSP-EnergyMicro-Gecko
ptxdist clean game
ptxdist compile game
ptxdist targetinstall game
ptxdist image root.romfs
ptxdist test flash-rootfs
exit 0
