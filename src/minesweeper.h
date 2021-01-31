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

struct msw_loc {
	int row;
	int col;
};

enum msw_ai_action {
	AI_NONE,
	AI_DIG,
	AI_REVEAL,
	AI_FLAG,
};

struct msw_ai_move {
	const char *description;
	int action;
	struct msw_loc loc;
};


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
struct msw_ai_move msw_ai(msw *game);

/* UI's */
int gui_main(int argc, char **argv);
int cli_main(int argc, char **argv);
int curses_main(int argc, char **argv);

#define for_each_row_col(pgame, LVAR) \
	for (LVAR.row = 0; LVAR.row < (pgame)->rows; LVAR.row++) \
		for (LVAR.col = 0; LVAR.col < (pgame)->columns; LVAR.col++)

#define for_each_neigh(game, NEIGHVAR, PLOC, IVAR) \
	for (NEIGHVAR = (struct msw_loc){.row=(PLOC)->row + rnbr[0], .col=(PLOC)->col + cnbr[0]}, IVAR = 0; \
	     IVAR < NUM_NEIGHBORS; \
	     IVAR++, NEIGHVAR = (struct msw_loc){.row=(PLOC)->row + rnbr[IVAR], .col=(PLOC)->col + cnbr[IVAR]}) \
	     if (msw_inbound(game, NEIGHVAR))

static inline int msw_inbound(msw *game, struct msw_loc loc)
{
	return (loc.row >= 0 && loc.row < game->rows && loc.col >= 0 &&
	        loc.col < game->columns);
}

static inline char msw_get_grid(msw *game, struct msw_loc loc)
{
	return game->grid[loc.row * game->columns + loc.col];
}

static inline char msw_get_visible(msw *game, struct msw_loc loc)
{
	return game->visible[loc.row * game->columns + loc.col];
}

#endif /* MINESWEEPER_H */
