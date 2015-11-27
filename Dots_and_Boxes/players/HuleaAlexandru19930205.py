# Alexandru Hulea 341 C4

from random import choice
from itertools import product, izip, count
from Queue import Queue

class HuleaAlexandru19930205:
	def __init__(self):
		self.name = "Hulea Alexandru"
		self.earlyGame = [2, self.evaluateEarlyGame, True]
		self.lateGame = [2, self.evaluateLateGame, False]

	def getNeighboursFromBoard(self, row, col, height, width):
		if (row % 2 == 0):
			if (row == 0):
				return [(0, col)]
			elif (row == 2 * height):
				return [(height - 1, col)]
			else:
				return [(row / 2 - 1, col), (row / 2, col)]	
		else:
			if (col == 0):
				return [((row - 1) / 2, col)]
			elif (col == width):
				return [((row - 1) / 2, col - 1)]
			else:
				return [((row - 1) / 2, col - 1), ((row - 1) / 2, col)]

	def closeCells(self, row, col, board, matrix, sides, list3):
		board[row][col] = 1
		height = len(matrix)
		width = len(matrix[0])
		neighbours = self.getNeighboursFromBoard(row, col, height, width)
		closedCells = 0
		for neighbour in neighbours:
			hisRow = neighbour[0]
			hisCol = neighbour[1]
			sides[matrix[hisRow][hisCol]] -= 1
			matrix[hisRow][hisCol] += 1
			sides[matrix[hisRow][hisCol]] += 1
			if (matrix[hisRow][hisCol] == 4):
				closedCells += 1
				list3.remove((hisRow, hisCol))
			elif (matrix[hisRow][hisCol] == 3):
				list3.append((hisRow, hisCol))
		return closedCells

	def openCells(self, row, col, board, matrix, sides, list3):
		board[row][col] = 0
		height = len(matrix)
		width = len(matrix[0])
		neighbours = self.getNeighboursFromBoard(row, col, height, width)
		for neighbour in neighbours:
			hisRow = neighbour[0]
			hisCol = neighbour[1]
			sides[matrix[hisRow][hisCol]] -= 1
			matrix[hisRow][hisCol] -= 1
			sides[matrix[hisRow][hisCol]] += 1
			if (matrix[hisRow][hisCol] == 2):
				list3.remove((hisRow, hisCol))
			elif (matrix[hisRow][hisCol] == 3):
				list3.append((hisRow, hisCol))

	def is_final(self, score, matrix):
		height = len(matrix)
		width = len(matrix[0])
		total_score = width * height
		if (total_score == score[0] + score[1]):
			return True
		return False

	def evaluateEarlyGame(self, board, score, matrix, sides, list3):
		return sides[3] + score[0] - score[1]

	def getNeighboursFromMatrix(self, row, col, matrix):
		height = len(matrix)
		width = len(matrix[0])
		up = (-1, 0)
		down = (1, 0)
		right = (0, 1)
		left = (0, -1)
		cell = (row, col)

		if (row == 0):
			if (col == 0):
				return [tuple([it1 + it2 for it1, it2 in zip(cell, right)]), \
				tuple([it1 + it2 for it1, it2 in zip(cell, down)])]
			elif (col == width - 1):
				return [tuple([it1 + it2 for it1, it2 in zip(cell, left)]), \
				tuple([it1 + it2 for it1, it2 in zip(cell, down)])]
			else:
				return [tuple([it1 + it2 for it1, it2 in zip(cell, left)]), \
				tuple([it1 + it2 for it1, it2 in zip(cell, right)]), \
				tuple([it1 + it2 for it1, it2 in zip(cell, down)])]
		elif (row == height - 1):
			if (col == 0):
				return [tuple([it1 + it2 for it1, it2 in zip(cell, up)]), \
				tuple([it1 + it2 for it1, it2 in zip(cell, right)])]
			elif (col == width - 1):
				return [tuple([it1 + it2 for it1, it2 in zip(cell, up)]), \
				tuple([it1 + it2 for it1, it2 in zip(cell, left)])]
			else:
				return [tuple([it1 + it2 for it1, it2 in zip(cell, left)]), \
				tuple([it1 + it2 for it1, it2 in zip(cell, right)]), \
				tuple([it1 + it2 for it1, it2 in zip(cell, up)])]
		else:
			if (col == 0):
				return [tuple([it1 + it2 for it1, it2 in zip(cell, up)]), \
				tuple([it1 + it2 for it1, it2 in zip(cell, right)]), \
				tuple([it1 + it2 for it1, it2 in zip(cell, down)])]
			elif (col == width - 1):
				return [tuple([it1 + it2 for it1, it2 in zip(cell, up)]), \
				tuple([it1 + it2 for it1, it2 in zip(cell, left)]), \
				tuple([it1 + it2 for it1, it2 in zip(cell, down)])]
			else:
				return [tuple([it1 + it2 for it1, it2 in zip(cell, left)]), \
				tuple([it1 + it2 for it1, it2 in zip(cell, right)]), \
				tuple([it1 + it2 for it1, it2 in zip(cell, up)]), \
				tuple([it1 + it2 for it1, it2 in zip(cell, down)])]

	def getCommonEdge(self, row, col, hisRow, hisCol, board):
		if (row == hisRow):
			if (col > hisCol):
				return board[2 * row + 1][col]
			elif (col < hisCol):
				return board[2 * row + 1][hisCol]
		elif (row > hisRow):
			return board[2 * row][col]
		elif (row < hisRow):
			return board[2 * hisRow][col]

	def getChainNeighbours(self, row, col, matrix, visited, board):
		neighbours = self.getNeighboursFromMatrix(row, col, matrix)
		chainNeighbours = []
		for neighbour in neighbours:
			hisRow = neighbour[0]
			hisCol = neighbour[1]
			if (visited[hisRow][hisCol] == 1):
				continue
			if (self.getCommonEdge(row, col, hisRow, hisCol, board) == 1):
				continue
			if (matrix[hisRow][hisCol] == 2):
				chainNeighbours.append((hisRow, hisCol))
		return chainNeighbours

	def getChain(self, row, col, matrix, board):
		height = len(matrix)
		width = len(matrix[0])
		visited = [[0 for column in range(width)] for rows in range(height)]
		visited[row][col] = 1
		length = 0
		q = Queue()
		q.put((row, col))
		row1 = row
		col1 = col
		while not q.empty():
			(row, col) = q.get()
			neighbours = self.getChainNeighbours(row, col, matrix, visited, board)
			length += len(neighbours)
			if (length >= 2):
				length = 2
				break
			for neighbour in neighbours:
				row = neighbour[0]
				col = neighbour[1]
				visited[row][col] = 1
				q.put(neighbour)
		return length

	def evaluateLateGame(self, board, score, matrix, sides, list3):
		height = len(matrix)
		width = len(matrix[0])

		if (len(list3) > 2):
			return 3 * len(list3) + sides[3] + score[0] - score[1]
		if (len(list3) == 2):
			length1 = self.getChain(list3[0][0], list3[0][1], matrix, board)
			length2 = self.getChain(list3[1][0], list3[1][1], matrix, board)
			length = length1 + length2
			if (length1 == 0 and length2 == 0):
				length = -(height * width)
			return length + sides[3] + score[0] - score[1]		
		elif (len(list3) == 1):
			length = self.getChain(list3[0][0], list3[0][1], matrix, board)
			return length + sides[3] + score[0] - score[1]
		else:
			length = -(height * width)
			if (sides[4] >= height * width - 4):
				length = -length
			return length + score[0] - score[1]

	def alphaBetaMin(self, alpha, beta, board, score, matrix, sides, list3, gameState):
		if (gameState[0] == 0):
			return (-apply(gameState[1], [board, score, matrix, sides, list3]), -1, -1) 

		if (self.is_final(score, matrix)):
			return (score[1] - score[0], -1, -1)

		moves = []
		board_height = len(board)
		for i in range(board_height):
			board_width = len(board[i])
			for j in range(board_width):
				if (board[i][j] == 0):
					moves.append((i, j))  

		bestRow = -1
		bestCol = -1

		for move in moves:
			row = move[0]
			col = move[1]

			closedCells = self.closeCells(row, col, board, matrix, sides, list3)
		
			result = 0
			if (closedCells == 0):	
				result = self.alphaBetaMax(alpha, beta, board, (score[1], score[0]), matrix, \
				sides, list3, [gameState[0] - 1, gameState[1], gameState[2]])
			else:
				result = self.alphaBetaMin(alpha, beta, board, (score[0] + closedCells, score[1]), \
				matrix, sides, list3, [gameState[0] - 1, gameState[1], gameState[2]])
			
			eval_score = result[0]

			self.openCells(row, col, board, matrix, sides, list3)

			if(eval_score <= alpha):
				return (alpha, bestRow, bestCol)
			if(eval_score < beta):
				bestRow = row
				bestCol = col
				beta = eval_score

		return (beta, bestRow, bestCol)

	def alphaBetaMax(self, alpha, beta, board, score, matrix, sides, list3, gameState):
		if (gameState[0] == 0):
			return (apply(gameState[1], [board, score, matrix, sides, list3]), -1, -1) 

		if (self.is_final(score, matrix)):
			return (score[0] - score[1], -1, -1)

		moves = []
		board_height = len(board)
		for i in range(board_height):
			board_width = len(board[i])
			for j in range(board_width):
				if (board[i][j] == 0):
					moves.append((i, j))  

		bestRow = -1
		bestCol = -1

		for move in moves:
			row = move[0]
			col = move[1]

			closedCells = self.closeCells(row, col, board, matrix, sides, list3)
		
			result = 0
			if (closedCells == 0):	
				result = self.alphaBetaMin(alpha, beta, board, (score[1], score[0]), matrix, sides, \
				list3, [gameState[0] - 1, gameState[1], gameState[2]])
			else:
				result = self.alphaBetaMax(alpha, beta, board, (score[0] + closedCells, score[1]), \
				matrix, sides, list3, [gameState[0] - 1, gameState[1], gameState[2]])
			
			eval_score = result[0]
			
			self.openCells(row, col, board, matrix, sides, list3)
			
			if(eval_score >= beta):
				return (beta, bestRow, bestCol)
			if(eval_score > alpha):
				bestRow = row
				bestCol = col
				alpha = eval_score

		return (alpha, bestRow, bestCol)

	def move(self, board, score):
		matrix = []
		height = (len(board) - 1) / 2
		width = len(board[0])
		sides = []
		list3 = []
		for i in range(5):
			sides.append(0)

		for i in range(height):
			row = []
			for j in range(width):
				up = board[2 * i][j]
				left = board[2 * i + 1][j]
				right = board[2 * i + 1][j + 1]
				down = board[2 * i + 2][j]
				value = up + left + right + down
				sides[value] += 1
				if (value == 3):
					list3.append((i, j))
				row.append(value)
			matrix.append(row)

		sum = sides[0] + sides[1] + sides[2]
		if (sides[2] * 5 / 4 >= sum and sides[3] > 0):
			gameState = self.lateGame
		else:
			gameState = self.earlyGame
		
		height = (len(board) - 1) / 2
		width = len(board[0])
		alpha = -4 * (width * height + 1)
		beta = 4 * (width * height + 1)

		result = self.alphaBetaMax(alpha, beta, board, score, matrix, sides, list3, gameState)
		return (result[1], result[2])