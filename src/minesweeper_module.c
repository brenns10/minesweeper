/***************************************************************************//**

  @file         minesweeper_module.c

  @author       Stephen Brennan

  @date         Sunday, 31 May 2015

  @brief        Python module for Minesweeper!

*******************************************************************************/

#include "minesweeper_module.h"

/*******************************************************************************

                                 Class Methods

*******************************************************************************/

static void Minesweeper_dealloc(Minesweeper *self)
{
  msw_destroy(&self->ob_game);
  Py_TYPE(self)->tp_free((PyObject*)self);
}

static int Minesweeper_init(Minesweeper *self, PyObject *args)
{
  int rows = 0, columns = 0, mines = 0;

  if (!PyArg_ParseTuple(args, "iii", &rows, &columns, &mines))
    return NULL;

  msw_init(&self->ob_game, rows, columns, mines);
  return 0;
}

static PyObject *Minesweeper_in_bounds(Minesweeper *self, PyObject *args)
{
  int row = 0, column = 0, rv;

  if (!PyArg_ParseTuple(args, "ii", &row, &column))
    return NULL;

  rv = msw_in_bounds(&self->ob_game, row, column);
  return PyBool_FromLong(rv);
}

static PyObject *Minesweeper_print(Minesweeper *self)
{
  msw_print(&self->ob_game, stdout);
  Py_RETURN_NONE;
}

static PyObject *Minesweeper_dig(Minesweeper *self, PyObject *args)
{
  int row = 0, column = 0, rv;

  if (!PyArg_ParseTuple(args, "ii", &row, &column))
    return NULL;

  rv = msw_dig(&self->ob_game, row, column);
  return PyLong_FromLong(rv);
}

static PyObject *Minesweeper_flag(Minesweeper *self, PyObject *args)
{
  int row = 0, column = 0, rv;

  if (!PyArg_ParseTuple(args, "ii", &row, &column))
    return NULL;

  rv = msw_flag(&self->ob_game, row, column);
  return PyLong_FromLong(rv);
}

static PyObject *Minesweeper_unflag(Minesweeper *self, PyObject *args)
{
  int row = 0, column = 0, rv;

  if (!PyArg_ParseTuple(args, "ii", &row, &column))
    return NULL;

  rv = msw_unflag(&self->ob_game, row, column);
  return PyLong_FromLong(rv);
}

static PyObject *Minesweeper_reveal(Minesweeper *self, PyObject *args)
{
  int row = 0, column = 0, rv;

  if (!PyArg_ParseTuple(args, "ii", &row, &column))
    return NULL;

  rv = msw_reveal(&self->ob_game, row, column);
  return PyLong_FromLong(rv);
}

static PyObject *Minesweeper_won(Minesweeper *self, PyObject *args)
{
  int rv = msw_won(&self->ob_game);
  return PyBool_FromLong(rv);
}

/*******************************************************************************

                               Class Definitions

*******************************************************************************/

static PyMemberDef Minesweeper_members[] = {
  {"rows", T_BYTE, offsetof(Minesweeper, ob_game) +
   offsetof(msw, rows), READONLY, "rows in the game"},
  {"columns", T_BYTE, offsetof(Minesweeper, ob_game) +
   offsetof(msw, columns), READONLY, "columns in the game"},
  {"mines", T_BYTE, offsetof(Minesweeper, ob_game) +
   offsetof(msw, mines), READONLY, "mines in the game"},
  {NULL} // sentinel
};

static PyMethodDef Minesweeper_methods[] = {
  {"in_bounds", (PyCFunction)Minesweeper_in_bounds, METH_VARARGS,
   "Return True if the given cells are in bounds."},
  {"print", (PyCFunction)Minesweeper_print, METH_NOARGS,
   "Print the game on stdout."},
  {"dig", (PyCFunction)Minesweeper_dig, METH_VARARGS,
   "Dig in a given cell."},
  {"flag", (PyCFunction)Minesweeper_flag, METH_VARARGS,
   "Flag a given cell."},
  {"unflag", (PyCFunction)Minesweeper_unflag, METH_VARARGS,
   "Unflag a given cell."},
  {"reveal", (PyCFunction)Minesweeper_reveal, METH_VARARGS,
   "Reveal at a given cell."},
  {"won", (PyCFunction)Minesweeper_won, METH_NOARGS,
   "Return True if the game is won."},
  {NULL} // sentinel
};

static PyTypeObject minesweeper_MinesweeperType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "minesweeper.Minesweeper",             // name
  sizeof(Minesweeper),                   // size
  0,                                     // item size
  (destructor)Minesweeper_dealloc,       // dealloc
  0,                                     // print
  0,                                     // getattr
  0,                                     // setattr
  0,                                     // reserved
  0,                                     // repr
  0,                                     // as number
  0,                                     // as sequence
  0,                                     // as mapping
  0,                                     // hash
  0,                                     // call
  0,                                     // str
  0,                                     // getattro
  0,                                     // setattro
  0,                                     // as buffer
  Py_TPFLAGS_DEFAULT,                    // flags
  "Minesweeper game.",                   // docstring
  0,                                     // traverse
  0,                                     // clear
  0,                                     // richcompare
  0,                                     // weaklistoffset
  0,                                     // iter
  0,                                     // iternext
  Minesweeper_methods,                   // methods
  Minesweeper_members,                   // members
  0,                                     // getset
  0,                                     // base
  0,                                     // dict
  0,                                     // descr_get
  0,                                     // descr_set
  0,                                     // dictoffset
  (initproc)Minesweeper_init,            // init
  0,                                     // alloc
  PyType_GenericNew,                     // new
};

/*******************************************************************************

                                 Module Methods

*******************************************************************************/

static PyObject *minesweeper_get_message(PyObject *self, PyObject *args)
{
  int message;

  if (!PyArg_ParseTuple(args, "i", &message))
    return NULL;

  return PyUnicode_FromString(MSW_MSG[message]);
}

static PyObject *minesweeper_status_ok(PyObject *self, PyObject *args)
{
  int message;

  if (!PyArg_ParseTuple(args, "i", &message))
    return NULL;

  return PyBool_FromLong(MSW_MOK(message));
}

/*******************************************************************************

                            Module Table Definitions

*******************************************************************************/

static PyMethodDef minesweeper_methods[] = {
  {"get_message", (PyCFunction)minesweeper_get_message, METH_VARARGS,
   "Get the string message associated with a return value."},
  {"status_ok", (PyCFunction)minesweeper_status_ok, METH_VARARGS,
   "Return True if a status message is OK to continue."},
  {NULL}
};

static struct PyModuleDef minesweeper_module = {
  PyModuleDef_HEAD_INIT,
  "minesweeper",      // module name
  NULL,               // docstring
  -1,                 // size of per-interpreter state??
  minesweeper_methods // methods
};

PyMODINIT_FUNC PyInit_minesweeper(void)
{
  PyObject *m;

  minesweeper_MinesweeperType.tp_new = PyType_GenericNew;
  if (PyType_Ready(&minesweeper_MinesweeperType) < 0)
    return NULL;

  m = PyModule_Create(&minesweeper_module);
  if (m == NULL)
    return NULL;

  Py_INCREF(&minesweeper_MinesweeperType);
  PyModule_AddObject(m, "Minesweeper",
                     (PyObject*) &minesweeper_MinesweeperType);

  PyModule_AddIntConstant(m, "MOVE", MSW_MMOVE);
  PyModule_AddIntConstant(m, "BOUND", MSW_MBOUND);
  PyModule_AddIntConstant(m, "FLAGERR", MSW_MFLAGERR);
  PyModule_AddIntConstant(m, "REVEALHF", MSW_MREVEALHF);
  PyModule_AddIntConstant(m, "REVEALN", MSW_MREVEALN);
  PyModule_AddIntConstant(m, "FLAGGED", MSW_FLAGGED);
  PyModule_AddIntConstant(m, "CMD", MSW_CMD);
  PyModule_AddIntConstant(m, "BOOM", MSW_MBOOM);
  PyModule_AddIntConstant(m, "UNFLAGERR", MSW_MUNFLAGERR);
  PyModule_AddIntConstant(m, "WIN", MSW_MWIN);
  return m;
}
