SERVER_EXEC = server
CLIENT_EXEC = client

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra

SERVER_SOURCES = server.cpp
CLIENT_SOURCES = client.cpp

SERVER_OBJECTS = $(SERVER_SOURCES:.cpp=.o)
CLIENT_OBJECTS = $(CLIENT_SOURCES:.cpp=.o)

all: $(SERVER_EXEC) $(CLIENT_EXEC)

$(SERVER_EXEC): $(SERVER_OBJECTS)
	@echo "Linking $(SERVER_EXEC)..."
	$(CXX) $(CXXFLAGS) $^ -o $@

$(CLIENT_EXEC): $(CLIENT_OBJECTS)
	@echo "Linking $(CLIENT_EXEC)..."
	$(CXX) $(CXXFLAGS) $^ -o $@

%.o: %.cpp
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "Cleaning up..."
	rm -f $(SERVER_EXEC) $(CLIENT_EXEC) $(SERVER_OBJECTS) $(CLIENT_OBJECTS)

rebuild: clean all

help:
	@echo "Makefile Usage:"
	@echo "  make all     : Build server and client executables."
	@echo "  make clean   : Remove executables and object files."
	@echo "  make rebuild : Clean and then build all."
	@echo "  make help    : Show this help message."

.PHONY: all clean rebuild help
