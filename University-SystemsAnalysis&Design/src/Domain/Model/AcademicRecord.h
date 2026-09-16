#ifndef ACADEMICRECORD_H
#define ACADEMICRECORD_H

#include <vector>
#include <string>
using namespace std;

class AcademicRecord {
    vector<string> passedCourses;

public:
    void addPassedCourse(const string& courseCode) {
        if (!hasPassed(courseCode)) {
            passedCourses.push_back(courseCode);
        }
    }

    bool hasPassed(const string& courseCode) const {
        for (const auto& passedCourse : passedCourses) {
            if (passedCourse == courseCode) {
                return true;
            }
        }

        return false;
    }
};

#endif
