This simulator is created to test the performance of a network managed by a logically-centralized SDN-controller using the Discrete Event Modelling and Simulation Methodology DEVS. The simulator is very simple and can be extended to evaluate the performance and limitations of Software-Defined Networks in large-scale networks. 

To excute the simulator:

-------Navigate to the project folder--------
-------Compilation---------------------------
$ make clean; make all
-------Testing atomic models----------------
$ cd bin
./SENDER_TEST 
$ ./PROCESSOR_TEST 
$ ./FLOWTABLE_TEST 
$ ./CONTROLLER_TEST 
$ ./RECEIVER_TEST 

-------Testing the simulator----------------
$ ./OFP ../input_data/input_ofp.txt 


-------clean-----------------------
rm -f bin/* build/*
