cmd_/workspace/src/03_drivers/03_char_driver_var_instances/Module.symvers := sed 's/\.ko$$/\.o/' /workspace/src/03_drivers/03_char_driver_var_instances/modules.order | scripts/mod/modpost    -o /workspace/src/03_drivers/03_char_driver_var_instances/Module.symvers -e -i Module.symvers   -T -
