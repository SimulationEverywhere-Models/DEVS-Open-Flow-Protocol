/**
* MOHAMMED ALSAEEDI 101109046
* COURSE_SYSC5104A - Carleton University
*
* FlowTable:
* Cadmium implementation of FlowTable atomic model
* 
*/

#ifndef _FLOWTABLE_HPP__
#define _FLOWTABLE_HPP__

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
struct FlowTable_defs{
    struct out : public out_port<Packet> {};
    struct match_out : public out_port<Packet> {};
    struct in : public in_port<Packet> {};
    struct match_in : public in_port<Packet> {};
};

template<typename TIME> class FlowTable{
    public:
    // ports definition
    using input_ports=tuple<typename FlowTable_defs::in, typename FlowTable_defs::match_in>;
    using output_ports=tuple<typename FlowTable_defs::out, typename FlowTable_defs::match_out>;
    // state definition
    struct state_type{
        bool transmitting;
        Packet packet; 
        int flowTableMaxSize;
        vector<pair<uint32_t, TIME>> flowTable;
    }; 
    state_type state;    
    // default constructor
    FlowTable() {
        state.flowTableMaxSize = 100;
        state.transmitting = false;
    }     
    // internal transition
    void internal_transition() {  
        state.transmitting = false;
    }
    // external transition
    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) { 
        if((get_messages<typename FlowTable_defs::in>(mbs).size()+get_messages<typename FlowTable_defs::match_in>(mbs).size())>1) 
                assert(false && "one message per time uniti");
            //for loop to remove any timeout flow entry elapsed time - flow entry time <> 20 second     
        for(const auto &x : get_messages<typename FlowTable_defs::in>(mbs)){
            //add flow entry to the flow table if controler response with the appropriate flow entry and the flow table is not full             
            if (state.flowTable.size() < state.flowTableMaxSize){                  
                state.packet = x;
                state.packet.matched = true;
                state.transmitting = true;
                pair<uint32_t, TIME> flowEntry = make_pair(state.packet.match, TIME());
                state.flowTable.push_back(flowEntry);
            }else{
                //remove non-fresh vector (flow entry)
                state.flowTable.erase(state.flowTable.begin());
                state.packet = x;
                state.packet.matched = true;
                state.transmitting = true;
                pair<uint32_t, TIME> flowEntry = make_pair(state.packet.match, TIME());
                state.flowTable.push_back(flowEntry);
            }     
        }
        for(const auto &x : get_messages<typename FlowTable_defs::match_in>(mbs)){
            state.packet = x;
            for(const auto &y : state.flowTable){
                if(state.packet.match == y.first){
                    state.packet.matched = true;
                }else{
                    state.packet.matched = false;
                }
            }
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
        Packet out;
        out = state.packet;
        if(state.transmitting){
            if(state.packet.matched){
                get_messages<typename FlowTable_defs::match_out>(bags).push_back(out);
            }else{
                get_messages<typename FlowTable_defs::out>(bags).push_back(out);
            }
        }
        return bags;
    }
    // time_advance function
    TIME time_advance() const {
        TIME next_internal;
        if (state.transmitting) {            
            next_internal = TIME("00:00:03");
        }else {
            next_internal = numeric_limits<TIME>::infinity();
        }    
        return next_internal;
    }

    friend ostringstream& operator<<(ostringstream& os, const typename FlowTable<TIME>::state_type& i) {
        os << "packetNum: " << i.packet.packetNum << " & matched: " << i.packet.matched; 
        return os;
    }
};    
#endif // _FLOWTABLE_HPP__