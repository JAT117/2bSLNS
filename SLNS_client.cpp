/* Spacecraft Lighting Network System Client
 * Client code written by John Austin Todd 2018
 * OLA combatability written by Jorge Cardona 2018
 * Sensor Interrupt written by Taylor Shinn 2018
 * Takes input from the SLNS server and commands OLA to change light values.
 * Sends sensor values back.
 * Dependencies: dmx512.cpp , dmx512.hpp for OLA 
 * compile: g++ -g -Wall -std=c++11 SLNS_client.cpp dmx512.cpp $(pkg-config --cflags --libs libola) -o Client
 */

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <ctime>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <math.h>

//OLA
#include "dmx512.hpp"

//Macros
#define port 9999
#define buff 128
//#define HOST "192.168.1.10" //for test server
#define HOST "192.168.1.12"  //server address hardcoded
#define ALARM 5

using namespace std;

//Fucntion Prototypes
int connect_to_server(void);
string time_processed(void); //timestamping
void display_RGB(int);

//Globals
int sockfd, reuse = 1,file, resp;	//only need the one socket on client side
struct sockaddr_in sADDR;
struct hostent *host; 		//this is typically needed for clients to get server information
char aRGB[8];
string message = "SLNS Client Confirmation message";

//#define scfpath "/home/pi/UNT-NASA/sensor_calibration_data.txt"
//fstream scf("home/pi/UNT-NASA/sensor_calibration_data.txt"); // Calibration file

int main()
{
	//needed for sensor calibration data
	//system("sudo rm ~/UNT-NASA/sensor_calibration_data.txt ; touch ~/UNT-NASA/sensor_calibration_data.txt ; sudo chmod 755 ~/UNT-NASA/sensor_calibration_data.txt");
	DMX512 ola; // Set up OLA (Jorge Cardona)
	connect_to_server();
	cout << "Connection made\n"; //connect to server success
	send(sockfd, message.c_str(), message.size(), MSG_DONTWAIT|MSG_NOSIGNAL); //send ack to server

	const char *bus = "/dev/i2c-1"; //set up sensor interupt (Taylor Shinn)
	if ((file = open(bus, O_RDWR)) < 0)
	{
		cout << "Failed to open the bus\n";
		exit(1);
	}
	ioctl(file, I2C_SLAVE, 0x29);
	signal(SIGALRM, display_RGB);
	alarm(ALARM);
	
	
	while(1)
	{
		char recv_data[buff];
		stringstream ss;
		string CMD, DATA;
		if((recv(sockfd, recv_data, sizeof(recv_data), MSG_NOSIGNAL)) == 0) // if connection is broken attempt to reconnect to server
		{
			cout << "Server Connection lost, Attempting to Reestablish\n";
			connect_to_server();
			cout << "Connection Reestablished\n"; //connect to server success
			resp = send(sockfd, message.c_str(), message.size(), MSG_DONTWAIT|MSG_NOSIGNAL); //send Client connection message to Server
			if ( resp <= 0 && errno == EPIPE )
			{
				close(sockfd);
				connect_to_server();
			}
		}
		else //process server commands
		{
			ss << recv_data;
			ss >> CMD >> DATA;
			cout << "Server Says:" << recv_data << endl;
			if (sizeof(CMD) != 0)
			{
				if((CMD == "SET")) //server sending GUI CR values or user defined values
				{
					string echo;
					ola.setData(DATA); //SET THE DATA
					cout << "Setting lights to:" << DATA << endl;
					//fstream scf;
					//scf.open( scfpath, ios::app);
					//string sensor_data = "OLA setting " + DATA + " RAW data " + aRGB + "\n";
					//scf << sensor_data; //gather Calibration test data
					//scf.close();
					if (sizeof(DATA) == 8)
					{
						cout << "Send to server " << echo << endl; //echo back RGB setting
						resp = send(sockfd, DATA.c_str(),sizeof(DATA), MSG_DONTWAIT|MSG_NOSIGNAL); //Send Response
						if ( resp == -1 && errno == EPIPE )
						{
							close(sockfd);
							connect_to_server();
						}
					}
				}
				else if(CMD == "GET")  //server fetching client sensor values for GUI request
				{
					cout << "Send To Server:" << aRGB << endl;
					if (sizeof(aRGB) == 8)
					{
						resp = send(sockfd, aRGB, sizeof(aRGB), MSG_DONTWAIT|MSG_NOSIGNAL); //Send Sensor data to Server
						if ( resp <= 0 && errno == EPIPE )
						{
							close(sockfd);
							connect_to_server();
						}
					}
				}
				else if(CMD == "PNG")  //echo when server checks that client is still connected
				{
					cout << "Pinging server\n";
					resp = send(sockfd, CMD.c_str(), sizeof(CMD), MSG_DONTWAIT|MSG_NOSIGNAL);
					if ( resp <= 0 && errno == EPIPE )
					{
						close(sockfd);
						connect_to_server();
					}

				}
				else if(CMD == "SHD") //Shut down client
				{
					string shutdown = "0000000"; //set OLA to "00000000" to update DMX driver here BEFORE shutting down client
					cout << "Shutting down\n";
					ola.setData(shutdown);
					shutdown = ola.sendOLA(); // The returned value is based on success
					resp = send(sockfd, shutdown.c_str(),sizeof(shutdown), MSG_DONTWAIT|MSG_NOSIGNAL); //Send Response
					if ( resp <= 0 && errno == EPIPE )
					{
						close(sockfd);
						connect_to_server();
					}
					else
						break;
				}
				else if(CMD == "SUS") //Suspend Client
				{
					string shutdown = "0000000"; //set OLA to "00000000" to update DMX driver and go into sleep mode
					cout << "Sleep Mode\n";
					ola.setData(shutdown);
					shutdown = ola.sendOLA(); // The returned value is based on success
					resp = send(sockfd, shutdown.c_str(),sizeof(shutdown), MSG_DONTWAIT|MSG_NOSIGNAL); //Send Response
					if ( resp <= 0 && errno == EPIPE )
					{
						close(sockfd);
						connect_to_server();
					}
				}
				memset(recv_data, 0, sizeof(recv_data));
				ss.clear();
				CMD.clear();
				DATA.clear();
			}
		}
	}
return 0;
}

int connect_to_server()
{
	if((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) //TCP socket created
	{
		perror("Error: Socket Not created");
		exit(EXIT_FAILURE);
	}
	host = gethostbyname(HOST); //Server Address assignment
	if(host == NULL)
	{
		perror("Host does not exist\n");
		exit(EXIT_FAILURE);
	}
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(int)) < 0)
	{
		perror("setsockopt(SO_REUSEADDR) failed");
		exit(EXIT_FAILURE);
	}
	bzero((char *) &sADDR, sizeof(sADDR));
	sADDR.sin_family = AF_INET;
	bcopy((char *) host->h_addr, (char *) &sADDR.sin_addr.s_addr, host->h_length);
	sADDR.sin_port = htons(port);
	bcopy((char *)host->h_addr,(char *)&sADDR.sin_addr.s_addr,host->h_length);

	cout << "Created Socket\n";

	while(connect(sockfd,(struct sockaddr*)&sADDR, sizeof(sADDR)) < 0)  //connect to server failure, try again
	{
		sleep(1);
		perror("Error: Failed to connect");
	}
	return sockfd;
}

string time_processed() //return time in format YYYY/MM/DD_HH:MM:SS:milliseconds
{
	char buf[40];
	char time_buff[40];
	struct timeval ts;
	time_t curtime;
	gettimeofday(&ts, NULL);
	curtime=ts.tv_sec;
	strftime(time_buff,40,"%Y/%m/%d_%T:",localtime(&curtime));
	sprintf(buf,"[%s%ld]",time_buff, ts.tv_usec);
	return buf;
}

void display_RGB(int s) //gets sensor data every 3 seconds and stores into aRGB variable
{
	// Select enable register(0x80)
	// Power ON, RGBC enable, wait time disable(0x03)
	char config[2] = {0};
	config[0] = 0x80;
	config[1] = 0x03;
	write(file, config, 2);
	// Select ALS time register(0x81)
	config[0] = 0x81;
	config[1] = 0x00;
	write(file, config, 2);
	// Select Wait Time register(0x83)
	// WTIME : 2.4ms(0xFF)
	config[0] = 0x83;
	config[1] = 0xFF;
	write(file, config, 2);
	// Select control register(0x8F)
	config[0] = 0x8F;
	config[1] = 0x00;
	write(file, config, 2);
	usleep(1000000);

	// Read 8 bytes of data from register(0x94)
	// cData lsb, cData msb, red lsb, red msb, green lsb, green msb, blue l$
	char reg[1] = {0x94};
	write(file, reg, 1);
	char data[8] = {0};
	if(read(file, data, 8) != 8)
	{
		cout << "Error: 404 Sensor not found\n";
	}
	else
	{
		// Convert the data
		//int clear = (data[1]);
		int RRaw = ((data[3]<<8)+ data[2]);
		int GRaw = ((data[5]<<8)+ data[4]);
		int BRaw = ((data[7]<<8)+ data[6]);
		//RGB sensor calibration functions
		int RCorrected = (((RRaw - 3183)*205)/25569)+25;
		int GCorrected = (((GRaw - 2820)*205)/24918)+25;
		int BCorrected = (((BRaw - 4050)*205)/37254)+25;

		// Calculate luminance
		int luminance = (-0.32466) * (RCorrected) + (1.57837) * (GCorrected) + (-0.73191) * (BCorrected);

		if(luminance < 0)
		{
			luminance = 0;
		}
		//sprintf(aRGB, "%d %d %d %d", luminance, RCorrected, GCorrected, BCorrected);
		sprintf(aRGB, "%02X%02X%02X%02X", luminance, RCorrected, GCorrected, BCorrected);
		cout << aRGB << endl;
	}
	alarm(ALARM);
	signal(SIGALRM, display_RGB);
}
