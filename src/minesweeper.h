/***************************************************************************//**

  @file         minesweeper.h

  @author       Stephen Brennan

  @date         Monday, 30 March 2015

  @brief        Declarations for minesweeper game.

*******************************************************************************/

#ifndef MINESWEEPER_H
#define MINESWEEPER_H

typedef struct smb_mine {

  int rows;
  int columns;
  char *mines;
  char *visible;

} smb_mine;

void smb_mine_init(smb_mine *obj, int rows, int columns);
smb_mine *smb_mine_create(int rows, int columns);
void smb_mine_destroy(smb_mine *obj);
void smb_mine_delete(smb_mine *obj);


#endif//MINESWEEPER_H
