cmd_/workspace/src/03_drivers/05_char_driver_sysfs/modules.order := {   echo /workspace/src/03_drivers/05_char_driver_sysfs/char_driver_sysfs.ko; :; } | awk '!x[$$0]++' - > /workspace/src/03_drivers/05_char_driver_sysfs/modules.order
