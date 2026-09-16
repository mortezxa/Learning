#ifndef SCHEDULEITEM_H
#define SCHEDULEITEM_H

#include <string>
using namespace std;

class ScheduleItem {
    string day;
    int startHour, endHour;

public:
    ScheduleItem(string d, int s, int e) : day(d), startHour(s), endHour(e) {}

    bool overlapsWith(const ScheduleItem& other) const {
        return day == other.day && startHour < other.endHour && other.startHour < endHour;
    }

    string toString() const {
        return day + " " + to_string(startHour) + "-" + to_string(endHour);
    }
};

#endif
