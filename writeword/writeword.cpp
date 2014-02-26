#pragma once
#include "StdAfx.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include "PreRobotMove.h"
#pragma comment(lib, "Wsock32.lib")
using namespace std;
#define  GROUP_IP	 "239.255.0.1"
#define  PORT		60001
#define  LOCAL_IP "192.168.1.23"
int dataGramIndex=0;
PreRobotMove prm;
DWORD WINAPI recvThread(LPVOID lp);
void sendData(); 
int threadsCount = 1;
HANDLE sThread; 

typedef struct
{
	int id;
	int number;
	double data[24];
	bool judge[2];
}EACHLEAP_DATA;

typedef struct
{
	double angledata[6];

}PRIMESCENCE_DATA;
double previousdata[3];
EACHLEAP_DATA leapData;
PRIMESCENCE_DATA angleDate;
ANGLE_DATA angle;

int index=0;

DWORD WINAPI recvThread(LPVOID lp)
{

	WORD VersionRequested;
	WSADATA WsaData;
	VersionRequested=MAKEWORD(2,2);
	WSAStartup(VersionRequested,&WsaData); 

	struct sockaddr_in localSock;
	struct ip_mreq group;


	int sd = socket(AF_INET, SOCK_DGRAM, 0);

	int reuse = 1;
	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));


	memset((char *) &localSock, 0, sizeof(localSock));
	localSock.sin_family = AF_INET;
	localSock.sin_port = htons(PORT);
	localSock.sin_addr.s_addr = INADDR_ANY;

	::bind(sd, (struct sockaddr*)&localSock, sizeof(localSock));

	group.imr_multiaddr.s_addr = inet_addr(GROUP_IP);
	group.imr_interface.s_addr = inet_addr(LOCAL_IP);
	setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group));

	struct sockaddr_in from;
	int fromlen = sizeof(sockaddr_in); 
	int nRecvLen = 1;
	char recvbuf[208];

	int a;

	while(1)
	{
		if(recvfrom(sd, (char*)recvbuf, 208, 0, (struct sockaddr*)&from, (socklen_t*)&fromlen))
		{
			index++;
		}

		memcpy(&leapData,recvbuf,sizeof(leapData));

		
	    sendData();
	}

	closesocket(sd);
	WSACleanup();

	return 0;
}


void sendData()
{	
	WSADATA data; 
	WSAStartup(MAKEWORD(2,0),&data);
	SOCKET s;
	s=socket(AF_INET,SOCK_DGRAM,0);
	sockaddr_in addr;
	addr.sin_family=AF_INET;
	addr.sin_port=htons(9999);
	addr.sin_addr.S_un.S_addr=inet_addr("127.0.0.1");
	char buf[48];
	double diff[3];
	   if(index==1){
		previousdata[0]=leapData.data[21];
		previousdata[1]=leapData.data[22];
		previousdata[2]=leapData.data[23];
	   }else{
		diff[0]=leapData.data[21]-previousdata[0];
		diff[1]=leapData.data[22]-previousdata[1];
		diff[2]=leapData.data[23]-previousdata[2];

		angle.angle[0]=-diff[0];
		angle.angle[2]=diff[1];
		angle.angle[1]=-diff[2];
		prm.TraverseCalNormal(angle);
		previousdata[0]=leapData.data[21];
		previousdata[1]=leapData.data[22];
		previousdata[2]=leapData.data[23];

	   
	angleDate.angledata[0]=(prm.Joint[0]-90)/180*3.1415926;
	angleDate.angledata[1]=prm.Joint[1]/180*3.1415926;
	angleDate.angledata[2]=prm.Joint[2]/180*3.1415926;
	angleDate.angledata[3]=prm.Joint[3]/180*3.1415926;
	angleDate.angledata[4]=prm.Joint[4]/180*3.1415926;
	angleDate.angledata[5]=prm.Joint[5]/180*3.1415926;

	memcpy(buf,&angleDate,sizeof(angleDate));
	sendto(s,buf,sizeof(buf),0, (struct sockaddr*)&addr, sizeof(addr)); 

	   }


	closesocket(s);

}

int main()
{  
	if(threadsCount == 1)
	{
		sThread=CreateThread(NULL,0,recvThread,NULL,0,NULL);
		threadsCount++;
	}
	WaitForSingleObject(sThread,INFINITE);
	return 1;
}

