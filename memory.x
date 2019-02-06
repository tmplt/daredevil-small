MEMORY
{
  /* TODO: Verify that these are correct */
  FLASH : ORIGIN = 0x0000000, LENGTH = 512K
  RAM : ORIGIN = 0x20000000, LENGTH = 16K
}

_stack_start = 0x1FFFFFFF;
_stext = 0x410;
