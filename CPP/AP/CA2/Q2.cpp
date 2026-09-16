#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>

using namespace std;

#define NORMAL "normal"
#define COVERED "covered"
#define CCTV "CCTV"
#define R_SPOT "request_spot"
#define A_SPOT "assign_spot"
#define PASS_TIME "pass_time"
#define CHECKOUT "checkout"
#define VP vector<parking_id>
#define VN vector<normal_cost>
#define VC vector<Cars>
#define V vector<check_days>



struct check_days
{
    int id;
    bool is_full;
    int day_counter;
};

struct normal_cost
{
    int size;
    int static_price;
    int price_per_day;
};

struct parking_id
{
    int id;
    int size;
    string type;
};

struct Cars
{
    string name;
    int size;

};


int calculate_total_cost_normal(int id, VP park_id, VN normal_c, VC car_data, V& check_day){
    for(auto& item : check_day){
        for(auto& item2 : normal_c){    
            for(auto& item3 : park_id){
                
                if(item.id == id && item3.id == id){
                    if(item3.size == item2.size){
                        int cost = (item2.static_price + (item2.price_per_day*item.day_counter));
                        return cost;
                    }
                }
            }
        }
    }
    return 0;
}

int calculate_total_cost_covered(int id, VP park_id, VN normal_c, VC car_data, V& check_day){
    for(auto& item : check_day){
        for(auto& item2 : normal_c){
            for(auto& item3 : park_id){
                
                if(item.id == id && item3.id == id){
                    if(item3.size == item2.size){
                        int cost = (item2.static_price + 50 + ((item2.price_per_day + 30)*(item.day_counter)));
                        return cost;
                    }
                }
            }
        }
    }
    return 0;
}

int calculate_total_cost_CCTV(int id, VP park_id, VN normal_c, VC car_data, V& check_day){
    for(auto& item : check_day){
        for(auto& item2 : normal_c){
            for(auto& item3 : park_id){
                
                if(item.id == id && item3.id == id){
                    if(item3.size == item2.size){
                        int cost = (item2.static_price + 80 + ((item2.price_per_day + 60)*(item.day_counter)));
                        return cost;
                    }
                }
            }
        }
    }
    return 0;
}

int process_calculate_total_cost(int id, VP park_id, VN normal_c, VC car_data, V& check_day){
    int total_cost;
    for(auto& item : park_id){
        if(item.id == id){
            if(item.type == NORMAL){
                total_cost = calculate_total_cost_normal(id, park_id , normal_c, car_data, check_day);
                return total_cost;
            }    
            else if(item.type == COVERED){
                total_cost = calculate_total_cost_covered(id, park_id , normal_c, car_data, check_day);
                return total_cost;
            }
            else if(item.type == CCTV){
                total_cost = calculate_total_cost_CCTV(id, park_id , normal_c, car_data, check_day);
                return total_cost;
            }
        }
    }
    return 0;
}

void process_request(parking_id item,normal_cost item2){
    if(item.type == NORMAL){
        cout << item.id << ": " << item.type  << " " << item2.static_price<< " " << item2.price_per_day <<"\n";
    }
    else if(item.type == COVERED){
        cout << item.id << ": " << item.type  << " " << (item2.static_price)+50 << " " << (item2.price_per_day)+30 <<"\n";
    }
    else if(item.type == CCTV){
        cout << item.id << ": " << item.type  << " " << (item2.static_price)+80 << " " << (item2.price_per_day)+60 <<"\n";
    }
}

void do_request_spot(const string& rest_of_line, VP park_id, VN normal_c, VC car_data, V& check_day){
    int car_size= 0;
    for(auto& item : car_data){
        if(item.name == rest_of_line){
            car_size = item.size;
        }
    }
    for(auto& item: park_id){
        for(auto& item2 : normal_c)
            if(item.size == car_size){
                if(item2.size == car_size){
                    process_request(item, item2);
                }
        }
    }
}

void do_assign_spot(const string& rest_of_line, VP park_id, VN normal_c, VC car_data, V& check_day){
    int parkId = atoi(rest_of_line.c_str());
    for(auto& item : check_day){
        if(item.id == parkId){
            item.is_full = 1;
            item.day_counter = 0;
        }
    }
    cout << "Spot " << parkId << " is occupied now.\n";
}

void do_checkout(const string& rest_of_line, VP park_id, VN normal_c, VC car_data, V& check_day){
    int check_id = atoi(rest_of_line.c_str());
    for(auto& item : check_day){
        if(item.id == check_id){
            item.is_full= 0;
        }
    }
    cout << "Spot " << check_id << " is free now.\n";
    cout << "Total cost: " << process_calculate_total_cost(check_id, park_id, normal_c, car_data, check_day) << "\n" ;
}

void do_pass_time(const string& rest_of_line , VP park_id, VN normal_c, VC car_data, V& check_day){
    int day_for_pass = atoi(rest_of_line.c_str());
    for(auto& item : check_day){
        if(item.is_full){
            item.day_counter += day_for_pass;
        }
    }
}

void process_inputs(const string& input, VP park_id, VN normal_c, VC car_data, V& check_day){
    stringstream ss(input);
    string first_word, rest_of_line;

    ss >> first_word;
        
    getline(ss, rest_of_line);
    if(rest_of_line[0] == ' ')
        rest_of_line.erase(0, 1);
    if(first_word == R_SPOT){
        do_request_spot(rest_of_line, park_id, normal_c, car_data, check_day);
    }
    else if(first_word == A_SPOT){
        do_assign_spot(rest_of_line, park_id, normal_c, car_data, check_day);
    }
    else if(first_word == CHECKOUT){
        do_checkout(rest_of_line, park_id, normal_c, car_data, check_day);
    }
    else if(first_word == PASS_TIME){
        do_pass_time(rest_of_line, park_id, normal_c,car_data, check_day);
    }
    else
        cout << "wrong inputs\n";
}   

VC readCSV1(const std::string& filename){
    VC car_list;
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return car_list;
    }

    string line;

    getline(file, line);

    while (getline(file, line)) {
        stringstream ss(line);
        string name;
        string sizeStr;
        int size;

        getline(ss, name, ',');

        getline(ss, sizeStr, ',');
        size = std::stoi(sizeStr);

        car_list.push_back({name, size});
    }

    file.close();
    return car_list;
}

VP readCSV2(const std::string& filename){
    VP data_list;
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << std::endl;
        return data_list;
    }

    string line;
    getline(file, line);

    while (getline(file, line)) {
        stringstream ss(line);
        string idStr, sizeStr, type;
        int id, size;
        int day_counter;
        bool is_full;
        getline(ss, idStr, ',');
        id = std::stoi(idStr);

        getline(ss, sizeStr, ',');
        size = std::stoi(sizeStr);

        getline(ss, type, ',');

        data_list.push_back({id, size, type});
    }

    file.close();
    return data_list;
}

VN readCSV3(const std::string& filename){
    VN cost_list;
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << std::endl;
        return cost_list;
    }

    string line;

    getline(file, line);
    
    while (getline(file, line)) {
        stringstream ss(line);
        string sizeStr, staticStr, priceStr;
        int size, isStatic, price;

        getline(ss, sizeStr, ',');
        size = std::stoi(sizeStr);

        getline(ss, staticStr, ',');
        isStatic = std::stoi(staticStr);
        
        std::getline(ss, priceStr, ',');
        price = std::stoi(priceStr);
        
        cost_list.push_back({size, isStatic, price});
    }

    file.close();
    return cost_list;
}

void fill_check_day(VP park_id, V& check_day){
    for(auto& item: park_id){
        check_days new_item;
        new_item.id = item.id;
        new_item.is_full = 0;
        check_day.push_back(new_item);
    }
}

void sort_by_id(VP& park_id){
    sort(park_id.begin(), park_id.end(), [](const parking_id& a, const parking_id& b){
        return (a.id < b.id);
    });
}


int main(int argc,char* argv[]){
    if (argc < 4){
        cerr << "error\n";
        return 1;
    }
    string FILE1 = argv[1];
    string FILE2 = argv[2];
    string FILE3 = argv[3];
    
    V check_day;
    string input;
    VC car_data = readCSV1(FILE1);
    VP park_id = readCSV2(FILE2);
    VN normal_c = readCSV3(FILE3);
    
    sort_by_id(park_id);
    fill_check_day(park_id, check_day);

    while(getline(cin, input)){
        process_inputs(input, park_id, normal_c, car_data, check_day);   
    }
    return 0;
}
