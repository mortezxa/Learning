#ifndef INMEMORYCOURSEREPOSITORY_H
#define INMEMORYCOURSEREPOSITORY_H

#include "../Domain/Repository/ICourseRepository.h"
#include <map>
using namespace std;

class InMemoryCourseRepository : public ICourseRepository {
    map<string, Course*> courses;
    map<string, CourseOffering*> offerings;

public:
    void saveCourse(Course* course) override {
        courses[course->getCourseCode()] = course;
    }

    void saveOffering(CourseOffering* offering) override {
        offerings[offering->getOfferingId()] = offering;
    }

    Course* findCourseByCode(const string& courseCode) override {
        return courses.count(courseCode) ? courses[courseCode] : nullptr;
    }

    CourseOffering* findOfferingById(const string& offeringId) override {
        return offerings.count(offeringId) ? offerings[offeringId] : nullptr;
    }

    vector<CourseOffering*> findAllOfferings() override {
        vector<CourseOffering*> result;
        for (const auto& item : offerings) {
            result.push_back(item.second);
        }

        return result;
    }
};

#endif
