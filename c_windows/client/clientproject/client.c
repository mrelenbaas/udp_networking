#include <winsock2.h> // https://forum.pellesc.de/index.php?topic=6611.0
#include <windows.h>
//#include <winsock.h>
#include <stdio.h>
#include <stdlib.h>

// https://www.youtube.com/watch?v=Q-CQctL3-xo

#define IP_TARGET "192.168.1.3"

BOOL bEnd = FALSE;

SOCKET MakeSocket( WORD wPort )
{
	SOCKET sock = (SOCKET)NULL;
	SOCKADDR_IN Addr = {0};

	sock = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if( sock == INVALID_SOCKET )
	{
		return (SOCKET)NULL;
	}

	Addr.sin_family = AF_INET;
	Addr.sin_port = htons( wPort );
	Addr.sin_addr.s_addr = inet_addr( IP_TARGET );

	//if( bind( sock, (SOCKADDR *)&Addr, sizeof(Addr) ) == SOCKET_ERROR )
	//{
	//	printf("bind failed with error %u\n", WSAGetLastError());
	//	closesocket( sock );
	//	return (SOCKET)NULL;
	//}

	return sock;
	//return NULL;
}

BOOL SendData( SOCKET sock, WORD wDstPort )
{
	SOCKADDR_IN SendAddr = {0};
	char buf[1024];

	SendAddr.sin_family = AF_INET;
	SendAddr.sin_port = htons( wDstPort );
	SendAddr.sin_addr.s_addr = inet_addr( IP_TARGET );

	printf( "Enter Message: " );
	fgets( buf, 1024, stdin );

	if ( buf[0] == 'q' )
		return FALSE;

	sendto( sock, buf, strlen(buf), 0, (SOCKADDR *)&SendAddr, sizeof(SendAddr) );

	return TRUE;
}

DWORD WINAPI RecvThread( LPVOID pParam )
{
	SOCKET sock = (SOCKET)pParam;
	SOCKADDR_IN RecvAddr = {0};
	int iRet, iRecvSize;
	char buf[1024];

	while( !bEnd )
	{
		iRecvSize = sizeof(RecvAddr);
		iRet = recvfrom( sock, buf, 1024, 0, (SOCKADDR *)&RecvAddr, &iRecvSize );

		if( iRet == SOCKET_ERROR )
			continue;

		buf[iRet] = '\0';
		printf("[%s:%d] : %s", inet_ntoa( RecvAddr.sin_addr ), htons( RecvAddr.sin_port ), buf );
	}

	printf( "Recv Thread End\n" );

	return 0;
}

int main( int argc, char** argv )
{
	WSADATA wsaData = {0};
	SOCKET sock;
	WORD wSrcPort, wDstPort;

	wSrcPort = (WORD)atoi( argv[1] );
	wDstPort = (WORD)atoi( argv[2] );

	//wSrcPort = (WORD)atoi("1234");
	//wDstPort = (WORD)atoi("1234");

	WSAStartup( MAKEWORD(2,2), &wsaData );

	sock = MakeSocket( wSrcPort );

	if( sock )
	{
		HANDLE hThread = CreateThread( NULL, 0, RecvThread, (PVOID)sock, 0, NULL );

		while(1)
		{
			if( !SendData( sock, wDstPort ) )
				break;
		}

		bEnd = TRUE;
		closesocket( sock );

		if( WaitForSingleObject( hThread, 3000 ) == WAIT_TIMEOUT )
		{
			printf( "Error: Thread End\n" );
			TerminateThread( hThread, 0 );
		}
	}
	else
	{
		printf( "ERROR: Socket was not make correctly.\n" );
	}

	WSACleanup();
	printf( "Complete.\n" );
	return 0;
}
