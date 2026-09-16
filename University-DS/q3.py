import ast
import itertools
import copy
import sys

def is_clear_path(board, r1, c1, r2, c2):
    if r1 == r2:
        for c in range(min(c1, c2) + 1, max(c1, c2)):
            if board[r1][c] != ".":
                return False
    elif c1 == c2:
        for r in range(min(r1, r2) + 1, max(r1, r2)):
            if board[r][c1] != ".":
                return False
    else:
        return False
    return True

def max_goals(board):
    players = [cell for row in board for cell in row if isinstance(cell, str) and cell.startswith("P")]
    best = 0
    for perm in itertools.permutations(players):
        b = copy.deepcopy(board)
        cnt = 0
        for p in perm:
            pos = [(i, j) for i in range(4) for j in range(4) if b[i][j] == p]
            if not pos:
                continue
            pr, pc = pos[0]
            g = "G" + p[1:]
            goals = [(i, j) for i in range(4) for j in range(4) if b[i][j] == g]
            if not goals:
                continue
            gr, gc = goals[0]
            if is_clear_path(b, pr, pc, gr, gc):
                cnt += 1
                b[pr][pc] = "."
        best = max(best, cnt)
    return best

if __name__ == "__main__":
    data = sys.stdin.read()
    board = ast.literal_eval(data)
    print(max_goals(board), end="")
