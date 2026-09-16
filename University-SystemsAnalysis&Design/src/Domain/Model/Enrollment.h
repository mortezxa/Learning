#ifndef ENROLLMENT_H
#define ENROLLMENT_H

#include "CourseOffering.h"
using namespace std;

class Enrollment {
    CourseOffering* offering;

public:
    Enrollment(CourseOffering* o) : offering(o) {}

    CourseOffering* getOffering() const {
        return offering;
    }
};

#endif
