/***************************************************************************//**

  @file         minesweeper.h

  @author       Stephen Brennan

  @date         Monday, 30 March 2015

  @brief        Declarations for minesweeper game.

*******************************************************************************/

#ifndef MINESWEEPER_H
#define MINESWEEPER_H

#define MSW_CLEAR   '0'
#define MSW_MINE    '!'
#define MSW_FLAG    'F'
#define MSW_UNKNOWN '#'

typedef struct smb_mine {

  char rows;
  char columns;
  char mines;
  char *grid;
  char *visible;

} smb_mine;

void smb_mine_init(smb_mine *obj, int rows, int columns, int mines);
smb_mine *smb_mine_create(int rows, int columns, int mines);
void smb_mine_destroy(smb_mine *obj);
void smb_mine_delete(smb_mine *obj);

#define MSW_PRINT_VISIBLE 0
#define MSW_PRINT_ALL 1

#endif // MINESWEEPER_H
