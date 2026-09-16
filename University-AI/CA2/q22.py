def minimax(self, board, depth, is_maximizing_player):
    winner = self.check_winner(board)
    if winner == 1:
        return 1  # Player 1 wins
    elif winner == -1:
        return -1  # Player 2 (Computer) wins
    elif winner == 0:
        return 0  # Tie

    if depth == 0:
        return 0  # Depth limit reached

    moves = self.get_possible_moves(board, -1 if is_maximizing_player else 1)
    best_value = -inf if is_maximizing_player else inf

    for move in moves:
        new_board = self.apply_move(board, move, -1 if is_maximizing_player else 1)
        if new_board is not None:
            value = self.minimax(new_board, depth - 1, not is_maximizing_player)
            if is_maximizing_player:
                best_value = max(best_value, value)
            else:
                best_value = min(best_value, value)
    return best_value



def minimax_with_alpha_beta(self, board, depth, alpha, beta, is_maximizing_player):
    winner = self.check_winner(board)
    if winner == 1:
        return 1  # Player 1 wins
    elif winner == -1:
        return -1  # Player 2 (Computer) wins
    elif winner == 0:
        return 0  # Tie

    if depth == 0:
        return 0  # Depth limit reached

    moves = self.get_possible_moves(board, -1 if is_maximizing_player else 1)
    best_value = -inf if is_maximizing_player else inf

    for move in moves:
        new_board = self.apply_move(board, move, -1 if is_maximizing_player else 1)
        if new_board is not None:
            value = self.minimax_with_alpha_beta(new_board, depth - 1, alpha, beta, not is_maximizing_player)
            if is_maximizing_player:
                best_value = max(best_value, value)
                alpha = max(alpha, best_value)
            else:
                best_value = min(best_value, value)
                beta = min(beta, best_value)

            if beta <= alpha:
                break  # Prune the branch

    return best_value