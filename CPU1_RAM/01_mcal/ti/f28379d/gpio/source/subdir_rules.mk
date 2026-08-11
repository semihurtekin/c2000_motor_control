################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
01_mcal/ti/f28379d/gpio/source/%.obj: ../01_mcal/ti/f28379d/gpio/source/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'C2000 Compiler - building file: "$<"'
	"C:/ti/ccs2100/ccs/tools/compiler/ti-cgt-c2000_25.11.1.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 --include_path="C:/ti/ccs2100/ccs/tools/compiler/ti-cgt-c2000_25.11.1.LTS/include" --include_path="C:/ti/C2000Ware_26_01_00_00/device_support/f2837xd/headers/include" --include_path="C:/ti/C2000Ware_26_01_00_00/device_support/f2837xd/common/include" --include_path="C:/Desktop/Work/TI/CCS_Workspace/c2000_motor_control/08_labs/00_gpio_output_lab" --include_path="C:/Desktop/Work/TI/CCS_Workspace/c2000_motor_control/01_mcal/ti/f28379d/gpio/source" --include_path="C:/Desktop/Work/TI/CCS_Workspace/c2000_motor_control/01_mcal/ti/f28379d/gpio/include" --define=CPU1 -g --diag_suppress=10063 --diag_warning=225 --display_error_number --gen_func_subsections=on --abi=coffabi --preproc_with_compile --preproc_dependency="01_mcal/ti/f28379d/gpio/source/$(basename $(<F)).d_raw" --obj_directory="01_mcal/ti/f28379d/gpio/source" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


