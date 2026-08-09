
ARCH            = x86_64

HEADERS         = common.h config.h files.h match.h partition.h drivers.h bootiso.h uiface.h
SRCS            = common.c config.c files.c match.c partition.c drivers.c bootiso.c uiface.c main.c 
OBJS            = $(SRCS:.c=.o)

SHAREDOBJ       = isoloader.so
TARGET          = isoloader.efi

INC             = /usr/include/efi
INCS            = -I$(INC) -I$(INC)/$(ARCH)

LIB             = /usr/lib
LIBS            = -lefi -lgnuefi
CRT             = $(LIB)/crt0-efi-$(ARCH).o
LDS             = $(LIB)/elf_$(ARCH)_efi.lds

CFLAGS          = $(INCS) -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -DEFI_FUNCTION_WRAPPER -DGNU_EFI_USE_MS_ABI
LDFLAGS         = -z defs -nostdlib -znocombreloc -T $(LDS) -shared -Bsymbolic -L $(LIB) $(CRT) $(LIBS)
OBJCPFLAGS      = -j .text -j .sdata -j .data -j .rodata -j .dynamic -j .dynsym -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-$(ARCH) --subsystem=10

INSTALL_DIR     = /boot/efi/EFI/isoloader


.PHONY: install clean

all: $(TARGET)

%.o: %.c $(HEADERS)
	gcc $(CFLAGS) -c $< -o $@

$(SHAREDOBJ): $(OBJS)
	ld $(LDFLAGS) $^ -o $@

$(TARGET): $(SHAREDOBJ)
	objcopy $(OBJCPFLAGS) $^ $@

install: $(TARGET)
	mkdir -p $(INSTALL_DIR)
	cp $(TARGET) $(INSTALL_DIR)/
	mkdir -p $(INSTALL_DIR)/drivers
	cp -r ../drivers/*.efi $(INSTALL_DIR)/drivers/

clean:
	rm -f $(TARGET) $(SHAREDOBJ) $(OBJS)

