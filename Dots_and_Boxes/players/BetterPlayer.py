# Tudor Berariu, 2015

from random import choice
from itertools import product, izip, count

class BetterPlayer:
  def __init__(self):
    self.name = "James Hetfield"

  def move(self, board, score):
    cells_height = (len(board) - 1) / 2
    cells_width = len(board[0])
    # Look for a cell with 3 walls
    for r, c in product(range(cells_height), range(cells_width)):
      cells = [(r*2,c), (r*2+2,c), (r*2+1,c), (r*2+1,c+1)]
      if sum(map(lambda (x, y): board[x][y], cells)) == 3:
        return next((x,y) for (x,y) in cells if board[x][y] == 0)
    # If there was no such cell, pick a random one
    good_rows = filter(lambda i: 0 in board[i], range(len(board)))
    row = choice(good_rows)
    col = choice([c for c, val in izip(count(), board[row]) if val == 0])
    return row, col
