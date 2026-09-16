#ifndef STUDENT_H
#define STUDENT_H

#include <vector>
#include <string>
#include <algorithm>
#include "Enrollment.h"
#include "AcademicRecord.h"
using namespace std;

class Student {
private:
    string studentId;
    string name;
    vector<Enrollment> enrollments;
    AcademicRecord record;

public:
    Student(string id, string n) : studentId(id), name(n) {}

    string getStudentId() const {
        return studentId;
    }

    string getName() const {
        return name;
    }

    void addPassedCourse(const string& courseCode) {
        record.addPassedCourse(courseCode);
    }

    bool hasPassed(Course* course) const {
        return record.hasPassed(course->getCourseCode());
    }

    const vector<Enrollment>& getEnrollments() const {
        return enrollments;
    }

    string enroll(CourseOffering* offering) {
        string validationError = validateCanTake(offering);
        if (!validationError.empty()) {
            return validationError;
        }

        if (offering->isFull()) {
            return "Offering capacity is full";
        }

        if (!offering->addEnrollment(studentId)) {
            return "Student is already enrolled in this offering";
        }

        enrollments.push_back(Enrollment(offering));
        return "";
    }

    string drop(CourseOffering* offering) {
        auto enrollmentIt = find_if(enrollments.begin(), enrollments.end(), [offering](const Enrollment& enrollment) {
            return enrollment.getOffering() == offering;
        });

        if (enrollmentIt == enrollments.end()) {
            return "This offering is not in the student's enrollments";
        }

        offering->removeEnrollment(studentId);
        enrollments.erase(enrollmentIt);
        return "";
    }

    string joinWaitingList(CourseOffering* offering) {
        string validationError = validateCanTake(offering);
        if (!validationError.empty()) {
            return validationError;
        }

        if (!offering->addToWaitingList(studentId)) {
            return "Cannot join the waiting list";
        }

        return "";
    }

    string leaveWaitingList(CourseOffering* offering) {
        if (!offering->hasStudentInWaitingList(studentId)) {
            return "Student is not in this offering's waiting list";
        }

        offering->removeFromWaitingList(studentId);
        return "";
    }

private:
    string validateCanTake(CourseOffering* offering) const {
        Course* course = offering->getCourse();

        if (hasPassed(course)) {
            return "Student has already passed this course";
        }

        if (isAlreadyTakingCourse(course->getCourseCode())) {
            return "Taking two offerings of the same course is not allowed";
        }

        for (const auto& prerequisiteCode : course->getPrerequisiteCourseCodes()) {
            if (!record.hasPassed(prerequisiteCode)) {
                return "Missing prerequisite: " + prerequisiteCode;
            }
        }

        for (const auto& enrollment : enrollments) {
            if (offering->conflictsWith(*enrollment.getOffering())) {
                return "Class or exam time conflicts with selected courses";
            }
        }

        return "";
    }

    bool isAlreadyTakingCourse(const string& courseCode) const {
        for (const auto& enrollment : enrollments) {
            if (enrollment.getOffering()->getCourse()->getCourseCode() == courseCode) {
                return true;
            }
        }

        return false;
    }
};

#endif
