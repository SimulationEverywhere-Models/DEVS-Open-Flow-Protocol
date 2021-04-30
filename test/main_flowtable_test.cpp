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
#include "../atomics/flowtable.hpp"

//C++ libraries
#include <iostream>
#include <string>

using namespace std;
using namespace cadmium;
using namespace cadmium::basic_models::pdevs;

using TIME = NDTime;

/***** Define input port for coupled models *****/
struct top_matchIn: public in_port<Packet>{};
struct top_matched: public in_port<Packet>{};

/***** Define output ports for coupled model *****/
struct top_matchOut: public out_port<Packet>{};
struct top_unmatched: public out_port<Packet>{};

/****** Input Reader atomic model declaration *******************/
template<typename T>
class InputReader_Packet : public iestream_input<Packet,T> {
    public:
        InputReader_Packet () = default;
        InputReader_Packet (const char* file_path) : iestream_input<Packet,T>(file_path) {}
};

int main(){

    /****** Input Reader atomic model instantiation *******************/
    const char * i_input_data_matchQ = "../input_data/flowtable_input_test_matchQ.txt"; //matching query
    shared_ptr<dynamic::modeling::model> input_reader_matchQ = dynamic::translate::make_dynamic_atomic_model<InputReader_Packet, TIME, const char*>("input_reader_matchQ", move(i_input_data_matchQ));
    const char * i_input_data_matchR = "../input_data/flowtable_input_test_matchR.txt"; //matching result
    shared_ptr<dynamic::modeling::model> input_reader_matchR = dynamic::translate::make_dynamic_atomic_model<InputReader_Packet, TIME, const char*>("input_reader_matchR", move(i_input_data_matchR));


    /****** Subnet atomic model instantiation *******************/
    shared_ptr<dynamic::modeling::model> flowtable1;
    flowtable1 = dynamic::translate::make_dynamic_atomic_model<FlowTable, TIME>("flowtable1");

    /*******TOP MODEL********/
    dynamic::modeling::Ports iports_TOP;
    iports_TOP = {};
    dynamic::modeling::Ports oports_TOP;
    oports_TOP = {typeid(top_matchOut), typeid(top_unmatched)};
    dynamic::modeling::Models submodels_TOP;
    submodels_TOP = {input_reader_matchQ, input_reader_matchR, flowtable1};
    dynamic::modeling::EICs eics_TOP;
    eics_TOP = {};
    dynamic::modeling::EOCs eocs_TOP;
    eocs_TOP = {
        dynamic::translate::make_EOC<FlowTable_defs::out,top_unmatched>("flowtable1"),
        dynamic::translate::make_EOC<FlowTable_defs::match_out,top_matchOut>("flowtable1")
    };
    dynamic::modeling::ICs ics_TOP;
    ics_TOP = {
        dynamic::translate::make_IC<iestream_input_defs<Packet>::out,FlowTable_defs::match_in>("input_reader_matchQ","flowtable1"),
        dynamic::translate::make_IC<iestream_input_defs<Packet>::out,FlowTable_defs::in>("input_reader_matchR","flowtable1")
    };
    shared_ptr<dynamic::modeling::coupled<TIME>> TOP;
    TOP = make_shared<dynamic::modeling::coupled<TIME>>(
        "TOP", submodels_TOP, iports_TOP, oports_TOP, eics_TOP, eocs_TOP, ics_TOP 
    );

    /*************** Loggers *******************/
    static ofstream out_messages("../simulation_results/flowtable_test_output_messages.txt");
    struct oss_sink_messages{
        static ostream& sink(){          
            return out_messages;
        }
    };
    static ofstream out_state("../simulation_results/flowtable_test_output_state.txt");
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
