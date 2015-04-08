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

const char *MSW_MSG[] = {
  "Make a move.",
  "Cell out of bounds.",
  "Can only flag an unknown cell.",
  "Can't reveal a hidden or flagged cell.",
  "Can't reveal if you haven't flagged all the mines around the cell.",
  "That cell is flagged.",
  "Bad command.",
  "BOOM!",
  "Can only unflag a flagged cell.",
  "You win!",
};

/**
   @brief Return whether or not a cell is in bounds.
 */
int msw_in_bounds(msw *game, int row, int column) {
  return (row >= 0 && row < game->rows &&
          column >= 0 && column < game->columns);
}

/**
   @brief Return the index of a cell.
 */
int msw_index(msw *game, int row, int column) {
  return row * game->columns + column;
}

/**
   @brief Randomly generate a grid for this game.
 */
void msw_generate_grid(msw *obj) {
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
void msw_initial_grid(msw *obj, int r, int c) {
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
void msw_init(msw *obj, int rows, int columns, int mines)
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
msw *msw_create(int rows, int columns, int mines)
{
  msw *obj = malloc(sizeof(msw));
  if (obj == NULL) {
    fprintf(stderr, "error: malloc() returned null.\n");
    exit(EXIT_FAILURE);
  }
  msw_init(obj, rows, columns, mines);
  return obj;
}

/**
   @brief Destroy a minesweeper game.
 */
void msw_destroy(msw *obj)
{
  // Cleanup logic
  free(obj->grid);
  free(obj->visible);
}

/**
   @brief Destroy and free a minesweeper game.
 */
void msw_delete(msw *obj) {
  if (obj) {
    msw_destroy(obj);
    free(obj);
  } else {
    fprintf(stderr, "msw_delete: called with null pointer.\n");
  }
}

/**
   @brief Print the current board to the given output stream.
   @param game The game to print.
   @param stream The stream to print to.
   @param buffer Which buffer to print -- the visible one or the true one.
 */
void msw_print_buf(msw *game, FILE *stream, char *buffer) {
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
void msw_print(msw *game, FILE *stream) {
  msw_print_buf(game, stream, game->visible);
}

/**
   @brief Dig at a given cell.
   @param game The current game.
   @param row The row to dig at.
   @param column The column to dig at.
   @returns A status variable of sorts.
 */
int msw_dig(msw *game, int row, int column) {
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
int msw_flag(msw *game, int r, int c) {
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
int msw_unflag(msw *game, int r, int c) {
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
int msw_reveal(msw *game, int r, int c) {
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

int msw_won(msw *game) {
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
