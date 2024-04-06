#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main (int argc, char **argv)
{
  uint8_t rom[4096];
  FILE *bios = NULL;
  FILE *cartridge = NULL;

  memset(rom, 0, 4096);

  if (argc != 3)
    {
      fprintf (stderr, "Erreur: argument manquant\n");
      fprintf (stderr, "Syntaxe: %s <biosfile> <cartridgefile>\n", argv[0]);
      exit (-1);
    }

  bios = fopen (argv[1], "rb");
  if (bios == NULL)
    {
      fprintf (stderr, "%s: erreur lors de l'ouverture du fichier %\ns", argv[0], argv[1]);
      exit (-2);
    }

  cartridge = fopen (argv[2], "rb");
  if (cartridge == NULL)
    {
      fprintf (stderr, "%s: erreur lors de l'ouverture du fichier %\ns", argv[0], argv[2]);
      exit (-3);
    }

  if (fread (rom, 1024, 1, bios) == 0)
    {
      fprintf (stderr, "%s: erreur lors de la lecture du fichier %s\n", argv[0], argv[1]);
      exit (-4);
    }

  if (fread (&rom[1024], 2048, 1, cartridge) == 0)
    {
      fprintf (stderr, "%s: erreur lors de la lecture du fichier %s\n", argv[0], argv[2]);
      exit (-5);
    }

  fprintf (stderr, "%s: traitement des fichier %s(BIOS) et %s(CARTOUCHE)\n", argv[0], argv[1],argv[2]);


  printf ("//\n");
  printf ("// %s: traitement des fichiers %s(BIOS) et %s(CARTOUCHE)\n", argv[0], argv[1],argv[2]);
  printf ("//\n");
  printf ("\n");
  printf ("#include <stdint.h>\n");
  printf ("\n");
  printf ("uint8_t rom[4096] = {\n");
  for (int i = 0; i < 256; i++)
    {
      printf("\t");
      for (int j = 0; j < 16; j++)
	{
	  printf ("0x%02X", rom[i*16+j]);
	  if ((i != 255) || (j != 15)) printf (", ");
	}
      printf("\n");
    }
  printf ("};\n");
  printf ("// %s: fin des fichiers %s et %s\n", argv[0], argv[1], argv[2]);

  fclose (bios);
  fclose (cartridge);

  exit(0);
}
