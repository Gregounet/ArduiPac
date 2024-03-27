#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

int main (int argc, char **argv)
{
  uint8_t bios_c52[64][16];
  FILE *file = NULL;

  if (argc != 2)
    {
      fprintf (stderr, "Erreur: argument manquant\n");
      fprintf (stderr, "Syntaxe: %s <biosfile>\n", argv[0]);
      exit (-1);
    }

  file = fopen (argv[1], "rb");
  if (file == NULL)
    {
      fprintf (stderr, "%s: erreur lors de l'ouverture du fichier %\ns", argv[0],
	      argv[1]);
      exit (-2);
    }

  if (fread (bios_c52, sizeof (bios_c52), 1, file) == 0)
    {
      fprintf (stderr, "%s: erreur lors de la lecture du fichier %s\n", argv[0],
	      argv[1]);
      exit (-3);
    }

  fprintf (stderr, "%s: traitement du fichier %s\n", argv[0], argv[1]);
  printf ("//\n");
  printf ("// %s: traitement du fichier %s\n", argv[0], argv[1]);
  printf ("//\n");
  printf ("\n");
  printf ("uint8t bios_c52[1024] = {\n");
  for (int i = 0; i < 64; i++)
    {
      printf("\t");
      for (int j = 0; j < 16; j++)
	{
	  printf ("\"0x%02X\"", bios_c52[i][j]);
	  if ((i != 63) || (j != 15)) printf (", ");
	}
      printf("\n");
    }
  printf ("};\n");
  printf ("// %s: fin du fichier %s\n", argv[0], argv[1]);

  fclose (file);
  exit(0);
}
