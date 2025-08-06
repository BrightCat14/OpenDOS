OUTPUT_FORMAT(elf32-i386)
ENTRY(start)

SECTIONS
{
    . = 0x100000;

    .text : {
        _kernel_start = .;    
        *(.text)
        _kernel_end = .;     
    }

    .rodata : {
        _rodata_start = .;
        *(.rodata)
        _rodata_end = .;
    }

    .data : {
        _data_start = .;
        *(.data)
        _data_end = .;
    }

    .bss : {
        _bss_start = .;
        *(COMMON)
        *(.bss)
        _bss_end = .;
    }
}
