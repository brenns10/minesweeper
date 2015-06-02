/***************************************************************************//**

  @file         minesweeper_module.h

  @author       Stephen Brennan

  @date         Sunday, 31 May 2015

  @brief        Declarations for Python Minesweeper module!

*******************************************************************************/

#ifndef MINESWEEPER_MODULE_H
#define MINESWEEPER_MODULE_H

#include <Python.h>
#include "structmember.h"
#include "minesweeper.h"

typedef struct {
  PyObject_HEAD
  msw ob_game;
} Minesweeper;

#endif//MINESWEEPER_MODULE_H
