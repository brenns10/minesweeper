Minesweeper
===========

This is a command line minesweeper game in C.


Compiling & Running
-------------------

Just run `make` to build.  The binary is `bin/release/main`.


Playing
-------

To play, run `bin/release/main`.  You're presented with a blank board.  You can
start typing commands.  The commands are a single character, followed by a row,
a comma, and a column.  Here are the possible commands right now:

* `d`: Dig.  This is what clicking normally is in minesweeper.
* `f`: Flag.  This places a flag.
* `u`: Unflag.  This removes a flag.
* `r`: Reveal.  Use this to dig all the neighbors of a cell marked with the
  number *n*, when you have already flagged the *n* neighbors that have mines.

Whitespace doesn't matter too much.  `d 0, 3`, `d0,3`, `d0, 3`, `d 0,3`, and `d
0 , 3` are all OK.

If you blow up, the game should terminate.  If you win, I'm pretty sure the game
won't actually do anything, so you'll just have to exit (`Ctrl-C`) and pat
yourself on your back.
