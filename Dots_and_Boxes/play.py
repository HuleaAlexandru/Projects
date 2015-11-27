# Tudor Berariu, 2015

def display_board(board, score):
  ch = {0: ' ', 1: '-'}
  cv = {0: ' ', 1: '|'}
  print "*" + "*".join(map(lambda x: ch[x], board[0])) + "*"
  for i in range(len(board) / 2):
    line = ""
    for j in range(len(board[i*2+1]) - 1):
      line = line + cv[board[i*2+1][j]]
      if board[i*2][j] + board[i*2+1][j] + board[i*2+1][j+1] + board[i*2+2][j] == 4:
        line = line + 'X'
      else:
        line = line + ' '
    line = line + cv[board[i*2+1][-1]]
    print line
    print "*" + "*".join(map(lambda x: ch[x], board[i*2+2])) + "*"
  print "Score: %d - %d" % score

class HumanPlayer:
  def __init__(self):
    self.name = "Human Player"

  def move(self, board, score):
    display_board(board, score)
    r = int(raw_input("Row: "))
    c = int(raw_input("Col: "))
    while board[r][c]:
      display_board(board, score)
      r = int(raw_input("Row: "))
      c = int(raw_input("Col: "))
    return (r, c)

if __name__ == "__main__":
  from game_server import play_game
  from players.HuleaAlexandru19930205 import HuleaAlexandru19930205
  result = play_game(HumanPlayer(), HuleaAlexandru19930205(), 5, 5, False)
  print "Final score: %d - %d (%s)" % result