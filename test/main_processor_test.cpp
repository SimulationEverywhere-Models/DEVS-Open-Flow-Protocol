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
#include "../atomics/processor.hpp"

//C++ libraries
#include <iostream>
#include <string>

using namespace std;
using namespace cadmium;
using namespace cadmium::basic_models::pdevs;

using TIME = NDTime;

/***** Define input port for coupled models *****/
struct top_in: public in_port<Packet>{};
struct top_ackIn: public in_port<int>{};
struct top_matchIn: public in_port<Packet>{};

/***** Define output ports for coupled model *****/
struct top_out: public out_port<Packet>{};
struct top_ackOut: public out_port<int>{};
struct top_matchOut: public out_port<Packet>{};

/****** Input Reader atomic model declaration *******************/
template<typename T>
class InputReader_Packet : public iestream_input<Packet,T> {
    public:
        InputReader_Packet () = default;
        InputReader_Packet (const char* file_path) : iestream_input<Packet,T>(file_path) {}
};

template<typename T>
class InputReader_Int : public iestream_input<int,T> {
    public:
        InputReader_Int () = default;
        InputReader_Int (const char* file_path) : iestream_input<int,T>(file_path) {}
};

int main(){

    /****** Input Reader atomic model instantiation *******************/
    const char * i_input_data_packet = "../input_data/processor_input_test_packet.txt";
    shared_ptr<dynamic::modeling::model> input_reader_packet = dynamic::translate::make_dynamic_atomic_model<InputReader_Packet, TIME, const char*>("input_reader_packet", move(i_input_data_packet));
    const char * i_input_data_ack = "../input_data/processor_input_test_ack.txt";
    shared_ptr<dynamic::modeling::model> input_reader_ack = dynamic::translate::make_dynamic_atomic_model<InputReader_Int, TIME, const char*>("input_reader_ack", move(i_input_data_ack));
    const char * i_input_data_matchin = "../input_data/processor_input_test_matchin.txt";
    shared_ptr<dynamic::modeling::model> input_reader_matchin = dynamic::translate::make_dynamic_atomic_model<InputReader_Packet, TIME, const char*>("input_reader_matchin", move(i_input_data_matchin));

    /****** Processor atomic model instantiation *******************/
    shared_ptr<dynamic::modeling::model> processor1 = dynamic::translate::make_dynamic_atomic_model<Processor, TIME>("processor1");

    /*******TOP MODEL********/
    dynamic::modeling::Ports iports_TOP = {};
    dynamic::modeling::Ports oports_TOP = {typeid(top_out), typeid(top_ackOut), typeid(top_matchOut)};
    dynamic::modeling::Models submodels_TOP = {input_reader_packet, input_reader_ack, input_reader_matchin, processor1};
    dynamic::modeling::EICs eics_TOP = {};
    dynamic::modeling::EOCs eocs_TOP;
    eocs_TOP = {
        dynamic::translate::make_EOC<Processor_defs::out,top_out>("processor1"),
        dynamic::translate::make_EOC<Processor_defs::done,top_ackOut>("processor1"),
        dynamic::translate::make_EOC<Processor_defs::match_out,top_matchOut>("processor1")
    };
    dynamic::modeling::ICs ics_TOP;
    ics_TOP = {
        dynamic::translate::make_IC<iestream_input_defs<Packet>::out,Processor_defs::in>("input_reader_packet","processor1"),
        dynamic::translate::make_IC<iestream_input_defs<int>::out,Processor_defs::ack>("input_reader_ack","processor1"),
        dynamic::translate::make_IC<iestream_input_defs<Packet>::out,Processor_defs::match_in>("input_reader_matchin","processor1")
    };
    shared_ptr<dynamic::modeling::coupled<TIME>> TOP;
    TOP = make_shared<dynamic::modeling::coupled<TIME>>(
        "TOP", submodels_TOP, iports_TOP, oports_TOP, eics_TOP, eocs_TOP, ics_TOP 
    );

    /*************** Loggers *******************/
    static ofstream out_messages("../simulation_results/processor_test_output_messages.txt");
    struct oss_sink_messages{
        static ostream& sink(){          
            return out_messages;
        }
    };
    static ofstream out_state("../simulation_results/processor_test_output_state.txt");
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
    r.run_until(NDTime("04:00:00:000"));
    return 0;
}
