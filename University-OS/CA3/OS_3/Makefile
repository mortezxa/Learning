CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread

# فایل‌های فاز ۱
PHASE1_SRCS = phase1/race_condition.cpp phase1/deadlock.cpp

# فایل‌های فاز ۲
PHASE2_SRCS = phase2/scheduler.cpp phase2/scenarios.cpp

# همه فایل‌ها
SRCS = main.cpp $(PHASE1_SRCS) $(PHASE2_SRCS)

TARGET = os_project

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)
