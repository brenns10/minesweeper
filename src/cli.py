#!/usr/bin/env python3
"""Python command line interface driver program for minesweeper."""

import minesweeper as msw
import re


def help():
    print('\nIn-game commands:')
    print('\t- "d ROW,COL" - dig at ROW,COL')
    print('\t- "f ROW,COL" - dig at ROW,COL')
    print('\t- "u ROW,COL" - dig at ROW,COL')
    print('\t- "r ROW,COL" - dig at ROW,COL')
    print('\t- "q" - quit')
    print('\t- "h" - help')


def main():
    game = msw.Minesweeper(10, 10, 20)
    status = msw.MOVE
    cmda = re.compile(r'(?P<cmd>[dfurDFUR]) ?(?P<row>\d+), ?(?P<col>\d+)')
    cmds = re.compile(r'(?P<cmd>[qhQH])')

    while msw.status_ok(status):
        game.print()
        print(msw.get_message(status))
        cmd_str = input("> ")
        match = cmda.match(cmd_str)
        if not match:
            match = cmds.match(cmd_str)
        if not match:
            continue

        if match.group('cmd').upper() == 'D':
            status = game.dig(int(match.group('row')), int(match.group('col')))
        elif match.group('cmd').upper() == 'R':
            status = game.reveal(int(match.group('row')), int(match.group('col')))
        elif match.group('cmd').upper() == 'U':
            status = game.unflag(int(match.group('row')), int(match.group('col')))
        elif match.group('cmd').upper() == 'F':
            status = game.flag(int(match.group('row')), int(match.group('col')))
        elif match.group('cmd').upper() == 'Q':
            return
        elif match.group('cmd').upper() == 'H':
            help()

        if game.won():
            status = msw.WON
            break

    print(msw.get_message(status))


if __name__ == '__main__':
    main()
