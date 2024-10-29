#!/bin/sh

filename=$0
script_dir=`dirname $filename`
executable_expose_name="Expose"
executable_leveling_name="Leveling"
executable_machinecontrol_name="Scanner"
bin_dir=.

xterm -T DDS -e bash -c "DCPSInfoRepo -ORBDebugLevel 10 -ORBLogFile DCPSInfoRepo.log -o repo.ior" &
sleep 2
xterm -T MachineControl -e bash -c "./$executable_machinecontrol_name $*; sleep 10" &
xterm -T Expose -e bash -c "./$executable_expose_name; sleep 10" &
xterm -T Leveling -e bash -c "./$executable_leveling_name; sleep 10" &
