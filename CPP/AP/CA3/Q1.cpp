#include <iostream>
#include <cmath>
#include <string>

using namespace std;

const int SIZE = 8;
int queen_column_in_row[SIZE];


bool threat(int r1, int c1, int r2, int c2) {
    return (r1 == r2 || c1 == c2 || abs(r1 - r2) == abs(c1 - c2));
}

bool safe_to_put_at_recursive(int row, int column, int prev_row, char board[SIZE][SIZE]) {
    if (prev_row >= row)
        return true;

    if (threat(row, column, prev_row, queen_column_in_row[prev_row]) || board[row][column] == '*')
        return false;

    return safe_to_put_at_recursive(row, column, prev_row + 1, board);
}

bool safe_to_put_at(int row, int column, char board[SIZE][SIZE]) {
    return safe_to_put_at_recursive(row, column, 0, board);
}

void try_column(int row, int column, int& solution_count, char board[SIZE][SIZE]);

void solve(int row, int& solution_count, char board[SIZE][SIZE]) {
    if (row == SIZE) {
        solution_count++;
        return;
    }

    try_column(row, 0, solution_count, board);
}

void try_column(int row, int column, int& solution_count, char board[SIZE][SIZE]) {
}

void do_process_input(int queen_column_in_row[8], char board[SIZE][SIZE], string line){
    
    for (int input_line_row = 0; input_line_row < SIZE; input_line_row++) {
        cin >> line;
        for (int input_line_column = 0; input_line_column < SIZE; input_line_column++) {
            board[input_line_row][input_line_column] = line[input_line_column];
        }
    }

    for (int row = 0; row < SIZE; row++)
        queen_column_in_row[row] = -1;

    int count = 0;
    solve(0, count, board);
    cout << count << endl;

}
int main() {
    char board[SIZE][SIZE];
    string line;
    do_process_input(queen_column_in_row, board, line);
    return 0;
}
