#include <math.h> 
#include <assert.h>
#include <iostream>
#include <fstream>
#include <string>

#include "packet.hpp"

/***************************************************/
/************* Output stream ************************/
/***************************************************/

ostream& operator<<(ostream& os, const Packet& msg) {
  os << msg.packetNum << " " << msg.matched << " " << msg.match;
  return os;
}

/***************************************************/
/************* Input stream ************************/
/***************************************************/

istream& operator>> (istream& is, Packet& msg) {
  is >> msg.packetNum;
  is >> msg.matched;
  is >> msg.match;
  return is;
}
