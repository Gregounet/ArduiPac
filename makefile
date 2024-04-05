CC = gcc

CFLAGS = -O3 -fomit-frame-pointer
LFLAGS = -s

arduipac: arduipac.o arduipac_8048.o arduipac_8245.o arduipac_bios_rom.o arduipac_cset.o arduipac_graphics.o arduipac_input.o mnemonics.o
	$(CC) $(LFLAGS) arduipac.o arduipac_8048.o arduipac_8245.o arduipac_bios_rom.o arduipac_cset.o arduipac_graphics.o arduipac_input.o mnemonics.o

clean:
	rm -f arduipac.o arduipac_8048.o arduipac_8245.o arduipac_bios_rom.o arduipac_cset.o arduipac_graphics.o arduipac_input.o mnemonics.o

arduipac_8048.o: arduipac_8048.c arduipac_8048.h arduipac_8245.h arduipac_vmachine.h arduipac_bios_rom.h mnemonics.h
	$(CC) $(CFLAGS) -c arduipac_8048.c -o arduipac_8048.o

arduipac_8245.o: arduipac_8245.c arduipac_8245.h arduipac_8048.h arduipac_vmachine.h arduipac_cset.h arduipac_graphics.h
	$(CC) $(CFLAGS) -c arduipac_8245.c -o arduipac_8245.o

arduipac_bios_rom.o: arduipac_bios_rom.c arduipac_bios_rom.h
	$(CC) $(CFLAGS) -c arduipac_bios_rom.c -o arduipac_bios_rom.o

arduipac.o: arduipac.c arduipac_vmachine.h arduipac_8048.h arduipac_8245.h arduipac_bios_rom.h
	$(CC) $(CFLAGS) -c arduipac.c -o arduipac.o

arduipac_cset.o: arduipac_cset.c arduipac_cset.h
	$(CC) $(CFLAGS) -c arduipac_cset.c -o arduipac_cset.o

arduipac_graphics.o: arduipac_graphics.c arduipac_graphics.h arduipac_8245.h
	$(CC) $(CFLAGS) -c arduipac_graphics.c -o arduipac_graphics.o

arduipac_input.o: arduipac_input.c arduipac_input.h arduipac_8048.h
	$(CC) $(CFLAGS) -c arduipac_input.c -o arduipac_input.o

arduipac_vmachine.o: arduipac_vmachine.c arduipac_vmachine.h arduipac_8048.h arduipac_8245.h arduipac_vmachine.h arduipac_bios_rom.h mnemonics.h
	$(CC) $(CFLAGS) -c arduipac_vmachine.c -o arduipac_vmachine.o

mnemonics.o: mnemonics.c mnemonics.h
	$(CC) $(CFLAGS) -c mnemonics.c -o mnemonics.o
