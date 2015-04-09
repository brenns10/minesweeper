/***************************************************************************//**

  @file         main.c

  @author       Stephen Brennan

  @date         Wednesday,  8 April 2015

  @brief        Main entry point for Minesweeper.

*******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "minesweeper.h"

static void usage(char *name)
{
  printf("usage: %s [gui|cli]\n", name);
  printf("\tgui: Use the GTK version.\n");
  printf("\tcli: Use the command line version.\n");
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
  if (argc < 2) {
    usage(argv[0]);
  }

  if (strcmp(argv[1], "gui") == 0) {
    return gui_main(argc - 1, argv + 1);
  } else if (strcmp(argv[1], "cli") == 0) {
    return cli_main(argc - 1, argv + 1);
  }

  usage(argv[0]);
}
