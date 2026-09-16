CXX = g++
CXXFLAGS = -Wall -std=c++11

all: sender receiver

sender: sender.cpp packet.hpp logger.hpp timer.hpp
	$(CXX) $(CXXFLAGS) sender.cpp -o sender

receiver: receiver.cpp packet.hpp logger.hpp
	$(CXX) $(CXXFLAGS) receiver.cpp -o receiver

clean:
	rm -f sender receiver output.txt cwnd.csv throughput.csv events.log receiver_events.log cwnd.svg throughput.svg

phase5: all
	./run_experiments.sh

bonus: all
	./run_fast_retransmit_experiments.sh
