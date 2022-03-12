_bootloader = 0x60000;
_kernel     = 0x80000;
_initramfs  = 0x20000000;

SECTIONS
{
  . = _bootloader;
  
  .text.boot : { *(.text.boot) }

  _stext = .;
  .text : { *(.text) }
  _etext = .;

  _srodata = .;
  .rodata : { *(.rodata) }
  _erodata = .;
  
  _sdata = .;
  .data : { *(.data) }
  _edata = .;

  _sbss = .;
  .bss : { *(.bss*) }
  _ebss = .;
  
  . = 0x400000;
  _estack = .;
}