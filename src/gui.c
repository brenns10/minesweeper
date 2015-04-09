/***************************************************************************//**

  @file         gui.c

  @author       Stephen Brennan

  @date         Wednesday,  8 April 2015

  @brief        GTK Gui for Minesweeper.

*******************************************************************************/

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

#include "minesweeper.h"

msw *game;
GtkWidget **buttons;
GtkWidget *label;

/**
   @brief Encode a minsweeper location into a gpointer.
 */
static gpointer gui_encode_location(unsigned short row, unsigned short col)
{
  uintptr_t p = row;
  p = p << (sizeof(col) * 8);
  p |= col;
  return (gpointer)p;
}

/**
   @brief Decode a minesweeper location from a gpointer into a pair of vars.
 */
static void gui_decode_location(gpointer p, unsigned short *row, unsigned short *col)
{
  uintptr_t mask = 0;
  mask = 0xFFFF;
  *col = (uintptr_t)p & mask;
  *row = (uintptr_t)p >> (sizeof(*col) * 8);
}

/**
   @brief Get the label associated with a minesweeper character.
 */
static char *gui_label(char c)
{
  switch (c) {
  case '0':
    return "0";
  case '1':
    return "1";
  case '2':
    return "2";
  case '3':
    return "3";
  case '4':
    return "4";
  case '5':
    return "5";
  case '6':
    return "6";
  case '7':
    return "7";
  case '8':
    return "8";
  case '#':
    return " ";
  case '!':
    return "!";
  case 'F':
    return "F";
  default:
    return "?";
  }
}

/**
   @brief Draw the labels onto the buttons and the status.
 */
static void gui_draw(int status)
{
  int max = game->rows * game->columns;
  int i;
  for (i = 0; i < max; i++) {
    gtk_button_set_label((GtkButton*) buttons[i], gui_label(game->visible[i]));
  }
  gtk_label_set_text(GTK_LABEL(label), MSW_MSG[status]);
}

/**
   @brief Handle a button click on the grid.
 */
static void gui_click(GtkWidget *widget, GdkEvent *event, gpointer data)
{
  unsigned short row, col;
  GdkEventButton *evtBttn = (GdkEventButton*)event;
  GtkWidget *dialog;
  GtkWidget *window = gtk_widget_get_toplevel(widget);
  int status;
  gui_decode_location(data, &row, &col);

  if (evtBttn->button == 1) {
    // Left click = DIG.
    status = msw_dig(game, row, col);
  } else if (evtBttn->button == 2) {
    // Middle click = REVEAL.
    status = msw_reveal(game, row, col);
  } else if (evtBttn->button == 3) {
    // Right click = FLAG.
    status = msw_flag(game, row, col);
    if (status == MSW_MFLAGERR) {
      // If flag failed, there must be a flag there already, so unflag.
      status = msw_unflag(game, row, col);
      if (status == MSW_MUNFLAGERR) {
        // If that failed too, return the original error.
        status = MSW_MFLAGERR;
      }
    }
  }

  // Draw the GUI after that.
  gui_draw(status);

  // Handle win/loss cases.
  if (msw_won(game)) {
    dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                    GTK_MESSAGE_INFO,
                                    GTK_BUTTONS_OK,
                                    "You won!");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(window);
  } else if (status == MSW_MBOOM) {
    dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                    GTK_MESSAGE_INFO,
                                    GTK_BUTTONS_OK,
                                    "You lost!");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(window);
  }
}

/**
   @brief "Activate" the GUI.  Called by my GTK application.
 */
void gui_activate(GtkApplication *app, gpointer user_data)
{
  GtkWidget *window;
  GtkWidget *grid;
  GtkWidget *button;
  int i, j, idx;

  // Create a window.
  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Minesweeper");
  gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);

  // Create a grid to put my game in.
  grid = gtk_grid_new();
  gtk_container_add(GTK_CONTAINER(window), grid);

  // Create the status label
  label = gtk_label_new("Make a move.");

  // Allocate space to store our buttons.
  buttons = calloc(game->rows * game->columns, sizeof(GtkWidget*));

  // Create a button for every cell in the game.
  for (i = 0; i < game->rows; i++) {
    for (j = 0; j < game->columns; j++) {
      idx = msw_index(game, i, j);
      button = gtk_button_new_with_label(" ");
      g_signal_connect(button, "button-release-event", G_CALLBACK(gui_click),
                       gui_encode_location(i, j));
      gtk_grid_attach(GTK_GRID(grid), button, j, i, 1, 1);
      buttons[idx] = button;
    }
  }
  gtk_grid_attach(GTK_GRID(grid), label, 0, game->rows, game->columns, 1);
  gtk_widget_show_all(window);
}

/**
   @brief Display usage about this program.
 */
static void usage(char *name) {
  printf("usage: %s [rows columns [mines]]\n", name);
  printf("\tPlay minesweeper.\n");
}


/**
   @brief Run the GUI, given rows, columns, and mines.
 */
static int gui_run(int argc, char **argv, int r, int c, int m)
{
  GtkApplication *app;
  int status;

  game = msw_create(r, c, m);
  app = gtk_application_new("com.stephen-brennan.minesweeper",
                            G_APPLICATION_FLAGS_NONE);
  g_signal_connect(app, "activate", G_CALLBACK(gui_activate), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  msw_delete(game);
  free(buttons);
  return status;
}

/**
   @brief Run a GTK game of minesweeper.
 */
int gui_main(int argc, char *argv[])
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

    if (argc >= 4) {
      sscanf(argv[3], "%d", &m);
      argv += 3;
      argc -= 3;
      if (m <= 0 || m > r*c) {
        fprintf(stderr, "error: bad number of mines (%d)\n", m);
        return EXIT_FAILURE;
      }
    } else {
      m = 20;
      argv += 2;
      argc -= 2;
    }
  } else {
    r = c = 10;
    m = 20;
  }

  gui_run(argc, argv, r,c,m);

  return 0;
}
