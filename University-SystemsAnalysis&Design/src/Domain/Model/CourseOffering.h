#ifndef COURSEOFFERING_H
#define COURSEOFFERING_H

#include "Course.h"
#include "ScheduleItem.h"
#include <algorithm>
#include <string>
#include <vector>
using namespace std;

class CourseOffering {
    string id;
    Course* course;
    int capacity;
    vector<string> enrolledStudentIds;
    vector<string> waitingStudentIds;
    ScheduleItem classSchedule;
    ScheduleItem examSchedule;

public:
    CourseOffering(string offeringId, Course* offeredCourse, int cap, ScheduleItem classTime, ScheduleItem examTime)
        : id(offeringId),
          course(offeredCourse),
          capacity(cap),
          classSchedule(classTime),
          examSchedule(examTime) {}

    string getOfferingId() const {
        return id;
    }

    Course* getCourse() const {
        return course;
    }

    int getCapacity() const {
        return capacity;
    }

    int getEnrolledCount() const {
        return static_cast<int>(enrolledStudentIds.size());
    }

    int getWaitingCount() const {
        return static_cast<int>(waitingStudentIds.size());
    }

    const ScheduleItem& getClassSchedule() const {
        return classSchedule;
    }

    const ScheduleItem& getExamSchedule() const {
        return examSchedule;
    }

    bool isFull() const {
        return getEnrolledCount() >= capacity;
    }

    bool hasStudentEnrolled(const string& studentId) const {
        return contains(enrolledStudentIds, studentId);
    }

    bool hasStudentInWaitingList(const string& studentId) const {
        return contains(waitingStudentIds, studentId);
    }

    bool conflictsWith(const CourseOffering& other) const {
        return classSchedule.overlapsWith(other.classSchedule)
            || examSchedule.overlapsWith(other.examSchedule);
    }

    bool addEnrollment(const string& studentId) {
        if (isFull() || hasStudentEnrolled(studentId)) {
            return false;
        }

        enrolledStudentIds.push_back(studentId);
        return true;
    }

    void removeEnrollment(const string& studentId) {
        removeStudent(enrolledStudentIds, studentId);
    }

    bool addToWaitingList(const string& studentId) {
        if (hasStudentEnrolled(studentId) || hasStudentInWaitingList(studentId)) {
            return false;
        }

        waitingStudentIds.push_back(studentId);
        return true;
    }

    void removeFromWaitingList(const string& studentId) {
        removeStudent(waitingStudentIds, studentId);
    }

private:
    static bool contains(const vector<string>& students, const string& studentId) {
        return find(students.begin(), students.end(), studentId) != students.end();
    }

    static void removeStudent(vector<string>& students, const string& studentId) {
        students.erase(remove(students.begin(), students.end(), studentId), students.end());
    }
};

#endif
