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

* `d ROW,COL`: Dig.  This is what clicking normally is in minesweeper.
* `f ROW,COL`: Flag.  This places a flag.
* `u ROW,COL`: Unflag.  This removes a flag.
* `r ROW,COL`: Reveal.  Use this to dig all the neighbors of a cell marked with
  the number *n*, when you have already flagged the *n* neighbors that have
  mines.
* `q`: Quit.  Use this if you're doing really poorly and just want to give up.
* `h`: Help.  Just lists all the commands.

Whitespace doesn't matter too much.  `d 0, 3`, `d0,3`, `d0, 3`, `d 0,3`, and `d
0 , 3` are all OK.  You can also (theoretically) enter multiple commands on one
line (separated by whitespace).


License
-------

This code is under the terms of the Revised BSD License.  See the file
`LICENSE.txt` for more details.
