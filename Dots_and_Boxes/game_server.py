# Tudor Berariu, 2015

from itertools import product
from signal import signal, alarm, SIGALRM

def get_move(board, score, move_fnc, timeout):
  if timeout:
    def handler(signum, frame):
      raise Exception("Timeout!")
    signal(SIGALRM, handler)
    alarm(1)
    try:
      move = move_fnc(board, score)
    finally:
      alarm(0)
    return move
  else:
    return move_fnc(board, score)

def play_game(player0, player1, height=10, width=10, timeout=True):
  cells_no = height * width
  board = [[0 for col in range(width + row % 2)] for row in range(height * 2 + 1)]
  score = (0,0)
  current = 0
  players = [player0, player1]
  while next((row for row in board if 0 in row), False):
    scored = False
    try:
      (r, c) = get_move(board, (score[current], score[1-current]), players[current].move, timeout)
      if board[r][c] != 0:
        # This means your algorithm sucks!
        return (cells_no * current, cells_no * (1-current), "wrong move")
      else:
        board[r][c] = 1
        if r % 2 == 0:
          # Check if cell above is closed now
          if r > 0 and sum([board[x][y] for (x,y) in [(r-2,c), (r-1,c), (r-1,c+1)]]) == 3:
            score = (score[0] + (1-current), score[1] + current)
            scored = True
          # Check if cell below is closed now
          if r < 2 * height and sum([board[x][y] for (x,y) in [(r+2,c), (r+1,c), (r+1,c+1)]]) == 3:
            score = (score[0] + (1-current), score[1] + current)
            scored = True
        else:
          # Check if cell on left is closed now
          if c > 0 and sum([board[x][y] for (x,y) in [(r-1,c-1), (r, c-1), (r+1, c-1)]]) == 3:
            score = (score[0] + (1-current), score[1] + current)
            scored = True
          # Check if cell on right is closed now
          if c < width and sum([board[x][y] for (x,y) in [(r-1,c), (r, c+1), (r+1, c)]]) == 3:
            score = (score[0] + (1-current), score[1] + current)
            scored = True
    except Exception as e:
      return (cells_no * current, cells_no * (1-current), str(e))
    if not scored:
      current = 1 -current
  return (score[0], score[1], "ok")

def get_players():
  from os import listdir
  from os.path import isfile, join
  from imp import load_source
  import inspect

  dir_path = "./players/"
  loader = lambda f: (f.strip(".py"), load_source(f.strip(".py"), join(dir_path, f)))
  is_player_file = lambda f: isfile(join(dir_path, f)) and f.endswith(".py") and not f.startswith("__")
  modules = [loader(f) for f in listdir(dir_path) if is_player_file(f)]

  players = []
  for name, module in modules:
    cls = next(obj[1] for obj in inspect.getmembers(module) if obj[0] == name)
    assert(inspect.isclass(cls))
    players.append(cls)

  return players

def export_results(stats):
  from jinja2 import Environment, FileSystemLoader
  env = Environment(loader=FileSystemLoader(searchpath="./templates/"))
  with open("stats.tex", "w") as out_file:
    out_file.write(env.get_template("template.tex").render({"stats": stats}))

def print_results(stats):
  print "{0:25s} | {1:5s} | {2:5s} | {3:10s}".format("Name", "Wins", "Loses", "Score")
  print "-" * 54
  for s in stats:
    print "{0:25s} | {1:5d} | {2:5d} | {3:10d}".format(s["name"], s["W"], s["L"], s["score"])

if __name__ == "__main__":
  players = get_players()
  stats = {name: {"W": 0, "L": 0, "Wmsg": {}, "Lmsg": {}, "score": 0} for name in map(lambda P: P().name, players)}
  for (P1, P2) in product(players, players):
    if P1 == P2:
      continue
    for size in [7, 11, 15]:
      p1 = P1()
      p2 = P2()
      (p1_score, p2_score, msg) = play_game(p1, p2, size, size)
      stats[p1.name]["score"] = stats[p1.name]["score"] + p1_score
      stats[p2.name]["score"] = stats[p2.name]["score"] + p2_score
      if p1_score > p2_score:
        winner = p1.name
        loser = p2.name
      else:
        winner = p2.name
        loser = p1.name
      stats[winner]["W"] = stats[winner]["W"] + 1
      stats[winner]["Wmsg"][msg] = stats[winner]["Wmsg"].get(msg, 0) + 1
      stats[loser]["L"] = stats[loser]["L"] + 1
      stats[loser]["Lmsg"][msg] = stats[loser]["Lmsg"].get(msg, 0) + 1

  # Transform the dictionary into a list
  def merge_dicts(d1, d2):
    d1.update(d2)
    return d1
  stats = [merge_dicts({"name": k}, v) for (k, v) in stats.items()]
  # Let's sort stats
  stats.sort(key=lambda x: (x["W"], x["score"]), reverse=True)
  from sys import argv
  if 'pdf' in argv:
    export_results(stats)
  else:
    print_results(stats)
