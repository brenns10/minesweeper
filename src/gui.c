/***************************************************************************//**

  @file         gui.c

  @author       Stephen Brennan

  @date         Wednesday,  8 April 2015

  @brief        GTK Gui for Minesweeper.

*******************************************************************************/

#include <gtk/gtk.h>

void gui_activate(GtkApplication *app, gpointer user_data)
{
  GtkWidget *window;
  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Window");
  gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);
  gtk_widget_show_all(window);
}

int gui_main(int argc, char **argv)
{
  GtkApplication *app;
  int status;

  app = gtk_application_new("com.stephen-brennan.minesweeper",
                            G_APPLICATION_FLAGS_NONE);
  g_signal_connect(app, "activate", G_CALLBACK(gui_activate), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);
  return status;
}
