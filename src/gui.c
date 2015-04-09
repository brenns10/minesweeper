/***************************************************************************//**

  @file         gui.c

  @author       Stephen Brennan

  @date         Wednesday,  8 April 2015

  @brief        GTK Gui for Minesweeper.

*******************************************************************************/

#include <stdint.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "minesweeper.h"

msw *game;
GtkWidget **buttons;

static gpointer gui_encode_location(unsigned short row, unsigned short col)
{
  uintptr_t p = row;
  p = p << (sizeof(col) * 8);
  p |= col;
  return (gpointer)p;
}

static void gui_decode_location(gpointer p, unsigned short *row, unsigned short *col)
{
  uintptr_t mask = 0;
  mask = (!(!mask) << (sizeof(*col) * 8)); // 000...111111
  *col = (uintptr_t)p & mask;
  *row = (uintptr_t)p >> (sizeof(*col) * 8);
}

static void gui_click(GtkWidget *widget, GdkEvent *event, gpointer data)
{
  unsigned short row, col;
  GdkEventButton *evtBttn = event;
  int status;
  gui_decode_location(data, &row, &col);

  if (evtBttn->button == 3) {
    status = msw_dig(game, row, col);
  }
}

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

  // Allocate space to store our buttons.
  buttons = calloc(game->rows * game->columns, sizeof(GtkWidget*));

  // Create a button for every cell in the game.
  for (i = 0; i < game->rows; i++) {
    for (j = 0; j < game->columns; j++) {
      idx = msw_index(game, i, j);
      button = gtk_button_new_with_label("#");
      g_signal_connect(button, "button-release-event", G_CALLBACK(gui_click),
                       gui_encode_location(i, j));
      gtk_grid_attach(GTK_GRID(grid), button, j, i, 1, 1);
      buttons[idx] = button;
    }
  }
  gtk_widget_show_all(window);
}

int gui_main(int argc, char **argv)
{
  GtkApplication *app;
  int status;

  game = msw_create(10, 10, 20);
  app = gtk_application_new("com.stephen-brennan.minesweeper",
                            G_APPLICATION_FLAGS_NONE);
  g_signal_connect(app, "activate", G_CALLBACK(gui_activate), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);
  return status;
}
