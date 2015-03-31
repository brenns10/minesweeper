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
char *MSW_MSG[] = {
  "Make a move.",
  "Cell out of bounds.",
  "Can only flag an unknown cell.",
  "Can't reveal a hidden or flagged cell.",
  "Can't reveal if you haven't flagged all the mines around the cell.",
  "That cell is flagged.",
  "BOOM!"
};

#define MSW_MMOVE 0
#define MSW_MBOUND 1
#define MSW_MFLAGERR 2
#define MSW_MREVEALHF 3
#define MSW_MREVEALN 4
#define MSW_FLAGGED 5
#define MSW_MBOOM 6

/* Macro to determine if the game can continue after a move. */
#define MSW_MOK(x) ((x) != MSW_MBOOM)

/* Game object. */
typedef struct smb_mine {

  char rows;
  char columns;
  char mines;
  char *grid;
  char *visible;

} smb_mine;

/* Construction/destruction. */
void smb_mine_init(smb_mine *obj, int rows, int columns, int mines);
smb_mine *smb_mine_create(int rows, int columns, int mines);
void smb_mine_destroy(smb_mine *obj);
void smb_mine_delete(smb_mine *obj);

/* Utilities. */
int msw_in_bounds(smb_mine *game, int row, int column);
int msw_index(smb_mine *game, int row, int column);
void msw_print(smb_mine *game, FILE *stream);

/* Game actions. */
int msw_dig(smb_mine *game, int row, int column);
int msw_flag(smb_mine *game, int r, int c);
int msw_reveal(smb_mine *game, int r, int c);
int msw_quit(smb_mine *game);

#endif /* MINESWEEPER_H */
