cmd_/workspace/src/02_modules/01_kernel_module/modules.order := {   echo /workspace/src/02_modules/01_kernel_module/skeleton_module.ko; :; } | awk '!x[$$0]++' - > /workspace/src/02_modules/01_kernel_module/modules.order
