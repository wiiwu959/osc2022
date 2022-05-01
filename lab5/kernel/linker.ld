SECTIONS
{
  . = 0x80000;

  _stext = .;
  .text : 
  { *(.text.startup)
    *(.text)
    *(.vector)
  }
  _etext = .;

  . = ALIGN(0x1000);
  _sdata = .;
  .data : { *(.data) }
  _edata = .;

  . = ALIGN(0x1000);
  _srodata = .;
  .rodata : { *(.rodata) }
  _erodata = .;
  

  . = ALIGN(0x1000);
  _sbss = .;
  .bss : { *(.bss*) }
  _ebss = .;
  _end = .;
  
  . = 0x400000;
  _estack = .;
}