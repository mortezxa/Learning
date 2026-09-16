#ifndef INSTRUCTOR_H
#define INSTRUCTOR_H

#include <string>
#include "Department.h"
using namespace std;

class Instructor {
private:
    string instructorId;
    string name;
    Department* department;

public:
    Instructor(string id, string n, Department* dept) 
        : instructorId(id), name(n), department(dept) {}

    string getName() const { return name; }
};

#endif
