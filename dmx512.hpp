/*
 * By Jorge Cardona
 * jac0656@unt.edu
 */

#pragma once
#include <string>

class DMX512{
  int outOfRange = 0;  
public:
 
  std::string A, R, G, B, OLA;
  
  unsigned int Ai, Ri, Gi, Bi;
  
  DMX512();
  void setOutOfRange(int i);
  int getOutOfRange();
  void setData(std::string DATA);
  void clientHandler();
  std::string sendOLA();
  void checkOLA();
};
