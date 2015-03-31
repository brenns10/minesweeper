/***************************************************************************//**

  @file         minesweeper.c

  @author       Stephen Brennan

  @date         Monday, 30 March 2015

  @brief        Minesweeper game implementations.

*******************************************************************************/

#include <stdio.h>  // fprintf, fputc, scanf
#include <stdlib.h> // srand, rand, calloc, malloc, free
#include <time.h>   // time
#include <string.h> // strcmp

#include "minesweeper.h"

/*
  Define all eight neighbors for a cell.  The array rnbr is the offset from the
  row for each neighbor, and the array cnbr is the offset from the column for
  each neighbor.
 */
#define NUM_NEIGHBORS 8
char rnbr[NUM_NEIGHBORS] = {-1, -1, -1,  0,  0,  1,  1,  1};
char cnbr[NUM_NEIGHBORS] = {-1,  0,  1, -1,  1, -1,  0,  1};

/**
   @brief Return whether or not a cell is in bounds.
 */
int msw_in_bounds(smb_mine *game, int row, int column) {
  return (row >= 0 && row < game->rows &&
          column >= 0 && column < game->columns);
}

/**
   @brief Return the index of a cell.
 */
int msw_index(smb_mine *game, int row, int column) {
  return row * game->columns + column;
}

/**
   @brief Randomly generate a grid for this game.
 */
void msw_generate_grid(smb_mine *obj) {
  int i, j, n;
  char tmp;
  int rows = obj->rows;
  int columns = obj->columns;
  int mines = obj->mines;
  int ncells = rows * columns;

  // Initialize the grid.
  for (i = 0; i < ncells; i++) {
    obj->grid[i] = MSW_CLEAR;
  }

  // Add the mines.
  for (i = 0; i < mines; i++) {
    obj->grid[i] = MSW_MINE;
  }

  // Shuffle the mines. (Fisher-Yates)
  for (i = ncells-1; i > 0; i--) {
    j = rand() % (i+1);
    tmp = obj->grid[j];
    obj->grid[j] = obj->grid[i];
    obj->grid[i] = tmp;
  }

  // Count adjacent mines.
  for (i = 0; i < rows; i++) {
    for (j = 0; j < columns; j++) {

      // For each cell:
      if (obj->grid[msw_index(obj, i, j)] == MSW_MINE) {

        // If it is a mine, loop over its neighbors.
        for (n = 0; n < NUM_NEIGHBORS; n++) {
          if (msw_in_bounds(obj, i + rnbr[n], j + cnbr[n]) &&
              obj->grid[msw_index(obj, i + rnbr[n], j + cnbr[n])] != MSW_MINE) {

            // If the neighbor is in bounds and not a mine, increment its count.
            obj->grid[msw_index(obj, i + rnbr[n], j + cnbr[n])]++;
          }
        }
      }
    }
  }
}

/**
   @brief Create the initial grid for a game.
   @param obj The game.
   @param r The row of the first dig.
   @param c The column of the first dig.

   When a user first digs, their dig should always land on a cell that is clear.
   This ensures that they will have at least a little bit of information to
   start with.  So, we keep generating grids until we get one where their
   initial selection is clear.
 */
void msw_initial_grid(smb_mine *obj, int r, int c) {
  srand(time(NULL));

  obj->grid = calloc(obj->rows * obj->columns, sizeof(char));
  if (obj->grid == NULL) {
    fprintf(stderr, "error: calloc() returned null.\n");
    exit(EXIT_FAILURE);
  }

  do {
    msw_generate_grid(obj);
  } while (obj->grid[msw_index(obj, r, c)] != MSW_CLEAR);
}

/**
   @brief Initialize a minesweeper game.
 */
void smb_mine_init(smb_mine *obj, int rows, int columns, int mines)
{
  int i;
  int ncells = rows * columns;

  // Initialization logic
  obj->rows = rows;
  obj->columns = columns;
  obj->mines = mines;
  obj->grid = NULL;
  obj->visible = calloc(ncells, sizeof(char));
  if (obj->visible == NULL) {
    fprintf(stderr, "error: calloc() returned null.\n");
    exit(EXIT_FAILURE);
  }

  // Initialize the visible board and underlying grid.
  for (i = 0; i < ncells; i++) {
    obj->visible[i] = MSW_UNKNOWN;
  }
}

/**
   @brief Create a minesweeper game.
 */
smb_mine *smb_mine_create(int rows, int columns, int mines)
{
  smb_mine *obj = malloc(sizeof(smb_mine));
  if (obj == NULL) {
    fprintf(stderr, "error: malloc() returned null.\n");
    exit(EXIT_FAILURE);
  }
  smb_mine_init(obj, rows, columns, mines);
  return obj;
}

/**
   @brief Destroy a minesweeper game.
 */
void smb_mine_destroy(smb_mine *obj)
{
  // Cleanup logic
  free(obj->grid);
  free(obj->visible);
}

/**
   @brief Destroy and free a minesweeper game.
 */
void smb_mine_delete(smb_mine *obj) {
  if (obj) {
    smb_mine_destroy(obj);
    free(obj);
  } else {
    fprintf(stderr, "smb_mine_delete: called with null pointer.\n");
  }
}

/**
   @brief Print the current board to the given output stream.
   @param game The game to print.
   @param stream The stream to print to.
   @param buffer Which buffer to print -- the visible one or the true one.
 */
void msw_print_buf(smb_mine *game, FILE *stream, char *buffer) {
  int i, j;
  char cell;

  // Print tens row:
  fprintf(stream, "  | ");
  for (i = 0; i < game->columns; i++) {
    if (i % 10 == 0) {
      fputc('0' + i / 10, stream);
    } else {
      fputc(' ', stream);
    }
  }

  // Print the ones row:
  fprintf(stream, "\n  | ");
  for (i = 0; i < game->columns; i++) {
    fputc('0' + i % 10, stream);
  }

  // Print the underline row:
  fprintf(stream, "\n--|-");
  for (i = 0; i < game->columns; i++) {
    fputc('-', stream);
  }
  fputc('\n', stream);

  // Print each row in the game board.
  for (i = 0; i < game->rows; i++) {
    fprintf(stream, "%2d| ", i);
    for (j = 0; j < game->columns; j++) {
      cell = buffer[msw_index(game, i, j)];
      fputc(cell, stream);
    }
    fputc('\n', stream);
  }
}

/**
   @brief Print a minesweeper game's visible board to a stream.
 */
void msw_print(smb_mine *game, FILE *stream) {
  msw_print_buf(game, stream, game->visible);
}

/**
   @brief Dig at a given cell.
   @param game The current game.
   @param row The row to dig at.
   @param column The column to dig at.
   @returns A status variable of sorts.
 */
int msw_dig(smb_mine *game, int row, int column) {
  int n;
  int index = msw_index(game, row, column);

  // If the cell is out of bounds, return some sort of error.
  if (!msw_in_bounds(game, row, column)) {
    return MSW_MBOUND;
  }

  // If the game hasn't started yet (i.e. the grid is null).
  if (game->grid == NULL) {
    // Initialize the game so that we have a 0 at the selected cell.
    msw_initial_grid(game, row, column);
  }

  if (game->grid[index] == MSW_CLEAR && game->visible[index] != MSW_CLEAR) {
    // If the selected cell is clear, and we haven't updated the display with
    // that information, update the display, and then recursively dig at each
    // neighbor.
    game->visible[index] = MSW_CLEAR;
    for (n = 0; n < NUM_NEIGHBORS; n++) {
      msw_dig(game, row + rnbr[n], column + cnbr[n]);
    }
    return MSW_MMOVE;
  } else if (game->visible[index] == MSW_FLAG) {
    // If the selected cell is a flag, do nothing.
    return MSW_FLAGGED;
  } else if (game->grid[index] == MSW_MINE) {
    // If the selected cell is a mine.
    game->visible[index] = MSW_MINE;
    return MSW_MBOOM; // BOOM
  } else {
    // Otherwise, reveal the data in the grid.
    game->visible[index] = game->grid[index];
    return MSW_MMOVE;
  }
}

/**
   @brief Stick a flag in a cell.
 */
int msw_flag(smb_mine *game, int r, int c) {
  int index = msw_index(game, r, c);
  if (game->visible[index] == MSW_UNKNOWN) {
    game->visible[index] = MSW_FLAG;
    return MSW_MMOVE;
  } else {
    return MSW_MFLAGERR;
  }
}

/**
   @brief Unflag a cell.
 */
int msw_unflag(smb_mine *game, int r, int c) {
  int index = msw_index(game, r, c);
  if (game->visible[index] == MSW_FLAG) {
    game->visible[index] = MSW_UNKNOWN;
    return MSW_MMOVE;
  } else {
    return MSW_MUNFLAGERR;
  }
}

/**
   @brief "Reveal" a cell.

   This operation will dig every neighboring cell if the current cell is marked
   n, and has n neighboring cells.
 */
int msw_reveal(smb_mine *game, int r, int c) {
  int n, rv;
  int nflags = 0;
  int nmarks = game->visible[msw_index(game, r, c)] - '0';
  int index = msw_index(game, r, c);

  if (game->visible[index] == MSW_UNKNOWN ||
      game->visible[index] == MSW_MINE ||
      game->visible[index] == MSW_FLAG) {
    return MSW_MREVEALHF;
  }

  // Count the flags around the cell.
  for (n = 0; n < NUM_NEIGHBORS; n++) {
    if (msw_in_bounds(game, r+rnbr[n], c+cnbr[n]) &&
        game->visible[msw_index(game, r+rnbr[n], c+cnbr[n])] == MSW_FLAG) {
      nflags++;
    }
  }

  // If there are at least n flags, we can dig around the cell.
  if (nflags >= nmarks) {
    for (n = 0; n < NUM_NEIGHBORS; n++) {
      if (msw_in_bounds(game, r+rnbr[n], c+cnbr[n])) {
        rv = msw_dig(game, r+rnbr[n], c+cnbr[n]);
        if (!MSW_MOK(rv)) return rv;
      }
    }
    return MSW_MMOVE;
  } else {
    return MSW_MREVEALN;
  }
}

int msw_won(smb_mine *game) {
  int ncells = game->rows * game->columns;
  int i;
  for (i = 0; i < ncells; i++) {
    if (game->grid[i] == MSW_MINE) {
      if (game->visible[i] != MSW_UNKNOWN &&
          game->visible[i] != MSW_FLAG) {
        return 0;
      }
    } else if (game->grid[i] != game->visible[i]) {
      return 0;
    }
  }
  return 1;
}

int msw_quit(smb_mine *game) {
  printf("Aww. Play again soon!\n");
  smb_mine_destroy(game);
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
  smb_mine game;
  int status = MSW_MMOVE;
  char op;

  smb_mine_init(&game, r, c, m);
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
  smb_mine_destroy(&game);
}

void usage(char *name) {
  printf("usage: %s [rows columns [mines]]\n", name);
  printf("\tPlay minesweeper.\n");
  help();
}

/**
   @brief Run a command oriented game of minesweeper.
 */
int main(int argc, char *argv[])
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
