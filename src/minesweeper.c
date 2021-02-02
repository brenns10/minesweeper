/*
 * minesweeper.c: Minesweeper game implementations
 *
 * March 30, 2015
 * Stephen Brennan
 */

#include <stdbool.h> // bool
#include <stdio.h>  // fprintf, fputc, scanf
#include <stdlib.h> // srand, rand, calloc, malloc, free
#include <string.h> // strcmp
#include <time.h>   // time

#include "minesweeper.h"

#define dp(fmt, ...) fprintf(stderr, "%s:%d: " fmt, __FILE__, __LINE__, __VA_ARGS__)

/*
 * Define all eight neighbors for a cell.  The array rnbr is the offset from the
 * row for each neighbor, and the array cnbr is the offset from the column for
 * each neighbor.
 */
#define NUM_NEIGHBORS 8
char rnbr[NUM_NEIGHBORS] = { -1, -1, -1, 0, 0, 1, 1, 1 };
char cnbr[NUM_NEIGHBORS] = { -1, 0, 1, -1, 1, -1, 0, 1 };

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
	"Undo is not supported",
	"End of undo history",
};

struct msw_mark {
	int group_mines;
	int group_count;
	int group_seen;
	int seen_generation;
	struct msw_mark *next;
};

struct msw_ai_percell {
	union {
		struct { /* For cells with a value */
			int flagged_neighbors; // how many neighbors have flags
			int unknown_neighbors; // how many neighbors in unknown state
			int total_neighbors;   // how many neighbors total
			int mine_count;        // how many mines (shown on this cell)

			/* Each valued cell defines a group which we track */
			struct msw_mark mark;
		};
		struct { /* For unknown */
			int markcnt;
			struct msw_mark *marks[8];
		};
	};
};

/**
 * @brief Return whether or not a cell is in bounds.
 */
int msw_in_bounds(msw *game, int row, int column)
{
	return (row >= 0 && row < game->rows && column >= 0 &&
	        column < game->columns);
}

/**
 * @brief Return the index of a cell.
 */
int msw_index(msw *game, int row, int column)
{
	return row * game->columns + column;
}

static inline void msw_set_grid(msw *game, struct msw_loc loc, char val)
{
	game->grid[loc.row * game->columns + loc.col] = val;
}

static inline void msw_set_visible_noundo(msw *game, struct msw_loc loc, char val)
{
	game->visible[loc.row * game->columns + loc.col] = val;
}
static inline void msw_set_visible(msw *game, struct msw_loc loc, char val)
{
	if (game->undo) {
		game->undo[game->undoidx].gen = game->gen;
		game->undo[game->undoidx].loc = loc;
		game->undo[game->undoidx].old = msw_get_visible(game, loc);
		game->undoidx++;
		game->undoidx %= game->undocap;
	}
	msw_set_visible_noundo(game, loc, val);
}

/**
 * @brief Randomly generate a grid for this game.
 */
void msw_generate_grid(msw *obj)
{
	int i, j;
	char tmp;
	int mines = obj->mines;
	int ncells = obj->rows * obj->columns;
	struct msw_loc loc, neigh;
	int iter;

	// Initialize the grid.
	for (i = 0; i < ncells; i++) {
		obj->grid[i] = MSW_CLEAR;
	}

	// Add the mines.
	for (i = 0; i < mines; i++) {
		obj->grid[i] = MSW_MINE;
	}

	// Shuffle the mines. (Fisher-Yates)
	for (i = ncells - 1; i > 0; i--) {
		j = rand() % (i + 1);
		tmp = obj->grid[j];
		obj->grid[j] = obj->grid[i];
		obj->grid[i] = tmp;
	}

	// Count adjacent mines.
	for_each_row_col(obj, loc)
	{
		tmp = msw_get_grid(obj, loc);

		if (tmp == MSW_MINE)
			continue;

		for_each_neigh(obj, neigh, &loc, iter)
		{
			if (msw_get_grid(obj, neigh) == MSW_MINE)
				tmp++;
		}
		msw_set_grid(obj, loc, tmp);
	}
}

/**
 * @brief Create the initial grid for a game.
 * @param obj The game.
 * @param r The row of the first dig.
 * @param c The column of the first dig.
 *
 * When a user first digs, their dig should always land on a cell that is clear.
 * This ensures that they will have at least a little bit of information to
 * start with.  So, we keep generating grids until we get one where their
 * initial selection is clear.
 */
void msw_initial_grid(msw *obj, int r, int c)
{
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
 * @brief Initialize a minesweeper game.
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
	obj->ai = calloc(ncells, sizeof(struct msw_ai_percell));
	obj->undo = NULL;
	obj->gen = 1;
	obj->undoidx = 0;
	obj->flags = 0;
	if (obj->visible == NULL) {
		fprintf(stderr, "error: calloc() returned null.\n");
		exit(EXIT_FAILURE);
	}

	// Initialize the visible board and underlying grid.
	for (i = 0; i < ncells; i++) {
		obj->visible[i] = MSW_UNKNOWN;
	}
}

void msw_enable_undo_logging(msw *obj, int cap)
{
	if (!obj->undo) {
		obj->undo = calloc(cap, sizeof(struct msw_undo_entry));
		obj->undoidx = 1;
		obj->undocap = cap;
		obj->undo[0].gen = obj->gen - 1;
		obj->gen = 2;
	}
}

void msw_end_turn(msw *obj)
{
	/* only increment generation if changes were made */
	if (obj->undo[(obj->undoidx - 1) % obj->undocap].gen == obj->gen)
		obj->gen++;
}

int msw_undo(msw *obj)
{
	if (!obj->undo)
		return MSW_MNOUNDO;
	int count = 0;
	dp("undo generation %d, idx %d\n", obj->gen, obj->undoidx);
	for (obj->undoidx = (obj->undoidx - 1) % obj->undocap;
	     obj->undo[obj->undoidx].gen == obj->gen - 1;
	     obj->undoidx = (obj->undoidx - 1) % obj->undocap) {
		msw_set_visible_noundo(obj, obj->undo[obj->undoidx].loc,
				       obj->undo[obj->undoidx].old);
		obj->undo[obj->undoidx].gen = 0;
		count += 1;
	}
	obj->undoidx = (obj->undoidx + 1) % obj->undocap;
	if (!count) {
		dp("cannot undo any more, %d, idx %d\n", obj->gen, obj->undoidx);
		return MSW_MENDUNDO;
	}
	obj->gen -= 1;
	return MSW_MMOVE;
}

/**
 * @brief Create a minesweeper game.
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
 * @brief Destroy a minesweeper game.
 */
void msw_destroy(msw *obj)
{
	// Cleanup logic
	free(obj->grid);
	free(obj->visible);
	free(obj->ai);
	free(obj->undo);
}

/**
 * @brief Destroy and free a minesweeper game.
 */
void msw_delete(msw *obj)
{
	if (obj) {
		msw_destroy(obj);
		free(obj);
	} else {
		fprintf(stderr, "msw_delete: called with null pointer.\n");
	}
}

/**
 * @brief Print the current board to the given output stream.
 * @param game The game to print.
 * @param stream The stream to print to.
 * @param buffer Which buffer to print -- the visible one or the true one.
 */
void msw_print_buf(msw *game, FILE *stream, char *buffer)
{
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
 * @brief Print a minesweeper game's visible board to a stream.
 */
void msw_print(msw *game, FILE *stream)
{
	msw_print_buf(game, stream, game->visible);
}

/**
 * @brief Dig at a given cell.
 * @param game The current game.
 * @param row The row to dig at.
 * @param column The column to dig at.
 * @returns A status variable of sorts.
 */
int msw_dig(msw *game, int row, int column)
{
	int iter;
	struct msw_loc neigh, loc = {.row=row, .col=column};

	// If the cell is out of bounds, return some sort of error.
	if (!msw_in_bounds(game, row, column)) {
		return MSW_MBOUND;
	}

	// If the game hasn't started yet (i.e. the grid is null).
	if (game->grid == NULL) {
		// Initialize the game so that we have a 0 at the selected cell.
		msw_initial_grid(game, row, column);
	}

	if (msw_get_grid(game, loc) == MSW_CLEAR &&
	    msw_get_visible(game, loc) != MSW_CLEAR) {
		// If the selected cell is clear, and we haven't updated the
		// display with that information, update the display, and then
		// recursively dig at each neighbor.
		msw_set_visible(game, loc, MSW_CLEAR);
		for_each_neigh(game, neigh, &loc, iter)
		{
			msw_dig(game, neigh.row, neigh.col);
		}
		return MSW_MMOVE;
	} else if (msw_get_visible(game, loc) == MSW_FLAG) {
		// If the selected cell is a flag, do nothing.
		return MSW_FLAGGED;
	} else if (msw_get_grid(game, loc) == MSW_MINE) {
		// If the selected cell is a mine.
		msw_set_visible(game, loc, MSW_MINE);
		return MSW_MBOOM; // BOOM
	} else {
		// Otherwise, reveal the data in the grid.
		msw_set_visible(game, loc, msw_get_grid(game, loc));
		return MSW_MMOVE;
	}
}

/**
 * @brief Stick a flag in a cell.
 */
int msw_flag(msw *game, int r, int c)
{
	struct msw_loc loc = {.row=r, .col=c};
	if (msw_get_visible(game, loc) == MSW_UNKNOWN) {
		msw_set_visible(game, loc, MSW_FLAG);
		game->flags++;
		return MSW_MMOVE;
	} else {
		return MSW_MFLAGERR;
	}
}

/**
 * @brief Unflag a cell.
 */
int msw_unflag(msw *game, int r, int c)
{
	struct msw_loc loc = {.row=r, .col=c};
	if (msw_get_visible(game, loc) == MSW_FLAG) {
		msw_set_visible(game, loc, MSW_UNKNOWN);
		game->flags--;
		return MSW_MMOVE;
	} else {
		return MSW_MUNFLAGERR;
	}
}

/**
 * @brief "Reveal" a cell.
 *
 * This operation will dig every neighboring cell if the current cell is marked
 * n, and has n neighboring cells.
 */
int msw_reveal(msw *game, int r, int c)
{
	int rv, iter;
	int nflags = 0;
	int nmarks = game->visible[msw_index(game, r, c)] - '0';
	struct msw_loc neigh, loc = {.row=r, .col=c};
	char val = msw_get_visible(game, loc);

	if (val == MSW_UNKNOWN || val == MSW_MINE || val == MSW_FLAG) {
		return MSW_MREVEALHF;
	}

	// Count the flags around the cell.
	for_each_neigh(game, neigh, &loc, iter)
	{
		if (msw_get_visible(game, neigh) == MSW_FLAG) {
			nflags++;
		}
	}

	// If there are at least n flags, we can dig around the cell.
	if (nflags >= nmarks) {
		for_each_neigh(game, neigh, &loc, iter)
		{
			rv = msw_dig(game, neigh.row, neigh.col);
			if (!MSW_MOK(rv))
				return rv;
		}
		return MSW_MMOVE;
	} else {
		return MSW_MREVEALN;
	}
}

int msw_won(msw *game)
{
	struct msw_loc loc;
	char val, vis;
	for_each_row_col(game, loc)
	{
		val = msw_get_grid(game, loc);
		vis = msw_get_visible(game, loc);
		if (val == MSW_MINE) {
			if (vis != MSW_UNKNOWN && vis != MSW_FLAG) {
				return 0;
			}
		} else if (val != vis) {
			return 0;
		}
	}
	return 1;
}

static inline struct msw_ai_percell *msw_get_percell(msw *game, struct msw_loc loc)
{
	struct msw_ai_percell *ptr = game->ai;
	return &ptr[loc.row * game->columns + loc.col];
}

static void msw_ai_add_mark(struct msw_ai_percell *pc, struct msw_mark *mark)
{
	pc->marks[pc->markcnt] = mark;
	pc->markcnt++;
}

static void msw_observe_mark(int generation, struct msw_mark *mark, struct msw_mark **full)
{
	if (mark->seen_generation != generation) {
		mark->seen_generation = generation;
		mark->group_seen = 0;
	}
	mark->group_seen++;
	if (mark->group_seen == mark->group_count) {
		mark->next = *full;
		*full = mark;
	}
}

static struct msw_ai_move msw_ai_fill_cell(msw *game, struct msw_loc loc)
{
	struct msw_loc neigh;
	struct msw_ai_percell *pc;
	struct msw_ai_move move;
	char val, neighval;
	int iter;

	move.action = AI_NONE;
	val = msw_get_visible(game, loc);
	pc = msw_get_percell(game, loc);

	if (val == MSW_UNKNOWN || val == MSW_CLEAR || val == MSW_FLAG)
		return move;

	pc->mine_count = val - '0';

	// count flagged / unknown neighbors
	for_each_neigh(game, neigh, &loc, iter)
	{
		neighval = msw_get_visible(game, neigh);
		pc->total_neighbors += 1;
		if (neighval == MSW_FLAG)
			pc->flagged_neighbors += 1;
		else if (neighval == MSW_UNKNOWN)
			pc->unknown_neighbors += 1;
	}

	if (pc->flagged_neighbors == pc->mine_count) {
		/* All mines accounted for. Reveal if necessary, we're done here. */
		if (pc->unknown_neighbors > 0) {
			move.action = AI_REVEAL;
			move.loc = loc;
			move.description = "Reveal (flag count matches cell count)";
		}
		return move;
	}
	if (pc->flagged_neighbors + pc->unknown_neighbors == pc->mine_count) {
		/* All unknowns are mines, flag them. */
		move.action = AI_FLAG;
		move.description = "Flag (only option for remaining unknowns)";
		for_each_neigh(game, neigh, &loc, iter)
		{
			neighval = msw_get_visible(game, neigh);
			if (neighval == MSW_UNKNOWN) {
				move.loc = neigh;
				return move;
			}
		}
		return move;
	}

	/* at this point, we have more unknowns than mines, define group */
	pc->mark.group_mines = pc->mine_count - pc->flagged_neighbors;
	pc->mark.group_count = pc->unknown_neighbors;
	for_each_neigh(game, neigh, &loc, iter)
	{
		neighval = msw_get_visible(game, neigh);
		if (neighval == MSW_UNKNOWN) {
			msw_ai_add_mark(msw_get_percell(game, neigh), &pc->mark);
		}
	}
	return move;
}

static struct msw_ai_move msw_ai_move_first_unmarked_neigh(
	msw *game, struct msw_loc loc, struct msw_mark *mark, int action, char *description)
{
	struct msw_loc neigh;
	struct msw_ai_percell *npc;
	char neighval;
	int iter, idx;
	bool has_mark;
	struct msw_ai_move move;
	move.action = AI_NONE;

	for_each_neigh(game, neigh, &loc, iter)
	{
		npc = msw_get_percell(game, neigh);
		neighval = msw_get_visible(game, neigh);
		if (neighval != MSW_UNKNOWN)
			continue;
		has_mark = false;
		for (idx = 0; idx < npc->markcnt; idx++) {
			if (npc->marks[idx] == mark) {
				has_mark = true;
				break;
			}
		}
		if (!has_mark) {
			return (struct msw_ai_move) {
				.loc=neigh,
				.action=action,
				.description=description,
			};
		}
	}
	dp("ERROR: assertion failed - no first unmarked neighbor (%d, %d)\n", loc.row, loc.col);
	return move;
}

static struct msw_ai_move msw_ai_process_groups(msw *game, struct msw_loc loc)
{
	/*
	 * No obvious moves exist.
	 *
	 * We have already made each cell part of multiple "groups" which place
	 * constraints on the members. Now, let's do some reasoning based on the
	 * groups.
	 */
	struct msw_loc neigh;
	struct msw_ai_percell *pc, *npc;
	char val, neighval;
	struct msw_mark *full = NULL;
	int iter, idx;
	struct msw_ai_move move;

	move.action = AI_NONE;
	pc = msw_get_percell(game, loc);
	val = msw_get_visible(game, loc);

	/* We can only do this analysis for cells which have a revealed value of
	 * 1 or greater */
	if (val < '1' || val >= '9')
		return move;
	if (pc->unknown_neighbors == 0)
		return move; // bail out early if there are no unknowns

	for_each_neigh(game, neigh, &loc, iter)
	{
		npc = msw_get_percell(game, neigh);
		neighval = msw_get_visible(game, neigh);
		if (neighval != MSW_UNKNOWN)
			continue;
		/* for every mark, "observe" it and add it to the full list if
		 * we've observed every cell with the same mark as a neighbor to
		 * this cell */
		for (idx = 0; idx < npc->markcnt; idx++) {
			msw_observe_mark(loc.row * game->columns + loc.col,
					 npc->marks[idx], &full);
		}
	}

	for (; full; full = full->next) {
		/* Can't do anything with our mark */
		if (full == &pc->mark)
			continue;
		dp("at (%d, %d) a full group %p\n", loc.row, loc.col, full);
		/* Can't do anything if the group is all neighbors */
		if (full->group_count >= pc->unknown_neighbors)
			continue;
		int remaining_mines = pc->mine_count - pc->flagged_neighbors - full->group_mines;
		int remaining_unknowns = pc->unknown_neighbors - full->group_count;
		dp("-> LOC: fn=%d un=%d tn=%d, mc=%d\n", pc->flagged_neighbors, pc->unknown_neighbors,
			pc->total_neighbors,
			pc->mine_count);
		dp("-> GROUP: mines=%d count=%d\n", full->group_mines, full->group_count);
		dp("-> COMB: remaining_mines=%d remaining_unknowns=%d\n", remaining_mines, remaining_unknowns);
		/* If the remaining mines is 0, dig all neighbors not in group.
		 * If the remaining mines is equal to all non-group neighbors, flag
		 * them all! */
		if (remaining_mines == 0)
			return msw_ai_move_first_unmarked_neigh(game, loc, full, AI_DIG,
				"Dig because others are superset explaining remainder");
		else if (remaining_mines == remaining_unknowns)
			return msw_ai_move_first_unmarked_neigh(game, loc, full, AI_FLAG,
				"Flag because others are superset explaining remainder");
	}
	return move;
}

struct msw_ai_move msw_ai(msw *game)
{
	struct msw_loc loc;
	struct msw_ai_move move;

	memset(game->ai, 0, sizeof(struct msw_ai_percell) * game->rows * game->columns);
	for_each_row_col(game, loc)
	{
		move = msw_ai_fill_cell(game, loc);
		if (move.action != AI_NONE)
			return move;
	}

	dp("Stumped: trying groups%c", '\n');

	for_each_row_col(game, loc)
	{
		move = msw_ai_process_groups(game, loc);
		if (move.action != AI_NONE)
			return move;
	}

	return (struct msw_ai_move) {
		.action = AI_NONE,
		.loc = (struct msw_loc){ .row=0, .col=0 },
		.description = "I'm stumped!",
	};
}
