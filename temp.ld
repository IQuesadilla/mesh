MEMORY
{
	FLASH (rx) : ORIGIN = 0x0, LENGTH = 0x77FF
	BOOT (rx) : ORIGIN = 0x7800, LENGTH = 0x0800
	RAM (xrw) : ORIGIN = 0x800100, LENGTH = 2k
}

SECTIONS { 
.text :
{
	PROVIDE(__text_start = .);
	*(.text)
	*(.text*)
	PROVIDE(__text_end = .);
} > FLASH
__etext = __text_end;

.data :
{
	PROVIDE(__data_start = .);
	*(.data)
	*(.data*)
    *(.rodata)
	*(.rodata*)
	_edata = .;
	PROVIDE(__data_end = .);
} > RAM AT> FLASH

__data_load_start = LOADADDR(.data);
__data_load_end = __data_load_start + (__data_end - __data_start);

.bss : ALIGN(4)
{
	PROVIDE(__bss_start = .);
	*(.bss)
	*(.bss*)
	*(COMMON)
	. = ALIGN(4);
	PROVIDE(__bss_end = .);
	PROVIDE(__stack_end = .);
} > RAM


.bootsec : ALIGN(4)
{
	__boot_start = .;
	*(.init4)
	KEEP(*(.bootsec.__boot))
	KEEP(*(.bootsec*))
	__boot_end = .;
} > BOOT
}