/*//////////////////////////////////////////////////////////////////////////
		     Author: Justin Howell
		     Date: April 14th
		     Filename: PingTool.h
		     Cloudflare Internship Project
		     Make sure to run this in sudo!
//////////////////////////////////////////////////////////////////////////*/

#ifndef PINGTOOL_H// include guard
#define PINGTOOL_H

#include <iostream>

#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <netdb.h> 
#include <unistd.h> 
#include <string.h> 
#include <stdlib.h> 
#include <netinet/ip_icmp.h> 
#include <time.h> 
#include <fcntl.h> 
#include <signal.h> 
#include <arpa/inet.h>

#include <cstring>
#include <cstdlib>

using namespace std;

/*//////////////////////////////////////////////////////////////////////////
			Global Variable
//////////////////////////////////////////////////////////////////////////*/

	bool LOOP_FLAG = true; //global for ease of interrupt handler use
	int TTL_GLOBAL = 64; //default value for TTL

/*//////////////////////////////////////////////////////////////////////////
		       Class/Struct Defintions
//////////////////////////////////////////////////////////////////////////*/

struct pingPacket{
	struct icmphdr header;
    	char message[8];
};

class PingTool{
private:
	static int portNumber; //call to port number 0
	static int timeOutTime; //time in seconds until time-out
	static int pingPacketSize; //size of packet
	static int ICMPStructSize; //size of ICMP struct
	static int messageSize;
	static int pingDelay; //time in microseconds until next attempted ping

	int ttl;

	struct sockaddr_in addressCon;
	int addrLength;

	int socketFile;

	char *ipAddress;
	char *reverseLookup;
	char *siteName;

public:
	PingTool(char *);

	char *lookup_dns(char *addressHost, struct sockaddr_in *addressCon);
	char *lookup_reverse(char *ipAddress);

	void ping();

};

#endif /* PINGTOOL_H */
