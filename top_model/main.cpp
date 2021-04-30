//Cadmium Simulator headers
#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/dynamic_model.hpp>
#include <cadmium/modeling/dynamic_model_translator.hpp>
#include <cadmium/engine/pdevs_dynamic_runner.hpp>
#include <cadmium/logger/common_loggers.hpp>

//Time class header
#include <NDTime.hpp>

//Messages structures
#include "../data_structures/packet.hpp"

//Atomic model headers
#include <cadmium/basic_model/pdevs/iestream.hpp> //Atomic model for inputs
#include "../atomics/sender.hpp"
#include "../atomics/processor.hpp"
#include "../atomics/flowtable.hpp"
#include "../atomics/controller.hpp"
#include "../atomics/receiver.hpp"

//C++ headers
#include <iostream>
#include <chrono>
#include <algorithm>
#include <string>


using namespace std;
using namespace cadmium;
using namespace cadmium::basic_models::pdevs;

using TIME = NDTime;

/***** Define input port for coupled models *****/
struct inp_control : public in_port<int>{}; //from the OFP
struct inp_ack : public in_port<int>{}; //from the receiver
struct inp_1 : public in_port<Packet>{}; //from the sender
struct inp_2 : public in_port<Packet>{}; //from the controller
/***** Define output ports for coupled model *****/
struct outp : public out_port<int>{}; //to the OFP
struct outp_ackNum : public out_port<int>{}; //to the OFP
struct outp_ack : public out_port<int>{}; //to the sender
struct outp_1 : public out_port<Packet>{}; //to the receiver
struct outp_2 : public out_port<Packet>{}; //to the controller

/****** Input Reader atomic model declaration *******************/
template<typename T>
class InputReader_Int : public iestream_input<int,T> {
public:
    InputReader_Int() = default;
    InputReader_Int(const char* file_path) : iestream_input<int,T>(file_path) {}
};

int main(int argc, char ** argv) {

    if (argc < 2) {
        cout << "Program used with wrong parameters. The program must be invoked as follow:";
        cout << argv[0] << " path to the input file " << endl;
        return 1; 
    }
    /****** Input Reader atomic model instantiation *******************/
    string input = argv[1];
    const char * i_input = input.c_str();
    shared_ptr<dynamic::modeling::model> input_reader = dynamic::translate::make_dynamic_atomic_model<InputReader_Int, TIME, const char* >("input_reader" , move(i_input));

    /****** Sender atomic model instantiation *******************/
    shared_ptr<dynamic::modeling::model> sender1 = dynamic::translate::make_dynamic_atomic_model<Sender, TIME>("sender1");

    /****** Receiver atomic model instantiation *******************/
    shared_ptr<dynamic::modeling::model> receiver1 = dynamic::translate::make_dynamic_atomic_model<Receiver, TIME>("receiver1");

    /****** Processor atomic models instantiation *******************/
    shared_ptr<dynamic::modeling::model> processor1 = dynamic::translate::make_dynamic_atomic_model<Processor, TIME>("processor1");

    /****** FlowTable atomic models instantiation *******************/
    shared_ptr<dynamic::modeling::model> flowtable1 = dynamic::translate::make_dynamic_atomic_model<FlowTable, TIME>("flowtable1");

    /****** Controller atomic models instantiation *******************/
    shared_ptr<dynamic::modeling::model> controller1 = dynamic::translate::make_dynamic_atomic_model<Controller, TIME>("controller1");

    /*******OFRouter COUPLED MODEL********/
    dynamic::modeling::Ports iports_OFRouter = {typeid(inp_1),typeid(inp_2), typeid(inp_ack)};
    dynamic::modeling::Ports oports_OFRouter = {typeid(outp_1),typeid(outp_2),typeid(outp_ack)};
    dynamic::modeling::Models submodels_OFRouter = {processor1, flowtable1};
    dynamic::modeling::EICs eics_OFRouter = {
        dynamic::translate::make_EIC<inp_1, Processor_defs::in>("processor1"),
        dynamic::translate::make_EIC<inp_2, FlowTable_defs::in>("flowtable1"),
        dynamic::translate::make_EIC<inp_ack, Processor_defs::ack>("processor1")
    };
    dynamic::modeling::EOCs eocs_OFRouter = {
        dynamic::translate::make_EOC<Processor_defs::done,outp_ack>("processor1"),
        dynamic::translate::make_EOC<Processor_defs::out,outp_1>("processor1"),
        dynamic::translate::make_EOC<FlowTable_defs::out,outp_2>("flowtable1")
    };
    dynamic::modeling::ICs ics_OFRouter = {
        dynamic::translate::make_IC<Processor_defs::match_out, FlowTable_defs::match_in>("processor1","flowtable1"),
        dynamic::translate::make_IC<FlowTable_defs::match_out, Processor_defs::match_in>("flowtable1","processor1")
    };
    shared_ptr<dynamic::modeling::coupled<TIME>> OFRouter;
    OFRouter = make_shared<dynamic::modeling::coupled<TIME>>(
        "OFRouter", submodels_OFRouter, iports_OFRouter, oports_OFRouter, eics_OFRouter, eocs_OFRouter, ics_OFRouter 
    );

    /*******OFP SIMULATOR COUPLED MODEL********/
    dynamic::modeling::Ports iports_OFP = {typeid(inp_control)};
    dynamic::modeling::Ports oports_OFP = {typeid(outp), typeid(outp_ackNum)};
    dynamic::modeling::Models submodels_OFP = {sender1, receiver1, controller1, OFRouter};
    dynamic::modeling::EICs eics_OFP = {
        dynamic::translate::make_EIC<inp_control, Sender_defs::controlIn>("sender1")
    };
    dynamic::modeling::EOCs eocs_OFP = {
        dynamic::translate::make_EOC<Sender_defs::packetNumSentOut,outp>("sender1"),
        dynamic::translate::make_EOC<Sender_defs::ackReceivedOut,outp_ackNum>("sender1")
    };
    dynamic::modeling::ICs ics_OFP = {
        dynamic::translate::make_IC<Sender_defs::packetSentOut, inp_1>("sender1","OFRouter"),
        dynamic::translate::make_IC<Receiver_defs::ackOut, inp_ack>("receiver1","OFRouter"),
        dynamic::translate::make_IC<Controller_defs::packet_out, inp_2>("controller1","OFRouter"),
        dynamic::translate::make_IC<outp_ack, Sender_defs::ackIn>("OFRouter","sender1"),
        dynamic::translate::make_IC<outp_1, Receiver_defs::packetReceivedIn>("OFRouter","receiver1"),
        dynamic::translate::make_IC<outp_2, Controller_defs::packet_in>("OFRouter","controller1")
    };
    shared_ptr<dynamic::modeling::coupled<TIME>> OFP;
    OFP = make_shared<dynamic::modeling::coupled<TIME>>(
        "OFP", submodels_OFP, iports_OFP, oports_OFP, eics_OFP, eocs_OFP, ics_OFP 
    );


    /*******TOP COUPLED MODEL********/
    dynamic::modeling::Ports iports_TOP = {};
    dynamic::modeling::Ports oports_TOP = {typeid(outp), typeid(outp_ackNum)};
    dynamic::modeling::Models submodels_TOP = {input_reader, OFP};
    dynamic::modeling::EICs eics_TOP = {};
    dynamic::modeling::EOCs eocs_TOP = {
        dynamic::translate::make_EOC<outp,outp>("OFP"),
        dynamic::translate::make_EOC<outp_ackNum,outp_ackNum>("OFP")
    };
    dynamic::modeling::ICs ics_TOP = {
        dynamic::translate::make_IC<iestream_input_defs<int>::out, inp_control>("input_reader","OFP")
    };
    shared_ptr<cadmium::dynamic::modeling::coupled<TIME>> TOP;
    TOP = make_shared<dynamic::modeling::coupled<TIME>>(
        "TOP", submodels_TOP, iports_TOP, oports_TOP, eics_TOP, eocs_TOP, ics_TOP 
    );

    /*************** Loggers *******************/
    static ofstream out_messages("../simulation_results/OFP_output_messages.txt");
    struct oss_sink_messages{
        static ostream& sink(){          
            return out_messages;
        }
    };
    static ofstream out_state("../simulation_results/OFP_output_state.txt");
    struct oss_sink_state{
        static ostream& sink(){          
            return out_state;
        }
    };
    
    using state=logger::logger<logger::logger_state, dynamic::logger::formatter<TIME>, oss_sink_state>;
    using log_messages=logger::logger<logger::logger_messages, dynamic::logger::formatter<TIME>, oss_sink_messages>;
    using global_time_mes=logger::logger<logger::logger_global_time, dynamic::logger::formatter<TIME>, oss_sink_messages>;
    using global_time_sta=logger::logger<logger::logger_global_time, dynamic::logger::formatter<TIME>, oss_sink_state>;

    using logger_top=logger::multilogger<state, log_messages, global_time_mes, global_time_sta>;

    /************** Runner call ************************/ 
    dynamic::engine::runner<NDTime, logger_top> r(TOP, {0});
    r.run_until_passivate();
    return 0;
}