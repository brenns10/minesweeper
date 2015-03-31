/***************************************************************************//**

  @file         minesweeper.c

  @author       Stephen Brennan

  @date         Monday, 30 March 2015

  @brief        Minesweeper game implementations.

*******************************************************************************/

#include <stdio.h>  // fprintf, fputc
#include <stdlib.h> // srand, rand
#include <time.h>   // time


#include "libstephen/base.h"
#include "minesweeper.h"

#define NUM_NEIGHBORS 8
char rnbr[NUM_NEIGHBORS] = {-1, -1, -1,  0,  0,  1,  1,  1};
char cnbr[NUM_NEIGHBORS] = {-1,  0,  1, -1,  1, -1,  0,  1};

int msw_in_bounds(smb_mine *game, int row, int column) {
  return (row >= 0 && row < game->rows &&
          column >= 0 && column < game->columns);
}

int msw_index(smb_mine *game, int row, int column) {
  return row * game->columns + column;
}

void msw_generate_grid(smb_mine *obj) {
  int i, j, n;
  int rows = obj->rows;
  int columns = obj->columns;
  int mines = obj->mines;
  int ncells = rows * columns;

  // Initialize the visible board and underlying grid.
  for (i = 0; i < rows*columns; i++) {
    obj->grid[i] = MSW_CLEAR;
  }

  // Naively initialize the mines.
  srand(time(NULL));
  while (mines > 0) {
    i = rand() % ncells;
    if (obj->grid[i] == MSW_CLEAR) {
      obj->grid[i] = MSW_MINE;
      mines--;
    }
  }

  // Initialize the rest of the board.
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

void msw_initial_grid(smb_mine *obj, int r, int c) {
  obj->grid = smb_new(char, obj->rows * obj->columns);
  do {
    msw_generate_grid(obj);
  } while (obj->grid[msw_index(obj, r, c)] != MSW_CLEAR);
}

void smb_mine_init(smb_mine *obj, int rows, int columns, int mines)
{
  int i, j, n;
  int ncells = rows * columns;

  // Initialization logic
  obj->rows = rows;
  obj->columns = columns;
  obj->mines = mines;
  obj->grid = NULL;
  obj->visible = smb_new(char, ncells);

  // Initialize the visible board and underlying grid.
  for (i = 0; i < rows*columns; i++) {
    obj->visible[i] = MSW_UNKNOWN;
  }
}

smb_mine *smb_mine_create(int rows, int columns, int mines)
{
  smb_mine *obj = smb_new(smb_mine, 1);
  smb_mine_init(obj, rows, columns, mines);
  return obj;
}

void smb_mine_destroy(smb_mine *obj)
{
  // Cleanup logic
  smb_free(obj->grid);
  smb_free(obj->visible);
}

void smb_mine_delete(smb_mine *obj) {
  if (obj) {
    smb_mine_destroy(obj);
    smb_free(obj);
  } else {
    fprintf(stderr, "smb_mine_delete: called with null pointer.\n");
  }
}

/**
   @brief Print the current board to the given output stream.
   @param game The game to print.
   @param stream The stream to print to.
   @return The number of bytes printed.
 */
int msw_print_buf(smb_mine *game, FILE *stream, int which) {
  int i, j, printed = 0;
  char *buffer = (which == MSW_PRINT_VISIBLE) ? game->visible : game->grid;
  char cell;

  // Print tens row:
  printed += fprintf(stream, "  | ");
  for (i = 0; i < game->columns; i++) {
    if (i % 10 == 0) {
      fputc('0' + i / 10, stream);
    } else {
      fputc(' ', stream);
    }
    printed++;
  }

  // Print the ones row:
  printed += fprintf(stream, "\n  | ");
  for (i = 0; i < game->columns; i++) {
    fputc('0' + i % 10, stream);
    printed++;
  }

  // Print the underline row:
  printed += fprintf(stream, "\n--|-");
  for (i = 0; i < game->columns; i++) {
    fputc('-', stream);
    printed++;
  }
  fputc('\n', stream);
  printed++;

  // Print each row in the game board.
  for (i = 0; i < game->rows; i++) {
    printed += fprintf(stream, "%2d| ", i);
    for (j = 0; j < game->columns; j++) {
      cell = buffer[msw_index(game, i, j)];
      fputc(cell, stream);
      printed++;
    }
    fputc('\n', stream);
    printed++;
  }
  return printed;
}

int msw_print(smb_mine *game, FILE *stream) {
  return msw_print_buf(game, stream, MSW_PRINT_VISIBLE);
}

int msw_dig(smb_mine *game, int row, int column) {
  int n;
  int index = msw_index(game, row, column);

  if (!msw_in_bounds(game, row, column)) {
    return 0;
  }

  if (game->grid == NULL) {
    msw_initial_grid(game, row, column);
  }

  if (game->grid[index] == MSW_CLEAR && game->visible[index] != MSW_CLEAR) {
    game->visible[index] = MSW_CLEAR;
    for (n = 0; n < NUM_NEIGHBORS; n++) {
      msw_dig(game, row + rnbr[n], column + cnbr[n]);
    }
  } else if (game->grid[index] == MSW_MINE) {
    game->visible[index] = MSW_MINE;
    return -1; // boom
  } else if (game->visible[index] == MSW_FLAG) {
    return 1;
  } else {
    game->visible[index] = game->grid[index];
  }
  return 1;
}

int msw_flag(smb_mine *game, int r, int c) {
  int index = msw_index(game, r, c);
  if (game->visible[index] == MSW_UNKNOWN) {
    game->visible[index] = MSW_FLAG;
  }
  return 1;
}

void cls() {
  printf("\e[1;1H\e[2J");
}

int main(int argc, char *argv[])
{
  smb_mine game;
  smb_status s = SMB_SUCCESS;
  int status = 1;
  int r, c;
  char op;
  char *input;
  smb_mine_init(&game, 10, 10, 20);

  cls();
  msw_print(&game, stdout);
  while (status == 1) {
    printf(">");
    scanf("%c %d, %d", &op, &r, &c);
    if (op == 'd') {
      status = msw_dig(&game, r, c);
    } else {
      status = msw_flag(&game, r, c);
    }
    cls();
    msw_print(&game, stdout);
  }

  smb_mine_destroy(&game);
  return 0;
}
