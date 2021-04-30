/**
* MOHAMMED ALSAEEDI 101109046
* COURSE_SYSC5104A - Carleton University
*
* Controller:
* Cadmium implementation of Controller atomic model
* 
*/

#ifndef _CONTROLLER_HPP__
#define _CONTROLLER_HPP__

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
struct Controller_defs{
    struct packet_out : public out_port<Packet> {};
    struct packet_in : public in_port<Packet> {};
};

template<typename TIME> class Controller{
    public:
        //Parameters to be overwriten when instantiating the atomic model
        TIME   transmittingTime;
        // default constructor
        Controller() noexcept{
          transmittingTime  = TIME("00:00:07");
          state.transmitting = false;
        }        
        // state definition
        struct state_type{
            bool transmitting;
            Packet packet;
            TIME next_internal;
        }; 
        state_type state;
        // ports definition
        using input_ports=tuple<typename Controller_defs::packet_in>;
        using output_ports=tuple<typename Controller_defs::packet_out>;
    // internal transition
    void internal_transition() {
        state.transmitting = false; 
    }
    // external transition
    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {         
        if(get_messages<typename Controller_defs::packet_in>(mbs).size()>1) 
            assert(false && "one message per time uniti");
        vector<Packet> message_port_in;
        message_port_in = get_messages<typename Controller_defs::packet_in>(mbs);  
        state.transmitting = true;
        state.packet = message_port_in[0];
        state.packet.matched = true;     
    }
    // confluence transition
    void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
        internal_transition();
        external_transition(TIME(), move(mbs));
    }
    // output function
    typename make_message_bags<output_ports>::type output() const {
        typename make_message_bags<output_ports>::type bags;
        if(state.transmitting){
            get_messages<typename Controller_defs::packet_out>(bags).push_back(state.packet);
        }
        return bags;
    }
    // time_advance function
    TIME time_advance() const {
        TIME next_internal;
        if (state.transmitting) {            
            next_internal = transmittingTime;
        }else {
            next_internal = numeric_limits<TIME>::infinity();
        }    
        return next_internal;
    }

    friend ostringstream& operator<<(ostringstream& os, const typename Controller<TIME>::state_type& i) {
        os << "packetNum: " << i.packet.packetNum << " & transmitting: " << i.transmitting; 
        return os;
    }
};    
#endif // _CONTROLLER_HPP__