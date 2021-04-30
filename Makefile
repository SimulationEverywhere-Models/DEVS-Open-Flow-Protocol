CC=g++
CFLAGS=-std=c++17

INCLUDECADMIUM=-I ../../cadmium/include
INCLUDEDESTIMES=-I ../../DESTimes/include

#CREATE BIN AND BUILD FOLDERS TO SAVE THE COMPILED FILES DURING RUNTIME
bin_folder := $(shell mkdir -p bin)
build_folder := $(shell mkdir -p build)
results_folder := $(shell mkdir -p simulation_results)

#TARGET TO COMPILE ALL THE TESTS TOGETHER (NOT SIMULATOR)
packet.o: data_structures/packet.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) data_structures/packet.cpp -o build/packet.o

main_top.o: top_model/main.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) top_model/main.cpp -o build/main_top.o
	
main_sender_test.o: test/main_sender_test.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) test/main_sender_test.cpp -o build/main_sender_test.o
	
main_processor_test.o: test/main_processor_test.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) test/main_processor_test.cpp -o build/main_processor_test.o
	
main_flowtable_test.o: test/main_flowtable_test.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) test/main_flowtable_test.cpp -o build/main_flowtable_test.o
	
main_controller_test.o: test/main_controller_test.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) test/main_controller_test.cpp -o build/main_controller_test.o
	
main_receiver_test.o: test/main_receiver_test.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) test/main_receiver_test.cpp -o build/main_receiver_test.o

tests: main_sender_test.o main_processor_test.o main_processor_test.o main_flowtable_test.o main_controller_test.o main_receiver_test.o packet.o
		$(CC) -g -o bin/SENDER_TEST build/main_sender_test.o build/packet.o 
		$(CC) -g -o bin/PROCESSOR_TEST build/main_processor_test.o build/packet.o 
		$(CC) -g -o bin/FLOWTABLE_TEST build/main_flowtable_test.o build/packet.o 
		$(CC) -g -o bin/CONTROLLER_TEST build/main_controller_test.o build/packet.o 
		$(CC) -g -o bin/RECEIVER_TEST build/main_receiver_test.o build/packet.o 

#TARGET TO COMPILE ONLY OFP SIMULATOR
simulator: main_top.o packet.o 
	$(CC) -g -o bin/OFP build/main_top.o build/packet.o 
	
#TARGET TO COMPILE EVERYTHING (OFP SIMULATOR + TESTS TOGETHER)
all: simulator tests

#CLEAN COMMANDS
clean: 
	rm -f bin/* build/*