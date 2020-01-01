/* Script for -n: mix text and data on same page */
/* Copyright (C) 2014-2018 Free Software Foundation, Inc.
   Copying and distribution of this script, with or without modification,
   are permitted in any medium without royalty provided the copyright
   notice and this notice are preserved.  */
OUTPUT_FORMAT("elf32-avr","elf32-avr","elf32-avr")
OUTPUT_ARCH(avr:51)

__BOOTLOADER_LENGTH__ = 1K;
__TEXT_REGION_LENGTH__ = 128K - __BOOTLOADER_LENGTH__;
__SYSTEM_DATA_LENGTH__ = 32;
__DATA_REGION_LENGTH__ = 16K-__SYSTEM_DATA_LENGTH__;
__EEPROM_REGION_LENGTH__ = 4K;
__FUSE_REGION_LENGTH__ = DEFINED(__FUSE_REGION_LENGTH__) ? __FUSE_REGION_LENGTH__ : 1K;
__LOCK_REGION_LENGTH__ = DEFINED(__LOCK_REGION_LENGTH__) ? __LOCK_REGION_LENGTH__ : 1K;
__SIGNATURE_REGION_LENGTH__ = DEFINED(__SIGNATURE_REGION_LENGTH__) ? __SIGNATURE_REGION_LENGTH__ : 1K;
__USER_SIGNATURE_REGION_LENGTH__ = DEFINED(__USER_SIGNATURE_REGION_LENGTH__) ? __USER_SIGNATURE_REGION_LENGTH__ : 1K;
MEMORY
{
  text           (rx)   : ORIGIN = 0, LENGTH = __TEXT_REGION_LENGTH__
  data           (rw!x) : ORIGIN = 0x800100, LENGTH = __DATA_REGION_LENGTH__
  sysdata        (rw!x) : ORIGIN = 0x800100+__DATA_REGION_LENGTH__, LENGTH = __SYSTEM_DATA_LENGTH__
  eeprom         (rw!x) : ORIGIN = 0x810000, LENGTH = __EEPROM_REGION_LENGTH__
  fuse      (rw!x) : ORIGIN = 0x820000, LENGTH = __FUSE_REGION_LENGTH__
  lock      (rw!x) : ORIGIN = 0x830000, LENGTH = __LOCK_REGION_LENGTH__
  signature (rw!x) : ORIGIN = 0x840000, LENGTH = __SIGNATURE_REGION_LENGTH__
  user_signatures (rw!x) : ORIGIN = 0x850000, LENGTH = __USER_SIGNATURE_REGION_LENGTH__
}
SECTIONS
{
  /* Read-only sections, merged into text segment: */
  .hash          : { *(.hash)		}
  .dynsym        : { *(.dynsym)		}
  .dynstr        : { *(.dynstr)		}
  .gnu.version   : { *(.gnu.version)	}
  .gnu.version_d   : { *(.gnu.version_d)	}
  .gnu.version_r   : { *(.gnu.version_r)	}
  .rel.init      : { *(.rel.init)		}
  .rela.init     : { *(.rela.init)	}
  .rel.text      :
    {
      *(.rel.text)
      *(.rel.text.*)
      *(.rel.gnu.linkonce.t*)
    }
  .rela.text     :
    {
      *(.rela.text)
      *(.rela.text.*)
      *(.rela.gnu.linkonce.t*)
    }
  .rel.fini      : { *(.rel.fini)		}
  .rela.fini     : { *(.rela.fini)	}
  .rel.rodata    :
    {
      *(.rel.rodata)
      *(.rel.rodata.*)
      *(.rel.gnu.linkonce.r*)
    }
  .rela.rodata   :
    {
      *(.rela.rodata)
      *(.rela.rodata.*)
      *(.rela.gnu.linkonce.r*)
    }
  .rel.data      :
    {
      *(.rel.data)
      *(.rel.data.*)
      *(.rel.gnu.linkonce.d*)
    }
  .rela.data     :
    {
      *(.rela.data)
      *(.rela.data.*)
      *(.rela.gnu.linkonce.d*)
    }
  .rel.ctors     : { *(.rel.ctors)	}
  .rela.ctors    : { *(.rela.ctors)	}
  .rel.dtors     : { *(.rel.dtors)	}
  .rela.dtors    : { *(.rela.dtors)	}
  .rel.got       : { *(.rel.got)		}
  .rela.got      : { *(.rela.got)		}
  .rel.bss       : { *(.rel.bss)		}
  .rela.bss      : { *(.rela.bss)		}
  .rel.plt       : { *(.rel.plt)		}
  .rela.plt      : { *(.rela.plt)		}

  .text   :
  {
    *(.vectors)
    KEEP(*(.vectors))
    *(.bottomtext)
    *(.bottomtext*)
    . = ALIGN(2);
    /* For data that needs to reside in the lower 64k of progmem.  */
     *(.progmem.gcc*)
    /* PR 13812: Placing the trampolines here gives a better chance
       that they will be in range of the code that uses them.  */
    . = ALIGN(2);
     __trampolines_start = . ;
    /* The jump trampolines for the 16-bit limited relocs will reside here.  */
    *(.trampolines)
     *(.trampolines*)
     __trampolines_end = . ;
    /* avr-libc expects these data to reside in lower 64K. */
     *libprintf_flt.a:*(.progmem.data)
     *libc.a:*(.progmem.data)
     *(.progmem.*)

    KEEP(*(.app_index.hidden))
    KEEP(*(.app_index.hidden*))
    __app_index_start = . ;
    KEEP(*(.app_index))
    KEEP(*(.app_index*))
    __app_index_end = . ;

    . = ALIGN(2);
    /* For code that needs to reside in the lower 128k progmem.  */
    *(.lowtext)
     *(.lowtext*)
     __ctors_start = . ;
     *(.ctors)
     __ctors_end = . ;
     __dtors_start = . ;
     *(.dtors)
     __dtors_end = . ;
    KEEP(SORT(*)(.ctors))
    KEEP(SORT(*)(.dtors))
    /* From this point on, we don't bother about wether the insns are
       below or above the 16 bits boundary.  */
    *(.init0)  /* Start here after reset.  */
    KEEP (*(.init0))
    *(.init1)
    KEEP (*(.init1))
    *(.init2)  /* Clear __zero_reg__, set up stack pointer.  */
    KEEP (*(.init2))
    *(.init3)
    KEEP (*(.init3))
    *(.init4)  /* Initialize data and BSS.  */
    KEEP (*(.init4))
    *(.init5)
    KEEP (*(.init5))
    *(.init6)  /* C++ constructors.  */
    KEEP (*(.init6))
    *(.init7)
    KEEP (*(.init7))
    *(.init8)
    KEEP (*(.init8))
    *(.init9)  /* Call main().  */
    KEEP (*(.init9))
    *(.text)
    . = ALIGN(2);
     *(.text.*)
    . = ALIGN(2);
    *(.fini9)  /* _exit() starts here.  */
    KEEP (*(.fini9))
    *(.fini8)
    KEEP (*(.fini8))
    *(.fini7)
    KEEP (*(.fini7))
    *(.fini6)  /* C++ destructors.  */
    KEEP (*(.fini6))
    *(.fini5)
    KEEP (*(.fini5))
    *(.fini4)
    KEEP (*(.fini4))
    *(.fini3)
    KEEP (*(.fini3))
    *(.fini2)
    KEEP (*(.fini2))
    *(.fini1)
    KEEP (*(.fini1))
    *(.fini0)  /* Infinite loop after program termination.  */
    KEEP (*(.fini0))
    /* For code that needs not to reside in the lower progmem.  */
    *(.hightext)
     *(.hightext*)
     *(.progmemx.*)
    *(.linehandlers)
    *(.linehandlers*)
    . = ALIGN(2);
    /* For tablejump instruction arrays.  We don't relax
       JMP / CALL instructions within these sections.  */
    *(.jumptables)
     *(.jumptables*)
     _etext = . ;
  }  > text
  .data :
  {
    PROVIDE (__data_start = .) ;
    build/system/*(.data)
    build/system/*(.data*)
    build/system/*(.gnu.linkonce.d*)
    build/system/*(.rodata)  /* We need to include .rodata here if gcc is used */
    build/system/*(.rodata*) /* with -fdata-sections.  */
    build/system/*(.gnu.linkonce.r*)
    . = ALIGN(2);
    _edata = . ;
    PROVIDE (__data_end = .) ;
  } > data AT> text
  __data_load_start = LOADADDR(.data);
  __data_load_end = __data_load_start + SIZEOF(.data);

  /* Global data not cleared after reset.  */
  .noinit :
  {
    build/system/*(.noinit*)
  } > data

  ASSERT((__kmh_jumptable_end & (~0x1FF)) == (__kmh_jumptable_start & (~0x1FF)), "keymatrixhandler jumptable cannot cross a 256-word boundary")

  /* Only one app can run at a time, so their bss sections all start at */
  /* the same offset. */
  __app_data_start = .;
  INCLUDE build/app_overlays.ld

  .sysbss :
  {
    PROVIDE (__bss_start = .) ;
    build/system/*(COMMON)
    build/system/*(.bss)
    build/system/*(.bss*)
    PROVIDE (__bss_end = .) ;
  }  > sysdata

  /* Stack goes just below system variables. */
  __stack = ADDR(.sysbss)-1;

  .eeprom  :
  {
    /* See .data above...  */
    KEEP(*(.eeprom*))
     __eeprom_end = . ;
  }  > eeprom
  .fuse  :
  {
    KEEP(*(.fuse))
    KEEP(*(.lfuse))
    KEEP(*(.hfuse))
    KEEP(*(.efuse))
  }  > fuse
  .lock  :
  {
    KEEP(*(.lock*))
  }  > lock
  .signature  :
  {
    KEEP(*(.signature*))
  }  > signature
  /* Stabs debugging sections.  */
  .stab 0 : { *(.stab) }
  .stabstr 0 : { *(.stabstr) }
  .stab.excl 0 : { *(.stab.excl) }
  .stab.exclstr 0 : { *(.stab.exclstr) }
  .stab.index 0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }
  .comment 0 : { *(.comment) }
  .note.gnu.build-id : { *(.note.gnu.build-id) }
  /* DWARF debug sections.
     Symbols in the DWARF debugging sections are relative to the beginning
     of the section so we begin them at 0.  */
  /* DWARF 1 */
  .debug          0 : { *(.debug) }
  .line           0 : { *(.line) }
  /* GNU DWARF 1 extensions */
  .debug_srcinfo  0 : { *(.debug_srcinfo) }
  .debug_sfnames  0 : { *(.debug_sfnames) }
  /* DWARF 1.1 and DWARF 2 */
  .debug_aranges  0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }
  /* DWARF 2 */
  .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
  .debug_abbrev   0 : { *(.debug_abbrev) }
  .debug_line     0 : { *(.debug_line .debug_line.* .debug_line_end ) }
  .debug_frame    0 : { *(.debug_frame) }
  .debug_str      0 : { *(.debug_str) }
  .debug_loc      0 : { *(.debug_loc) }
  .debug_macinfo  0 : { *(.debug_macinfo) }
  /* SGI/MIPS DWARF 2 extensions */
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames  0 : { *(.debug_varnames) }
  /* DWARF 3 */
  .debug_pubtypes 0 : { *(.debug_pubtypes) }
  .debug_ranges   0 : { *(.debug_ranges) }
  /* DWARF Extension.  */
  .debug_macro    0 : { *(.debug_macro) }
  .debug_addr     0 : { *(.debug_addr) }
}
