#ifndef __TABLE_H
#define __TABLE_H

extern struct lookup_mnemonics
{
  char *mnemonic;
  unsigned char bytes;
  unsigned char type;
} lookup[];

#endif
