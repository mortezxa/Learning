#ifndef DEPARTMENT_H
#define DEPARTMENT_H

#include <string>
using namespace std;

class Department {
private:
    string deptId;
    string name;

public:
    Department(string id, string n) : deptId(id), name(n) {}

    string getName() const { return name; }
    string getDeptId() const { return deptId; }
};

#endif
