/**
* MOHAMMED ALSAEEDI 101109046
* COURSE_SYSC5104A - Carleton University
*
* Sender:
* Cadmium implementation of Sender atomic model
* 
*/

#ifndef __SENDER_HPP__
#define __SENDER_HPP__

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
struct Sender_defs{
    struct controlIn : public in_port<int> { };
    struct ackIn : public in_port<int> { };
    struct packetSentOut : public out_port<Packet> { };
    struct packetNumSentOut : public out_port<int> { };
    struct ackReceivedOut : public out_port<int> {};
};

template<typename TIME> class Sender{
    public:
        //Parameters to be overwriten when instantiating the atomic model
        TIME   sendingTime;
        TIME   ackTimeout;
        // default constructor
        Sender() noexcept{
          sendingTime  = TIME("00:00:05");
          ackTimeout          = TIME("00:00:15");
          state.next_internal    = std::numeric_limits<TIME>::infinity();
          state.model_active     = false;
          state.ackNum = 0;
        }
        
        // state definition
        struct state_type{
            Packet packet;
            int totalPacketNum;
            bool ack;
            int ackNum;
            bool sending;
            bool model_active;
            TIME next_internal;
        }; 
        state_type state;
        // ports definition
        using input_ports=std::tuple<typename Sender_defs::controlIn, typename Sender_defs::ackIn>;
        using output_ports=std::tuple<typename Sender_defs::packetSentOut, typename Sender_defs::packetNumSentOut, typename Sender_defs::ackReceivedOut>;

        // internal transition
        void internal_transition() {
            if (state.ack){
                if (state.packet.packetNum < state.totalPacketNum){
                    state.packet.packetNum ++;
                state.ack = false;
                state.packet.match = (uint32_t)rand();
                state.sending = true;
                state.model_active = true; 
                state.next_internal = sendingTime;   
                } else {
                    state.model_active = false;
                    state.next_internal = std::numeric_limits<TIME>::infinity();
                }
            } else{
                if (state.sending){
                    state.sending = false;
                    state.model_active = true;
                    state.next_internal = ackTimeout;
                } else {
                    state.sending = true;
                    state.model_active = true;
                    state.next_internal = sendingTime;    
                } 
            }   
        }

        // external transition
        void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) { 
            if((get_messages<typename Sender_defs::controlIn>(mbs).size()+get_messages<typename Sender_defs::ackIn>(mbs).size())>1) 
                    assert(false && "one message per time uniti");
            for(const auto &x : get_messages<typename Sender_defs::controlIn>(mbs)){
                if(state.model_active == false){
                    state.totalPacketNum = x;
                    if (state.totalPacketNum > 0){
                        state.packet.packetNum = 1;
                        state.packet.match = (uint32_t)rand();
                        state.ack = false;
                        state.sending = true;
                        state.packet.matched = false;
                        state.model_active = true;
                        state.next_internal = sendingTime;
                    }else{
                        if(state.next_internal != std::numeric_limits<TIME>::infinity()){
                            state.next_internal = state.next_internal - e;
                        }
                    }
                }
            }
            for(const auto &x : get_messages<typename Sender_defs::ackIn>(mbs)){
                if(state.model_active == true) { 
                    state.ack = true;
                    state.ackNum = x;
                    state.sending = false;
                    state.next_internal = TIME("00:00:00");
                }            
            }
        }

        // confluence transition
        void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
            internal_transition();
            external_transition(TIME(), std::move(mbs));
        }

        // output function
        typename make_message_bags<output_ports>::type output() const {
            typename make_message_bags<output_ports>::type bags;
            if (state.sending){
                get_messages<typename Sender_defs::packetSentOut>(bags).push_back(state.packet);
                get_messages<typename Sender_defs::packetNumSentOut>(bags).push_back(state.packet.packetNum);
            }else{
                if (state.ack){
                    get_messages<typename Sender_defs::ackReceivedOut>(bags).push_back(state.ackNum);
                }
            }    
            return bags;
        }

        // time_advance function
        TIME time_advance() const {  
             return state.next_internal;
        }

        friend std::ostringstream& operator<<(std::ostringstream& os, const typename Sender<TIME>::state_type& i) {
            os << "packetNum: " << i.packet.packetNum << " & totalPacketNum: " << i.totalPacketNum; 
        return os;
        }
};     
#endif // __SENDER_HPP__