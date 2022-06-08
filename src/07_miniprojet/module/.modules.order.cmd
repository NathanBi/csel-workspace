cmd_/workspace/src/07_miniprojet/module/modules.order := {   echo /workspace/src/07_miniprojet/module/mymodule.ko; :; } | awk '!x[$$0]++' - > /workspace/src/07_miniprojet/module/modules.order
