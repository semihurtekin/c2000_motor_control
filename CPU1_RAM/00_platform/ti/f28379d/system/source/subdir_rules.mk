################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
00_platform/ti/f28379d/system/source/%.obj: ../00_platform/ti/f28379d/system/source/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'C2000 Compiler - building file: "$<"'
	"C:/ti/ccs2100/ccs/tools/compiler/ti-cgt-c2000_25.11.1.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 --include_path="C:/ti/ccs2100/ccs/tools/compiler/ti-cgt-c2000_25.11.1.LTS/include" --include_path="C:/ti/C2000Ware_26_01_00_00/device_support/f2837xd/headers/include" --include_path="C:/ti/C2000Ware_26_01_00_00/device_support/f2837xd/common/include" --define=CPU1 -g --diag_suppress=10063 --diag_warning=225 --display_error_number --abi=coffabi --preproc_with_compile --preproc_dependency="00_platform/ti/f28379d/system/source/$(basename $(<F)).d_raw" --obj_directory="00_platform/ti/f28379d/system/source" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


