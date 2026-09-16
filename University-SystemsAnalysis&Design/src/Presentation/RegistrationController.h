#ifndef REGISTRATIONCONTROLLER_H
#define REGISTRATIONCONTROLLER_H

#include "../Domain/Repository/IStudentRepository.h"
#include "../Domain/Repository/ICourseRepository.h"
#include <string>
#include <vector>
using namespace std;

struct OperationResultView {
    bool success;
    string message;
};

struct CourseOfferingView {
    string offeringId;
    string courseCode;
    string courseTitle;
    string classTime;
    string examTime;
    int capacity;
    int enrolledCount;
    int waitingCount;
};

class RegistrationController {
    IStudentRepository& sRepo;
    ICourseRepository& cRepo;

public:
    RegistrationController(IStudentRepository& sr, ICourseRepository& cr) : sRepo(sr), cRepo(cr) {}

    vector<CourseOfferingView> viewAvailableOfferings() {
        vector<CourseOfferingView> views;
        for (auto* offering : cRepo.findAllOfferings()) {
            views.push_back(toView(offering));
        }

        return views;
    }

    OperationResultView enrollCourse(const string& studentId, const string& offeringId) {
        auto lookup = findStudentAndOffering(studentId, offeringId);
        if (!lookup.success) {
            return lookup.result;
        }

        string error = lookup.student->enroll(lookup.offering);
        return makeResult(error.empty(), error.empty() ? "Offering enrolled successfully" : error);
    }

    OperationResultView dropCourse(const string& studentId, const string& offeringId) {
        auto lookup = findStudentAndOffering(studentId, offeringId);
        if (!lookup.success) {
            return lookup.result;
        }

        string error = lookup.student->drop(lookup.offering);
        return makeResult(error.empty(), error.empty() ? "Offering dropped successfully" : error);
    }

    OperationResultView joinWaitingList(const string& studentId, const string& offeringId) {
        auto lookup = findStudentAndOffering(studentId, offeringId);
        if (!lookup.success) {
            return lookup.result;
        }

        string error = lookup.student->joinWaitingList(lookup.offering);
        return makeResult(error.empty(), error.empty() ? "Student joined the waiting list" : error);
    }

    OperationResultView leaveWaitingList(const string& studentId, const string& offeringId) {
        auto lookup = findStudentAndOffering(studentId, offeringId);
        if (!lookup.success) {
            return lookup.result;
        }

        string error = lookup.student->leaveWaitingList(lookup.offering);
        return makeResult(error.empty(), error.empty() ? "Student left the waiting list" : error);
    }

private:
    struct LookupResult {
        bool success;
        Student* student;
        CourseOffering* offering;
        OperationResultView result;
    };

    LookupResult findStudentAndOffering(const string& studentId, const string& offeringId) {
        Student* student = sRepo.findById(studentId);
        if (student == nullptr) {
            return {false, nullptr, nullptr, makeResult(false, "Student not found")};
        }

        CourseOffering* offering = cRepo.findOfferingById(offeringId);
        if (offering == nullptr) {
            return {false, nullptr, nullptr, makeResult(false, "Course offering not found")};
        }

        return {true, student, offering, makeResult(true, "")};
    }

    static CourseOfferingView toView(CourseOffering* offering) {
        return {
            offering->getOfferingId(),
            offering->getCourse()->getCourseCode(),
            offering->getCourse()->getTitle(),
            offering->getClassSchedule().toString(),
            offering->getExamSchedule().toString(),
            offering->getCapacity(),
            offering->getEnrolledCount(),
            offering->getWaitingCount()
        };
    }

    static OperationResultView makeResult(bool success, const string& message) {
        return {success, message};
    }
};

#endif
