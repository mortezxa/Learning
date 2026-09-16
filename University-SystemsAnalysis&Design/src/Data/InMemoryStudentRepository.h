#ifndef INMEMORYSTUDENTREPOSITORY_H
#define INMEMORYSTUDENTREPOSITORY_H

#include "../Domain/Repository/IStudentRepository.h"
#include <map>
using namespace std;

class InMemoryStudentRepository : public IStudentRepository {
    map<string, Student*> db;

public:
    void save(Student* student) override {
        db[student->getStudentId()] = student;
    }

    Student* findById(const string& id) override {
        return db.count(id) ? db[id] : nullptr;
    }
};

#endif
