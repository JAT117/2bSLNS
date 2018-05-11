/*
 * By Jorge Cardona
 * jac0656@unt.edu
 */

//Compiles with: g++ testDMX512.c++ dmx512.c++ $(pkg-config --cflags --libs libola) -std=c++11 -Wall
//Here testDMX.c++ is a file that includes this library

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <ctype.h> //isxdigit()
#include <unistd.h> //sleep

#include "dmx512.hpp"

//OLA

#include <ola/DmxBuffer.h>
#include <ola/Logging.h>
#include <ola/client/StreamingClient.h>

DMX512::DMX512(){
    
  // Lets make sure OLA is up and running
  std::string line = "";
  std::string lines;
  std::vector<std::string> i;
  
  system("pidof olad  > tempFile-1");
  
  std::ifstream fd("tempFile-1");
  if(!fd)
    std::cerr << "CANT OPEN FILE\n";
  
  while(std::getline(fd, line)){
    i.push_back(line);
  }
  
  if(i.size() <= 0){
    std::cout << "OLA IS DOWN\n";

    system("sudo ola_dev_info > /dev/null 2>&1 &");
    for(int i=0; i<10; i++){
      std::cout << "\nOLA is Starting Up..";
      sleep(1);
    }
    
    std::cout << "\nPatching device 8's port 1 to Universe 1\n";
    std::cout << "\tdevice 8 is our FTDI USB Adapter\n";
    std::cout << "\tsee ola_dev_info\n";
    sleep(3);
    system("ola_patch -d 2 -p 1 -u 1 > /dev/null 2>&1 &"); 

    std::cout << "Sending dummy data\n";
    system("ola_streaming_client  -u 1 -d 1,2,255,255 > /dev/null 2>&1 &");
    sleep(3);
  }
  else{
    system("sudo ola_dev_info > /dev/null 2>&1 &");
    system("ola_patch -d 2 -p 1 -u 1 > /dev/null 2>&1 &"); 
    //std::cout << "OLA IS UP\n";
  }
  
  system("rm tempFile-1");
  std::cout << "OLA IS UP\n";
  
}


void DMX512::setOutOfRange(int i ){
  outOfRange = i;
}

int DMX512::getOutOfRange( ){
  return outOfRange;
}

void DMX512::setData(std::string DATA){
  try{
    A = DATA.substr(0, 2);
    R = DATA.substr(2, 2);
    G = DATA.substr(4, 2);
    B = DATA.substr(6, 7);
  }
  catch(std::out_of_range ){
    A = "00";
    R = "00";
    G = "00";
    B = "00";
    
    setOutOfRange(1);
  }
  
  std::cout << A << R << G << B << std::endl;
  clientHandler();
}

void DMX512::clientHandler(){
  
  std::stringstream ssOLA;
  ssOLA.clear();
  ssOLA << std::hex << A << " " <<  std::hex << R << " " <<  std::hex << G <<" " << std::hex << B;
  ssOLA >> Ai >> Ri >> Gi >> Bi;
  std::cout <<"Int Equivalent: " << Ai <<" " <<  Ri << " " <<  Gi << " " <<  Bi << std::endl;
  
}


void DMX512::checkOLA(){
  // It doesnt Hurt to check...
  std::string line = "";
  std::vector<std::string> i;
  system("pidof olad  > tempFile-1");
  std::ifstream fd("tempFile-1");
  if(!fd)
    std::cerr << "CANT OPEN FILE\n";
  while(std::getline(fd, line)){
    i.push_back(line);
  }
  if(i.size() <= 0){
    std::cout << "OLA IS DOWN\n";
    system("sudo olad -l 4 > /dev/null 2>&1 &");
    system("ola_patch -d 2 -p 1 -u 1 > /dev/null 2>&1 &"); 
    system("ola_streaming_client  -u 1 -d 1,2,255,255 > /dev/null 2>&1 &");
  }
  else
    std::cout << "OLA IS UP\n";
  system("rm tempFile-1");
}


std::string  DMX512::sendOLA(){
  
  /*NOTE:
   * Return types:
   *  1 OLA Success
   * -1 OLA Failure
   * -2 Wrong Data Type
   * -3 Empty String
   */
    
  unsigned int universe = 1;
  ola::DmxBuffer buffer; // A DmxBuffer to hold the data.
  ola::client::StreamingClient ola_client((ola::client::StreamingClient::Options()));
  
  int temp = 0;
  OLA = A+R+G+B;
  std::vector<char> cDATA(OLA.c_str(), OLA.c_str() +(OLA.size()));
  
  //Syntesize data
  for (auto i: cDATA){ //C++11 for range
    std::cout << i << ' ';
    if(isxdigit(i) == 0){
      std::cout << "Wrong data type at possition: " << temp<< "\n";
      return "-2";
    }
    temp += 1;
  }
  
  if (temp > 0){
    std::cout << "Data processed\n";
    temp = 0;
  }

  // In case it went down while we are running...
  checkOLA();
  
  if (!ola_client.Setup()){
    std::cerr << "Setup failed" << std::endl;
    return "-1";
  }
  
  buffer.SetChannel(0, Ai); // Intensity or Alpha
  buffer.SetChannel(1, Ri); // Red
  buffer.SetChannel(2, Gi); // Green
  buffer.SetChannel(3, Bi); // Blue                                                                                                                                         
  if(!ola_client.SendDmx(universe, buffer)){
    return "-1";
  }
  
  std::cout << "\nColor Sent!\n\n";
  std::cout << Ai <<" " <<  Ri <<  " " << Gi << " " << Bi <<std::endl;
  
  if(getOutOfRange()){
    setOutOfRange(0);
    return "-3";
  }
  
  return "1";
}
