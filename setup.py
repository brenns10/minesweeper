#!/usr/bin/env python3
"""Setup for minesweeper module."""

from distutils.core import setup, Extension


setup(
    name='minesweeper',
    version='1.0',
    ext_modules=[
        Extension('minesweeper',
                  ['src/minesweeper.c', 'src/minesweeper_module.c']),
    ],
)
