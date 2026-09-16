#include <iostream>
#include <vector>
#include <cmath>

using namespace std;

int calculate_cost(vector<int> numbers, int x){
    if (x == numbers.size()-1){
        return 0;
    }
    if (x == numbers.size()-2){
        return (abs(numbers[numbers.size()-1]-numbers[numbers.size()-2]));
    }
    int cost_recersive1 =(abs(numbers[x] - numbers[x+1]) + calculate_cost(numbers, x+1));
    int cost_recersive2 = (abs(numbers[x] - numbers[x+2]) + calculate_cost(numbers, x+2));
    int total_cost = min(cost_recersive1,cost_recersive2);
    return total_cost;
}


vector<int> get_vec_input(){
    int n;
    cin >> n;

    vector<int> numbers(n);

    for (int i = 0; i < n; i++){
        cin >> numbers[i];
    }
    return numbers;
}
int main(){
    vector<int> numbers;
    numbers = get_vec_input();
    int cost = calculate_cost(numbers, 0);
    cout << cost << endl;
    return 0;
}
