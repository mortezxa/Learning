#ifndef SEMESTER_H
#define SEMESTER_H

#include <string>
using namespace std;

class Semester {
private:
    string semesterId;
    string termName;
    bool active;

public:
    Semester(string id, string name, bool isActive) 
        : semesterId(id), termName(name), active(isActive) {}

    bool isActive() const {
        return active;
    }

    string getTermName() const {
        return termName;
    }
};

#endif
