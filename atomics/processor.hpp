/**
* MOHAMMED ALSAEEDI 101109046
* COURSE_SYSC5104A - Carleton University
*
* Processor:
* Cadmium implementation of Processor atomic model
* 
*/

#ifndef _PROCESSOR_HPP__
#define _PROCESSOR_HPP__

#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/message_bag.hpp>

#include <limits>
#include <assert.h>
#include <string>
#include <random>

#include "../data_structures/packet.hpp"

using namespace cadmium;
using namespace std;

//Port definition
struct Processor_defs{
    struct out : public out_port<Packet> {};
    struct match_out : public out_port<Packet> {};
    struct done : public out_port<int> {};
    struct in : public in_port<Packet> {};
    struct match_in : public in_port<Packet> {};
    struct ack : public in_port<int> {};
};

template<typename TIME> class Processor{
    public:
        //Parameters to be overwriten when instantiating the atomic model

        // default constructor
        Processor() {
            state.transmitting = false;
            state.ack = false;
            state.ackNum = 0;
        }
        
        // state definition
        struct state_type{
            bool transmitting;
            bool ack;
            int ackNum;
            Packet packet;         
        }; 
        state_type state;
        // ports definition
        using input_ports=tuple<typename Processor_defs::in, typename Processor_defs::ack, typename Processor_defs::match_in>;
        using output_ports=tuple<typename Processor_defs::out, typename Processor_defs::done, typename Processor_defs::match_out>;
    // internal transition
    void internal_transition() {
        state.transmitting = false;
    }
    // external transition
    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) { 
        if((get_messages<typename Processor_defs::in>(mbs).size())>1) 
                assert(false && "one message per time uniti"); 
        
        for(const auto &x : get_messages<typename Processor_defs::in>(mbs)){  
            state.packet = x;
            state.ack = false;
            state.transmitting = true;
        }
        
        for(const auto &x : get_messages<typename Processor_defs::ack>(mbs)){
            state.ackNum = x;
            state.ack = true;
            state.transmitting = true; 
        }

        for(const auto &x : get_messages<typename Processor_defs::match_in>(mbs)){
            state.packet.matched = x.matched;
            state.ack = false;
            state.transmitting = true;   
        }
    }
    // confluence transition
    void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
        internal_transition();
        external_transition(TIME(), move(mbs));
    }
    // output function
    typename make_message_bags<output_ports>::type output() const {
        typename make_message_bags<output_ports>::type bags;
        if (state.transmitting){  
            if(state.ack){
                get_messages<typename Processor_defs::done>(bags).push_back(state.packet.packetNum); 
            }else{    
                if(state.packet.matched){
                    get_messages<typename Processor_defs::out>(bags).push_back(state.packet);  
                }else{
                    get_messages<typename Processor_defs::match_out>(bags).push_back(state.packet); 
                }
            }
        }
        return bags;
    }
    // time_advance function
    TIME time_advance() const {
        TIME next_internal;
        if (state.transmitting) {            
            next_internal = TIME("00:00:02");
        }else {
            next_internal = numeric_limits<TIME>::infinity();
        }    
        return next_internal;
    }

    friend ostringstream& operator<<(ostringstream& os, const typename Processor<TIME>::state_type& i) {
        os << "packetNum: " << i.packet.packetNum << " & transmitting: " << i.transmitting; 
        return os;
    }
};    
#endif // _PROCESSOR_HPP__