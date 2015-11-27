# Tudor Berariu, 2015

from random import choice
from itertools import count, izip

class RandomPlayer:
  def __init__(self):
    self.name = "Cristi Minculescu"

  def move(self, board, score):
    (row_idx, row) = choice([(idx, row) for (idx, row) in zip(count(), board) if 0 in row])
    col_idx = choice([col_idx for col_idx, value in zip(count(), row) if value == 0])
    return (row_idx, col_idx)
