/***************************************************************************//**

  @file         cli.c

  @author       Stephen Brennan

  @date         Wednesday,  8 April 2015

  @brief        Command Line Interface for Minesweeper.

*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "minesweeper.h"

int msw_quit(msw *game) {
  printf("Aww. Play again soon!\n");
  msw_destroy(game);
  exit(EXIT_SUCCESS);
}

/**
   @brief Clear the screen (for POSIX terminals).
 */
void cls() {
  printf("\e[1;1H\e[2J");
}

void help() {
  printf("\nIn-game commands:\n");
  printf("\t- 'd ROW,COL' - dig at ROW,COL\n");
  printf("\t- 'f ROW,COL' - flag ROW,COL\n");
  printf("\t- 'u ROW,COL' - unflag (remove flag) ROW,COL\n");
  printf("\t- 'r ROW,COL' - reveal ROW,COL\n");
  printf("\t- 'q' - quit\n");
  printf("\t- 'h' - help\n");
}

/**
   @brief Run a whole game via CLI.
 */
void run_game(int r, int c, int m)
{
  msw game;
  int status = MSW_MMOVE;
  char op;

  msw_init(&game, r, c, m);
  cls();
  msw_print(&game, stdout);
  while (MSW_MOK(status)) {
    printf("%s\n", MSW_MSG[status]);
    printf(">");
    scanf(" %c", &op);
    if (op == 'q' || op == 'Q') {
      msw_quit(&game);
    } else if (op == 'h' || op == 'H') {
      help();
      status = MSW_MMOVE;
      continue;
    }

    scanf(" %d , %d", &r, &c);
    if (op == 'd' || op == 'D') {
      status = msw_dig(&game, r, c);
    } else if (op == 'r' || op == 'R') {
      status = msw_reveal(&game, r, c);
    } else if (op == 'u' || op == 'U') {
      status = msw_unflag(&game, r, c);
    } else if (op == 'f' || op == 'F') {
      status = msw_flag(&game, r, c);
    } else {
      status = MSW_CMD;
    }
    cls();
    msw_print(&game, stdout);

    if (msw_won(&game)) {
      status = MSW_MWIN;
      break;
    }
  }
  printf("%s\n", MSW_MSG[status]);
  msw_destroy(&game);
}

static void usage(char *name) {
  printf("usage: %s [rows columns [mines]]\n", name);
  printf("\tPlay minesweeper.\n");
  help();
}

/**
   @brief Run a command oriented game of minesweeper.
 */
int cli_main(int argc, char *argv[])
{
  int r, c, m;

  // Show usage screen.
  if (argc >= 2 && strcmp(argv[1], "-h") == 0) {
    usage(argv[0]);
    return EXIT_SUCCESS;
  }

  // Set the grid size, if given.
  if (argc >= 3) {
    sscanf(argv[1], "%d", &r);
    sscanf(argv[2], "%d", &c);
    if (r <= 0 || c <= 0 || r > 255 || c > 255) {
      fprintf(stderr, "error: bad grid size (%dx%d)\n", r, c);
      return EXIT_FAILURE;
    }
  } else {
    r = c = 10;
  }

  if (argc >= 4) {
    sscanf(argv[3], "%d", &m);
    if (m <= 0 || m > r*c) {
      fprintf(stderr, "error: bad number of mines (%d)\n", m);
      return EXIT_FAILURE;
    }
  } else {
    m = 20;
  }

  run_game(r,c,m);

  return 0;
}
