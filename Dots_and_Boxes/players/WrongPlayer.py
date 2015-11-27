# Tudor Berariu, 2015

from random import randint

class WrongPlayer:
  def __init__(self):
    self.name = "Dan Bittman"

  def move(self, board, score):
    # from time import sleep
    # sleep(0.9)
    row = randint(0, len(board)-1)
    col = randint(0, len(board[row])-1)
    return (row, col)
