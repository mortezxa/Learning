#ifndef ISTUDENTREPOSITORY_H
#define ISTUDENTREPOSITORY_H

#include "../Model/Student.h"
#include <string>
using namespace std;

class IStudentRepository {
public:
    virtual ~IStudentRepository() = default;
    virtual void save(Student* s) = 0;
    virtual Student* findById(const string& id) = 0;
};

#endif
