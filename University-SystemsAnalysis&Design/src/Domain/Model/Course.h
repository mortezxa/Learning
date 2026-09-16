#ifndef COURSE_H
#define COURSE_H

#include <string>
#include <vector>
using namespace std;

class Course {
    string code;
    string title;
    vector<string> prerequisiteCourseCodes;

public:
    Course(string courseCode, string courseTitle, vector<string> prerequisites = {})
        : code(courseCode), title(courseTitle), prerequisiteCourseCodes(prerequisites) {}

    string getCourseCode() const {
        return code;
    }

    string getTitle() const {
        return title;
    }

    const vector<string>& getPrerequisiteCourseCodes() const {
        return prerequisiteCourseCodes;
    }
};

#endif
