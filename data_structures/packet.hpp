#ifndef BOOST_SIMULATION_PACKET_HPP
#define BOOST_SIMULATION_PACKET_HPP

#include <assert.h>
#include <iostream>
#include <string>

using namespace std;

/*******************************************/
/**************** Packet ****************/
/*******************************************/
struct Packet{
  Packet(){}
  Packet(int i_packetNum, bool i_matched, uint32_t i_match)
   :packetNum(i_packetNum), matched(i_matched), match(i_match){}

  	int   packetNum; // packet number
    bool   matched; // true if the packet header is matched one of the flow entries in the router's flow table
  	uint32_t   match; // matching header (to lookup the forwarding rules installed by the controller in the flow table)
};

istream& operator>> (istream& is, Packet& msg);

ostream& operator<<(ostream& os, const Packet& msg);


#endif // BOOST_SIMULATION_PACKET_HPP