ARMGNU ?= aarch64-unknown-linux-gnu

COPS = -Wall -Wno-format -Wno-unused-parameter -Wno-write-strings -nostdlib -nostartfiles -ffreestanding -Iinclude -mgeneral-regs-only -ggdb
ASMOPS = -Iinclude -ggdb
LDFLAGS = -r -b binary

QEMUARGS = -M raspi3b -cpu cortex-a53 -serial null -chardev stdio,id=uart1 -serial chardev:uart1 -monitor none

BUILD_DIR = build
SRC_DIR = src

V = 0
ifeq ($(V),1)
cxxcompile = $(ARMGNU)-gcc $(COPS) -MMD $(1)
scompile = $(ARMGNU)-gcc $(ASMOPS) -MMD $(1)
link = $(ARMGNU)-ld $(LDFLAGS) $(1)
linkimg = $(ARMGNU)-ld $(1)
run = $(1) $(3)
else
cxxcompile = @/bin/echo " " $(2) && $(ARMGNU)-gcc $(COPS) -MMD $(1)
scompile = @/bin/echo " " $(2) && $(ARMGNU)-gcc $(ASMOPS) -MMD $(1)
link = @/bin/echo " " $(2) && $(ARMGNU)-ld $(LDFLAGS) $(1)
linkimg = @/bin/echo " " $(2) && $(ARMGNU)-ld $(1)
run = @$(if $(2),/bin/echo " " $(2) $(3) &&,) $(1) $(3)
endif

all : kernel8.img

clean :
	rm -rf $(BUILD_DIR) *.img 

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cc
	$(call run,mkdir -p $(@D))
	$(call cxxcompile, -c $< -o $@,COMPILE $<)
	$(call run,$(ARMGNU)-nm -n $@ >$@.sym)

$(BUILD_DIR)/%_s.o: $(SRC_DIR)/%.S
	$(call scompile, -c $< -o $@,COMPILE-S $<)
	$(call run,$(ARMGNU)-nm -n $@ >$@.sym)

$(BUILD_DIR)/font_psf.o: $(SRC_DIR)/font.psf
	$(call link, -o $@ $<,LINK $<)
	$(call run,$(ARMGNU)-nm -n $@ >$@.sym)

C_FILES = $(wildcard $(SRC_DIR)/*.cc)
ASM_FILES = $(wildcard $(SRC_DIR)/*.S)
OBJ_FILES = $(C_FILES:$(SRC_DIR)/%.cc=$(BUILD_DIR)/%.o)
OBJ_FILES += $(ASM_FILES:$(SRC_DIR)/%.S=$(BUILD_DIR)/%_s.o)
OBJ_FILES += $(BUILD_DIR)/font_psf.o

DEP_FILES = $(OBJ_FILES:%.o=%.d)
-include $(DEP_FILES)

ifeq ($(D),1)
QEMUARGS += -d int,cpu_reset,guest_errors -no-reboot
endif

kernel8.img: $(SRC_DIR)/linker.ld $(OBJ_FILES)
	$(call linkimg, -T $< -o $(BUILD_DIR)/kernel8.elf $(OBJ_FILES),CREATE $@)
	$(call run,$(ARMGNU)-objcopy $(BUILD_DIR)/kernel8.elf -O binary $@)

run : kernel8.img
		$(call run,qemu-system-aarch64 $(QEMUARGS) -kernel $<)

run-nographic: kernel8.img
		$(call run,qemu-system-aarch64 -nographic $(QEMUARGS) -kernel $<)