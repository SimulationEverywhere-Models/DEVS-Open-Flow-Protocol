/**
* MOHAMMED ALSAEEDI 101109046
* COURSE_SYSC5104A - Carleton University
*
* Receiver:
* Cadmium implementation of Receiver atomic model
* 
*/

#ifndef __RECEIVER_HPP__
#define __RECEIVER_HPP__


#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/message_bag.hpp>

#include <limits>
#include <assert.h>
#include <string>

#include "../data_structures/packet.hpp"

using namespace cadmium;
using namespace std;

//Port definition
struct Receiver_defs{
struct ackOut : public out_port<int> { };
struct packetReceivedIn : public in_port<Packet> { };
};

template<typename TIME> class Receiver{
public:
    //Parameters to be overwriten when instantiating the atomic model
    TIME   preparationTime;
    TIME   timeout;
    // default constructor
    Receiver() noexcept{
      preparationTime  = TIME("00:00:02");
      state.ackNum    = 0;
      state.sending  = false;
    }
    
    // state definition
    struct state_type{
      int ackNum;
      bool sending;
    }; 
    state_type state;
    // ports definition
    using input_ports=std::tuple<typename Receiver_defs::packetReceivedIn>;
    using output_ports=std::tuple<typename Receiver_defs::ackOut>;

    // internal transition
    void internal_transition() {
          state.sending = false;
    }

    // external transition
    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) { 
      if(get_messages<typename Receiver_defs::packetReceivedIn>(mbs).size()>1) 
          assert(false && "one message per time uniti");
      vector<Packet> message_port_in;
      message_port_in = get_messages<typename Receiver_defs::packetReceivedIn>(mbs);
      state.ackNum = message_port_in[0].packetNum;
      state.sending = true;                     
    }

    // confluence transition
    void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
      internal_transition();
      external_transition(TIME(), std::move(mbs));
    }

    // output function
    typename make_message_bags<output_ports>::type output() const {
      typename make_message_bags<output_ports>::type bags;
      get_messages<typename Receiver_defs::ackOut>(bags).push_back(state.ackNum);
      return bags;
    }

    // time_advance function
    TIME time_advance() const {  
      TIME next_internal;
      if (state.sending) {
        next_internal = preparationTime;
      }else {
        next_internal = std::numeric_limits<TIME>::infinity();
      }    
      return next_internal;
    }

    friend std::ostringstream& operator<<(std::ostringstream& os, const typename Receiver<TIME>::state_type& i) {
        os << "packetNum: " << i.ackNum; 
    return os;
    }
};     


#endif // __RECEIVER_HPP__