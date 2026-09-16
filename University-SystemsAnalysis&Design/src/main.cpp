#include "Data/InMemoryCourseRepository.h"
#include "Data/InMemoryStudentRepository.h"
#include "Presentation/RegistrationController.h"
#include <iostream>
#include <vector>
using namespace std;

void printResult(const string& title, const OperationResultView& result) {
    cout << title << ": " << (result.success ? "OK" : "FAILED")
              << " - " << result.message << '\n';
}

void printOfferings(RegistrationController& controller) {
    cout << "\nAvailable course offerings:\n";
    for (const auto& offering : controller.viewAvailableOfferings()) {
        cout << "- " << offering.offeringId
                  << " | " << offering.courseCode
                  << " " << offering.courseTitle
                  << " | Class: " << offering.classTime
                  << " | Exam: " << offering.examTime
                  << " | Capacity: " << offering.enrolledCount << "/" << offering.capacity
                  << " | Waiting: " << offering.waitingCount
                  << '\n';
    }
}

int main() {
    InMemoryCourseRepository courseRepository;
    InMemoryStudentRepository studentRepository;

    Course math("MATH101", "General Mathematics");
    Course programming("CS101", "Advanced Programming");
    Course algorithms("CS201", "Algorithm Design", {"CS101"});
    Course database("CS301", "Database Systems", {"CS101"});

    courseRepository.saveCourse(&math);
    courseRepository.saveCourse(&programming);
    courseRepository.saveCourse(&algorithms);
    courseRepository.saveCourse(&database);

    CourseOffering mathGroup1("MATH101-01", &math, 2, ScheduleItem("Saturday", 8, 10), ScheduleItem("Monday", 8, 10));
    CourseOffering mathGroup2("MATH101-02", &math, 2, ScheduleItem("Sunday", 10, 12), ScheduleItem("Tuesday", 8, 10));
    CourseOffering programmingGroup("CS101-01", &programming, 1, ScheduleItem("Saturday", 10, 12), ScheduleItem("Wednesday", 8, 10));
    CourseOffering algorithmsGroup("CS201-01", &algorithms, 2, ScheduleItem("Sunday", 10, 12), ScheduleItem("Thursday", 8, 10));
    CourseOffering databaseGroup("CS301-01", &database, 2, ScheduleItem("Monday", 12, 14), ScheduleItem("Thursday", 8, 10));

    courseRepository.saveOffering(&mathGroup1);
    courseRepository.saveOffering(&mathGroup2);
    courseRepository.saveOffering(&programmingGroup);
    courseRepository.saveOffering(&algorithmsGroup);
    courseRepository.saveOffering(&databaseGroup);

    Student ali("S1", "Ali Rezaei");
    ali.addPassedCourse("CS101");
    Student zahra("S2", "Zahra Ahmadi");
    zahra.addPassedCourse("MATH101");
    Student morteza("S3", "Morteza MohammadiMotlagh");

    studentRepository.save(&ali);
    studentRepository.save(&zahra);
    studentRepository.save(&morteza);

    RegistrationController controller(studentRepository, courseRepository);

    printOfferings(controller);

    cout << "\nSample registration scenario:\n";
    printResult("Successful math enrollment for Ali", controller.enrollCourse("S1", "MATH101-01"));
    printResult("Prevent taking two offerings of one course", controller.enrollCourse("S1", "MATH101-02"));
    printResult("Prevent taking a passed course", controller.enrollCourse("S1", "CS101-01"));
    printResult("Prevent missing prerequisite", controller.enrollCourse("S2", "CS201-01"));
    printResult("Successful algorithm enrollment for Ali", controller.enrollCourse("S1", "CS201-01"));
    printResult("Prevent exam time conflict", controller.enrollCourse("S1", "CS301-01"));
    printResult("Fill programming capacity by Zahra", controller.enrollCourse("S2", "CS101-01"));
    printResult("Morteza joins programming waiting list", controller.joinWaitingList("S3", "CS101-01"));
    printResult("Morteza leaves programming waiting list", controller.leaveWaitingList("S3", "CS101-01"));
    printResult("Ali drops math offering", controller.dropCourse("S1", "MATH101-01"));

    printOfferings(controller);

    return 0;
}
