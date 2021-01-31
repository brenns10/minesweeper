/***************************************************************************//**

  @file         minesweeper.h

  @author       Stephen Brennan

  @date         Monday, 30 March 2015

  @brief        Declarations for minesweeper game.

*******************************************************************************/

#ifndef MINESWEEPER_H
#define MINESWEEPER_H

/*
  Characters for each cell in minesweeper.
 */
#define MSW_CLEAR   '0'
#define MSW_MINE    '!'
#define MSW_FLAG    'F'
#define MSW_UNKNOWN '#'

/*
  Messages for user interface.
 */
extern const char *MSW_MSG[];

#define MSW_MMOVE 0
#define MSW_MBOUND 1
#define MSW_MFLAGERR 2
#define MSW_MREVEALHF 3
#define MSW_MREVEALN 4
#define MSW_FLAGGED 5
#define MSW_CMD 6
#define MSW_MBOOM 7
#define MSW_MUNFLAGERR 8
#define MSW_MWIN 9

/* Macro to determine if the game can continue after a move. */
#define MSW_MOK(x) ((x) != MSW_MBOOM)

#define msw_vcell(pgame, r, c) (pgame)->visible[(r) * (pgame)->columns + (c)]

/* Game object. */
typedef struct msw {

  char rows;
  char columns;
  char mines;
  char *grid;
  char *visible;

} msw;

/* Construction/destruction. */
void msw_init(msw *obj, int rows, int columns, int mines);
msw *msw_create(int rows, int columns, int mines);
void msw_destroy(msw *obj);
void msw_delete(msw *obj);

/* Utilities. */
int msw_in_bounds(msw *game, int row, int column);
int msw_index(msw *game, int row, int column);
void msw_print(msw *game, FILE *stream);

/* Game actions. */
int msw_dig(msw *game, int row, int column);
int msw_flag(msw *game, int r, int c);
int msw_unflag(msw *game, int r, int c);
int msw_reveal(msw *game, int r, int c);
int msw_won(msw *game);

/* UI's */
int gui_main(int argc, char **argv);
int cli_main(int argc, char **argv);
int curses_main(int argc, char **argv);

#endif /* MINESWEEPER_H */
