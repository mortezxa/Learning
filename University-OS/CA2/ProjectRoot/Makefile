CXX      = g++
CXXFLAGS = -Wall -Wextra -std=c++17

TARGET = pipeline

SRCS = main.cpp \
       include/stage1.cpp \
       include/stage2.cpp \
       include/stage3.cpp

OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
	rm -f /tmp/fifo_s1_s2 /tmp/fifo_s3_s2

.PHONY: all clean
