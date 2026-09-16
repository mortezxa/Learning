#ifndef ICOURSEREPOSITORY_H
#define ICOURSEREPOSITORY_H

#include "../Model/Course.h"
#include "../Model/CourseOffering.h"
#include <string>
#include <vector>
using namespace std;

class ICourseRepository {
public:
    virtual ~ICourseRepository() = default;
    virtual void saveCourse(Course* course) = 0;
    virtual void saveOffering(CourseOffering* offering) = 0;
    virtual Course* findCourseByCode(const string& courseCode) = 0;
    virtual CourseOffering* findOfferingById(const string& offeringId) = 0;
    virtual vector<CourseOffering*> findAllOfferings() = 0;
};

#endif
