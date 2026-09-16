#include <iostream>
#include <cmath>
#include <set>
#include <string>

using namespace std;

constexpr char ADDITION = '+';
constexpr char SUBTRACTION = '-';
constexpr char MULTIPLICATION = '*';
constexpr char DIVISION = '/';
constexpr char ZERO = '0';

set<int> evaluate_expression(const string& expression, int start, int end);

int calculate(int left, int right, char op) {
    if (op == ADDITION) return left + right;
    if (op == SUBTRACTION) return left - right;
    if (op == MULTIPLICATION) return left * right;
    if (op == DIVISION) return right != 0 ? left / right : 0;
    return 0;
}

int parse_number(const string& expression, int start, int end, int number) {
    if (start > end) return number;
    if (!isdigit(expression[start])) return -1;
    
    number = number * 10 + (expression[start] - ZERO);
    return parse_number(expression, start + 1, end, number);
}

void combine_results(set<int>& results, set<int>::iterator leftIt, set<int>::iterator leftEnd, set<int>::iterator rightIt, set<int>::iterator rightEnd, char op) {
    if (leftIt == leftEnd) return;
    
    if (rightIt == rightEnd) {
        combine_results(results, next(leftIt), leftEnd, rightIt, rightEnd, op);
        return;
    }
    
    results.insert(calculate(*leftIt, *rightIt, op));
    combine_results(results, leftIt, leftEnd, next(rightIt), rightEnd, op);}

void find_operators(const string& expression, int start, int end, set<int>& results) {
}

set<int> evaluate_expression(const string& expression, int start, int end) {
}

void do_process_input() {
    int n;
    string expression;
    
    cin >> n;
    cin >> expression;
    
    if (n > 10){
        cerr << "wrong input\n";
        exit(1);
    }
        
    set<int> results = evaluate_expression(expression, 0, expression.size()- 1);
    cout << results.size() << endl;
}


int main() {
    do_process_input();
    return 0;
}
