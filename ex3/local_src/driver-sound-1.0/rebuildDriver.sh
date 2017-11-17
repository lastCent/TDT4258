#!/bin/bash

# Do minimal driver rebuild:
cd /home/tdt4258/Desktop/OSELAS.BSP-EnergyMicro-Gecko
ptxdist clean driver-sound
ptxdist compile driver-sound
ptxdist targetinstall driver-sound
ptxdist targetinstall kernel
ptxdist image root.romfs
#ptxdist test flash-rootfs


