/*//////////////////////////////////////////////////////////////////////////
		     Author: Justin Howell
		     Date: April 14th
		     Filename: PingTool.cpp
		     Cloudflare Internship Project
		     Make sure to run this in sudo!
//////////////////////////////////////////////////////////////////////////*/

#include "PingTool.h"

using namespace std;

/*//////////////////////////////////////////////////////////////////////////
		     Utility (Structs and Utility Functions)
//////////////////////////////////////////////////////////////////////////*/

void intHandler(int x){
	LOOP_FLAG = false;
}

unsigned short checksum(void *inputBuffer, int length){
	unsigned short *buffer = (unsigned short*)inputBuffer; 
	unsigned int sum = 0;
	unsigned short result = 0;

	for (sum = 0; length > 1; length -= 2){
		sum += *buffer++;
	}
	if(length == 1){
		sum += *(unsigned char*)buffer;
	}
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	result = ~sum;
	return result;
}

/*//////////////////////////////////////////////////////////////////////////
				Constructors
//////////////////////////////////////////////////////////////////////////*/

	int PingTool::portNumber = 0; //call to port number 0
	int PingTool::timeOutTime = 1; //time in seconds until time-out
	int PingTool::pingPacketSize = 64; //size of packet
	int PingTool::ICMPStructSize = 56; //size of ICMP struct
	int PingTool::messageSize = 8;
	int PingTool::pingDelay = 2000000; //time in microseconds until next attempted ping

PingTool::PingTool(char *inputArg){

		ttl = TTL_GLOBAL;

		siteName = inputArg;
		ipAddress = lookup_dns(inputArg, &addressCon);

		cout << "Address found: " << ipAddress << endl;

		reverseLookup = lookup_reverse(ipAddress);

		cout << "Connecting to " << siteName << "..." << endl;
		cout << "Reverse lookup found: " << reverseLookup << endl;

		socketFile = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
		if(socketFile < 0){
			cout << "Socket file failed to be created!" << endl;
			return;
		}

		cout << endl;
}

/*//////////////////////////////////////////////////////////////////////////
				Ping Function
//////////////////////////////////////////////////////////////////////////*/

void PingTool::ping(){

	//variables created for each pinging session

	int messageCount = 0, packetLoss;

	int counter;

	unsigned int addressLength = 0;

	struct sockaddr_in *pingAddress = &addressCon;

    	struct pingPacket packet; 

	struct sockaddr_in returnAddress;

	//time variables

	struct timespec timeStart, timeEnd, tfs, tfe;
	long double rttMsec = 0, totalMsec = 0;

	struct timeval timeOut;
	timeOut.tv_sec = timeOutTime;
	timeOut.tv_usec = 0;

	//time set

	clock_gettime(CLOCK_MONOTONIC, &tfs);

	if (setsockopt(socketFile, SOL_IP, IP_TTL, &ttl, sizeof(ttl)) != 0)
	{
		cout << "Setting socket options to current time-to-live value failed!" << endl;
		cout << "This can happen because you're not running sudo!" << endl;
		return;
	}

	if (setsockopt(socketFile, SOL_SOCKET, SO_RCVTIMEO, (const char*) &timeOut, 					sizeof timeOut) != 0){
		cout << "Setting socket options for timeout time failed" << endl;
		return;
	}

	while(LOOP_FLAG){

		bzero(&packet, sizeof(packet));

		packet.header.type = ICMP_ECHO;
		packet.header.un.echo.id = getpid();

		for (counter = 0; counter < sizeof(packet.message) - 1; counter++){
			packet.message[counter] = counter + '0';
		}

		packet.message[counter] = 0;
		packet.header.un.echo.sequence = messageCount++;
		packet.header.checksum = checksum(&packet, sizeof(packet));


		usleep(pingDelay);

		clock_gettime(CLOCK_MONOTONIC, &timeStart);

		
		if(sendto(socketFile, &packet, sizeof(packet), 0,
					(struct sockaddr*) pingAddress,
						sizeof(*pingAddress)) <= 0){
			cout << "Packet failed to send!" << endl;
		}



		addressLength = sizeof(returnAddress);

		ssize_t test = recvfrom(socketFile, &packet, sizeof(packet), 0,  
             			(struct sockaddr*)&returnAddress, &addressLength);

 		if (  test <= 0 && messageCount > 1)  {
			cout << "Packet failed to be received!" << endl;
			packetLoss++;
		}else{

			clock_gettime(CLOCK_MONOTONIC, &timeEnd);

 			double timePassed = ((double)(timeEnd.tv_nsec -  
                                		timeStart.tv_nsec))/1000000.0;
			rttMsec = (timeEnd.tv_sec- 
					timeStart.tv_sec) * 1000.0 + timePassed; 


			if(!(packet.header.type == 69 && packet.header.code == 0)){
					cout << "ERROR: ICMP type: " <<
						packet.header.type << "code: " <<
							packet.header.code << endl;
			}else{
					cout << "Pinging: " << siteName << "." << endl;
					cout << "Ip: " << ipAddress << endl;
					cout << "Ping #: " << messageCount << endl;
					cout << "RTT Time: " << rttMsec << endl;
					cout << "Total Packets Lost: " << packetLoss
									 << endl;

					cout << endl << endl;
			}
		}
	}

	clock_gettime(CLOCK_MONOTONIC, &tfe);
	double timePassed = ((double)(tfe.tv_nsec - tfs.tv_nsec))/1000000.0;
	
	totalMsec = (tfe.tv_sec - tfs.tv_sec) * 1000.0 + timePassed;

	

}

/*//////////////////////////////////////////////////////////////////////////
				Lookup Reverse reverseLookup
//////////////////////////////////////////////////////////////////////////*/

char * PingTool::lookup_reverse(char *ipAddress){
	
	struct sockaddr_in temp;

	socklen_t length;

	char buffer[NI_MAXHOST], *returnBuffer;

	temp.sin_family = AF_INET;
	temp.sin_addr.s_addr = inet_addr(ipAddress);
	length = sizeof(struct sockaddr_in);

	if(getnameinfo((struct sockaddr *) &temp, length, buffer,
						 sizeof(buffer), NULL, 0, NI_NAMEREQD)){
		cout << "Could not get reverse host name!" << endl;
		return NULL;
	}
	returnBuffer = new char[(strlen(buffer) + 1)*sizeof(char)];

	strcpy(returnBuffer, buffer);

	return returnBuffer;

}

//////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

char * PingTool::lookup_dns(char *addressHost, struct sockaddr_in *addressCon){

	cout << "Looking up DNS:" << endl;

	struct hostent *hostEntity;

	char *ip = new char[NI_MAXHOST*sizeof(char)];


	if((hostEntity = gethostbyname(addressHost)) == NULL){
		cout << "Could not get DNS name!" << endl;
		return NULL;
	}	

	strcpy(ip, inet_ntoa(*(struct in_addr *) hostEntity->h_addr));
	
	(*addressCon).sin_family = hostEntity->	h_addrtype;
	(*addressCon).sin_port = htons(portNumber);
	(*addressCon).sin_addr.s_addr = *(long*)hostEntity->h_addr;

	return ip;
}

/*//////////////////////////////////////////////////////////////////////////
				Main Function
//////////////////////////////////////////////////////////////////////////*/

int main(int argc, char *argv[]){

	switch(argc){
		case 2:
			//do nothing, file name and url/ip address given
			break;
		case 3: 
			//TTL argument given
			TTL_GLOBAL = atoi(argv[2]);
			break;
		default:
			cout << "Correct formats: " << endl;
			cout << " ./.. 'url/ip address' " << endl;
			cout << " ./.. 'url/ip' 'ttl number' " << endl;
			return 0;
	}

	cout << "//////////////////////////////////" << endl;
	cout << "	Author: Justin Howell      " << endl;
	cout << "   Cloudflare Intership Project   " << endl;
	cout << "//////////////////////////////////" << endl;
	cout << "//////////////////////////////////" << endl;
	cout << endl;
	cout << "This code was tested using sudo,  " << endl;
	cout << "and pings a URL or IPv4 address.  " << endl;
	cout << "You can interrupt the pinging by  " << endl;
	cout << "Pressing Control-C.               " << endl;
	cout << endl << "This program supports setting" << endl;
	cout << "the TTL through a second command  " << endl;
	cout << "line argument!" << endl;
	cout << endl;
	cout << "//////////////////////////////////" << endl;
	cout << "//////////////////////////////////" << endl;
	
	if(argc == 3){
	cout << endl << "TTL set to " << TTL_GLOBAL << " from command line." << endl;
	cout << endl;	
	}

	cout << "Starting now: " << endl;
	usleep(2500000);

	PingTool pinger(argv[1]);

	signal(SIGINT, intHandler);

	pinger.ping();

	return 0;

}
