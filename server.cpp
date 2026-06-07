



#define _CRT_SECURE_NO_WARNINGS
// We need to create a field to read headers so the client code knows what kind of payload it is.
// and how many bytes follow that header
#include <stdint.h> // gives fixed-size integer types. used for protocol headers
// uint32_t = 32-bit unsigned
// uint64_t = 64-bit unsigned 
#include <inttypes.h> // lets me print 64 bit numbers safely 
// The second biggest change: stop assuming one recv == one full thing
#include <stdlib.h> // gives access to malloc/free




#define _WIN32_WINNT 0x0601


#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <cstdio>
#include <string>

#pragma comment(lib, "Ws2_32.lib")



#define DEFAULT_PORT "9090"
#define DEFAULT_BUF_LEN 512
#define MAX_FILENAME 256





// helper function for recv
// we need this helper becasye recv only reads x amount of bytes at a time, so to read thousands of bytes
// we need to loop 

// tcp is also a stream, not message-based delivery which mean recv is not guarenteed to get all the bytes in 1 call
// therefore we need to call it multiple times
// recv doesnt have a set amount of bytes it can return at once, it all depends on network conditions, OS/socket buffering, etc
int recv_exact(SOCKET s, char* buf, int len) {
	int total = 0;
	while (total < len) {
		int r = recv(s, buf + total, len - total, 0);
		// writes to buf, the + total just makes it so we dont overwrite what we already wrote to 
		// the buffer since it's in a while loop
		if (r == 0) return 0;              // client disconnected
		if (r == SOCKET_ERROR) return SOCKET_ERROR;
		total += r;
	}
	return total;
}



// Custom packet header
struct fileHeader {

	uint32_t protocol; // this is going to validate whether or not the incoming packet is using my pre defined protocol
	uint32_t type;
	uint64_t size;
	char filename[MAX_FILENAME];

};












int main() {

	


	// initialization

	struct addrinfo* result = nullptr, * ptr = nullptr, hints{}; 
	WSADATA wsadata;
	WORD version;
	version = MAKEWORD(2, 2);



	int startup_result = WSAStartup(version, &wsadata);
	printf("WSAStartup Result: %d\n", startup_result);
	if (startup_result != 0) {

		
		return 1;
	}




	// preparing the server's local address infoormation
	SOCKET listening_socket = INVALID_SOCKET; 

	hints.ai_family = AF_INET; // ipv4 or ipv6
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	int my_result = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result); // function fills results with data it's gotten
	if (my_result != 0) {
		printf("Error: %d\n", my_result);
		printf("getaddrinfo failed: %d %s\n", my_result, gai_strerrorA(my_result));
		WSACleanup();
		return 1;

	}
	else if (my_result == 0) {
		printf("Address resolution suceeded\n");
	}



	// creating the socket
	listening_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (listening_socket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}








	// binding the created socket

	int bind_result = bind(listening_socket, result->ai_addr, (int)result->ai_addrlen);
	if (bind_result == 0) {
		printf("Socket successfully bound: %d\n", bind_result);

	}
	else {
		int error = WSAGetLastError();
		printf("Error Socket could not bind: %d\n", error);
		closesocket(listening_socket); 
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result); 






	if (listen(listening_socket, SOMAXCONN) == SOCKET_ERROR) {
		printf("Socket listening error: %ld\n", WSAGetLastError());
		closesocket(listening_socket);
		WSACleanup();
		return 1;
	}

	printf("Listening...\n");







	

	SOCKET clientSocket;
	sockaddr_in client; // no sockaddr because this is ipv4, sockaddr is generic, sockaddr_in is specific
	int q = sizeof(client);
	char convertedAddress[16];
	int convertedLen = sizeof(convertedAddress);



	clientSocket = accept(listening_socket, (sockaddr*)&client, &q);





	if (clientSocket == INVALID_SOCKET) {
		printf("Error accepting: %d", WSAGetLastError());
		closesocket(listening_socket);
		WSACleanup();
		return 1;
	}
	else {
		int binary_ip = getpeername(clientSocket, (sockaddr*)&client, &q);
		printf("binary_ip: %d\n", binary_ip);
		inet_ntop(AF_INET, &client.sin_addr, convertedAddress, convertedLen);



		printf("strlen: %d\n", strlen(convertedAddress));
		printf("Device connected successfully\n");

		// convertedAddress == "192.168.2.147"        doesnt work because convertedAddress decays into a pointer to the first
		// element &convertedAddress[0] and "192.168.2.147" decays into a pointer 
		if (strcmp(convertedAddress, "192.168.2.147") == 0) {
			printf("Keshawn's laptop connected: %s\n", convertedAddress);
		}
		else {
			printf("Unknown device connected. ip = %s\n", convertedAddress);
		}
	}














	int iResult;
	bool continueAccepting = true;
	int filesReceived = 0;
	while (continueAccepting) {








		// Block 1 creates an instance of the fileHeader structure and reads the data from the incoming header into the instance
		fileHeader incomingHeader;


		// type cast the address of incoming header &incomingheader to char so C++ treeats this buffer as a char buffer
		// we change it to char* so we can just read all the incoming bytes into the memory layout
		// after the bytes are read into the layout, the structure keeps its original form with uin32, uint64, char etc
		// and then we can interprate the bytes based on that once they've been read in.
		int incomingHeaderResult = recv_exact(clientSocket, (char*)&incomingHeader, sizeof(fileHeader));

		if (incomingHeaderResult == SOCKET_ERROR) {
			printf("Connection Error!!!\n");
			break;
		}

		else if (incomingHeaderResult == 0) {
			printf("Client disconnected\n");
			break;
		}

		


		else {

			if (incomingHeader.protocol != 42) {
				printf("User is not running protocol KO\n");
				break;
			}
			else {
				printf("Protocol accepted\n");

				if (incomingHeader.type == 1) {
					printf("The user has sent a message\n");
					printf("Header Data\n");
					printf("Protocol %d\n", incomingHeader.protocol);
					printf("Type %d\n", incomingHeader.type);
					printf("Size %llu\n", (unsigned long long)incomingHeader.size);
					printf("File name %s\n", incomingHeader.filename);
				}

				else if (incomingHeader.type == 2) {
					printf("\n\nThe user has sent a file\n\n");
					printf("Header Data\n");
					printf("Protocol %d\n", incomingHeader.protocol);
					printf("Type %d\n", incomingHeader.type);
					printf("Size %llu\n", (unsigned long long)incomingHeader.size);
					printf("File name %s\n\n", incomingHeader.filename);




					// recv_exact will read in the right amount of butes because readIncomingBytesBuffer is set to the size of 
					// the file being transfered in the payload header
					std::vector<char> readIncomingBytesBuffer(incomingHeader.size);
					iResult = recv_exact(clientSocket, (char*)readIncomingBytesBuffer.data(), (int)readIncomingBytesBuffer.size());

					if (iResult == SOCKET_ERROR) {
						printf("Payload receive failed...\n");
						break;
					}
					else if (iResult == 0) {
						printf("Client disconnected during payload transmission...\n");
						break;
					}
					else {


						// just for printing the raw bytes so it looks cool :D
						printf("Content being written to disk: ");
						for (size_t i = 0; i < readIncomingBytesBuffer.size(); i++) {
							printf("[%c] ", readIncomingBytesBuffer.data()[i]);
						}

						FILE* writeToDisk = fopen(incomingHeader.filename, "wb");
						if (writeToDisk != NULL) {
							auto byteCheck = fwrite(readIncomingBytesBuffer.data(), sizeof(char), readIncomingBytesBuffer.size(), writeToDisk);
							if (byteCheck != (size_t)incomingHeader.size) {
								printf("File corrupted in transmission or disk write\n");
								fclose(writeToDisk);
								break;
							}
							fclose(writeToDisk);
						}
						else {
							printf("Could not open file!\n");
							break;
						}



						printf("\n\nFile successfully received\n");
						filesReceived++;
					}
				}


				else if (incomingHeader.type == 3) {
					printf("The user has terminated the session\n");
					break;
				}

				else {
					printf("The user has sent an unknown packet type\n");
				}
			}
		}
		if (filesReceived >= 1) {
			printf("Waiting for next file...\n");
		}
	}


	printf("Session terminated\n");
	closesocket(clientSocket);
	closesocket(listening_socket);
	WSACleanup();






	return 0;

}
