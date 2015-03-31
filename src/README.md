Minesweeper
===========

This is a command line minesweeper game in C.


Compiling & Running
-------------------

This program "depends" on my
[libstephen](https://github.com/brenns10/libstephen) library.  Really the only
purpose it serves right now is to provide my usual memory allocation macros,
which do error checking for me.  If my future implementations still don't
require any exciting features of my library, I'll remove the dependency.  But
since it's there right now, my git repo has a submodule, and that complicates
things slightly.

```bash
git clone https://github.com/brenns10/minesweeper
cd minesweeper
git submodule init
git submodule update
make
bin/release/main
```


Playing
-------

To play, run `bin/release/main`.  You're presented with a blank board.  You can
start typing commands.  The commands are a single character, followed by a row,
a comma, and a column.  Here are the possible commands right now:

* `d`: Dig.  This is what clicking normally is in minesweeper.
* `f`: Flag.  This places a flag.

Whitespace doesn't matter too much.  `d 0, 3`, `d0,3`, `d0, 3`, and `d 0,3` are
all OK.  I guess the only thing you can't do is put a space between the row and
the comma.

If you blow up, the game should terminate.  If you win, I'm pretty sure the game
won't actually do anything, so you'll just have to exit (`Ctrl-C`) and pat
yourself on your back.
