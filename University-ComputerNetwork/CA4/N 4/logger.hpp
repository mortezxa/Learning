#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <string>

using namespace std;

class Logger {
public:
    static void info(const string& msg) {
        cout << "[INFO] " << msg << endl;
    }
    static void error(const string& msg) {
        cerr << "[ERROR] " << msg << endl;
    }
    static void success(const string& msg) {
        cout << "[SUCCESS] " << msg << endl;
    }
};

#endif
