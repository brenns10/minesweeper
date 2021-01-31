#include <ncurses.h>
#include "minesweeper.h"

struct msw_curses {
	struct msw game;
	WINDOW *board, *messages;
	unsigned int cur_row, cur_col;
	struct msw_loc size;
};

enum msw_color {
	MC_RED = 0,
	MC_ZERO,
	MC_ONE,
	MC_TWO,
	MC_THREE,
	MC_FOUR,
	MC_FIVE,
	MC_SIX,
	MC_SEVEN,
	MC_EIGHT,
};

static void draw_game(struct msw_curses *mc)
{
	box(mc->board, 0, 0);
	for (unsigned int r = 0; r < mc->game.rows; r++) {
		for (unsigned int c = 0; c < mc->game.columns; c++) {
			wmove(mc->board, r + 1, c + 1);
			char cell = msw_vcell(&mc->game, r, c);
			int toprint;
			switch (cell) {
			case MSW_CLEAR:
				toprint = ' ';
				break;
			case MSW_FLAG:
				toprint = '*' | COLOR_PAIR(MC_RED);
				break;
			case MSW_UNKNOWN:
				toprint = cell;
				break;
			case MSW_MINE:
				toprint = '!' | COLOR_PAIR(MC_RED);
				break;
			default:
				toprint = cell | COLOR_PAIR(MC_ZERO + (cell - '0'));
				break;
			}
			if (r == mc->cur_row && c == mc->cur_col) {
				toprint |= A_REVERSE;
			}
			waddch(mc->board, toprint);
		}
	}
	wnoutrefresh(mc->board);
}

static void init_game(struct msw_curses *mc, int rows, int cols, int mines)
{
	msw_init(&mc->game, rows, cols, mines);

	// NCURSES initialization:
	initscr();            // initialize curses
	start_color();        // well, start color!
	cbreak();             // pass key presses to program, but not signals
	noecho();             // don't echo key presses to screen
	keypad(stdscr, TRUE); // allow arrow keys
	curs_set(0);          // set the cursor to invisible

	/*
	 * For some reason, getch() needs to happen before output renders?
	 * I truly have no idea why this is the case. But in any case, set the
	 * timeout to zero, run getch(), and then go into blocking mode since we
	 * want to wait for key presses during the game.
	 */
	timeout(0);
	getch();
	timeout(-1);

	getmaxyx(stdscr, mc->size.row, mc->size.col);

	// Create window as a good abstraction in case we add other components
	mc->board = newwin(rows + 2, cols + 2, 0, 0);
	mc->cur_row = mc->cur_col = 0;

	mc->messages = newwin(rows + 2, 60, 0, cols + 2);
	scrollok(mc->messages, true);

	init_pair(MC_RED, COLOR_RED, COLOR_BLACK);
	init_pair(MC_ONE, COLOR_BLUE, COLOR_BLACK);
	init_pair(MC_TWO, COLOR_GREEN, COLOR_BLACK);
	init_pair(MC_THREE, COLOR_YELLOW, COLOR_BLACK);
	init_pair(MC_FOUR, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(MC_FIVE, COLOR_RED, COLOR_BLACK);
	init_pair(MC_SIX, COLOR_CYAN, COLOR_BLACK);
	init_pair(MC_SEVEN, COLOR_WHITE, COLOR_BLACK);
	init_pair(MC_EIGHT, COLOR_WHITE, COLOR_BLACK);


	draw_game(mc);
	doupdate();
}

static void destroy_game(struct msw_curses *mc)
{
	wclear(mc->board);
	endwin();
	msw_destroy(&mc->game);
}

static void game_move(struct msw_curses *mc, int r, int c) {
	if (r >= 0 && r < mc->game.rows)
		mc->cur_row = r;
	if (c >= 0 && c < mc->game.columns)
		mc->cur_col = c;
}

void game_loop(struct msw_curses *mc)
{
	int key;
	int status = MSW_MMOVE;
	struct msw_ai_move move;

	while (MSW_MOK(status) && (key = getch()) != 'q') {
		switch (key) {
		case 'h':
			game_move(mc, mc->cur_row, mc->cur_col - 1);
			break;
		case 'j':
			game_move(mc, mc->cur_row + 1, mc->cur_col);
			break;
		case 'k':
			game_move(mc, mc->cur_row - 1, mc->cur_col);
			break;
		case 'l':
			game_move(mc, mc->cur_row, mc->cur_col + 1);
			break;
		case 'd':
			status = msw_dig(&mc->game, mc->cur_row, mc->cur_col);
			break;
		case 'f':
			status = msw_flag(&mc->game, mc->cur_row, mc->cur_col);
			break;
		case 'u':
			status = msw_unflag(&mc->game, mc->cur_row, mc->cur_col);
			break;
		case 'r':
			status = msw_reveal(&mc->game, mc->cur_row, mc->cur_col);
			break;
		case 'a':
			move = msw_ai(&mc->game);
			if (move.action == AI_REVEAL) {
				mc->cur_row = move.loc.row;
				mc->cur_col = move.loc.col;
				status = msw_reveal(&mc->game, mc->cur_row, mc->cur_col);
			} else if (move.action == AI_FLAG) {
				mc->cur_row = move.loc.row;
				mc->cur_col = move.loc.col;
				status = msw_flag(&mc->game, mc->cur_row, mc->cur_col);
			}
			wprintw(mc->messages, "%s\n", move.description);
			wnoutrefresh(mc->messages);
		default:
			break;
		}
		//printf("key: %c, r=%d c=%d\n", key, mc->cur_row, mc->cur_col);
		draw_game(mc);
		doupdate();
	}
}

int curses_main(int argc, char **argv)
{
	int rows = 16, cols = 30, mines = 99;
	struct msw_curses mc;

	// TODO accept row/col/mine arg
	init_game(&mc, rows, cols, mines);
	game_loop(&mc);
	destroy_game(&mc);
	return 0;
}
