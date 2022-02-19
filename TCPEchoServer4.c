#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Practical.h"

//These are all the methods used in the file
void createSocket();
void constructServerAddress();
void assignSocketToAddress();
void incomingConnection();
void acceptConnection();
void printClientInfo();
void receiveMessage();
void printMessage();
void sendGreeting();
void sendPhrase();
void sendFinalMessage();

//These are all the variables used in the file
static const int MAXPENDING = 5;
in_port_t servPort;		// This will hold which port the server is using
int servSock;			// Socket descriptor for server
int clntSock;			// Socket descriptor for client
struct sockaddr_in clntAddr;	// Local address; internet socket address structure
char* serverType = "netsvr";	// Assignment asked for these two values to be hardcoded
char* requestType = "type0";	// Assignment asked for these two values to be hardcoded
struct sockaddr_in servAddr;	// Local address; internet socket address structure
char buffer[BUFSIZE];		// Used to temporarily hold incoming messages
char resetBuffer[BUFSIZE];	// Used to clear the buffer before a new one is read
char clientAddress[BUFSIZE];	// The address of the client
size_t clientUsernameLen;	// The length the client's username
ssize_t numBytes;		//Used for error handling


int main(int argc, char *argv[]) 
{
	
	if (argc != 2) // Test for correct number of arguments
		DieWithUserMessage("Parameter(s)", "<Server Port>");
		
	servPort = atoi(argv[1]);	// First arg:  local port ascii to integer
	
	createSocket();		// Create socket for incoming connections
	constructServerAddress();	// Construct local address structure
	
	assignSocketToAddress();	// Cast servaddr as generic socket address structure
	incomingConnection();		// Listen for any incoming connection on the the server's socket

	// Create an infinite loop. The server will always look for new connections after finishing them
	for (;;)
	{ 
		struct sockaddr_in clntAddr;
		acceptConnection();	// Connect to a client	
		receiveMessage();	// Receive a message, placing it into the buffer
		printClientInfo();	// Print information about the connected client to the console
		sendGreeting();	// Send a client a greeting, containing some of it's information
		
		sendPhrase();		// Promt for a message to be sent to the client
		receiveMessage();	// Receive a message, placing it into the buffer
		printMessage();	// Print the contents of the buffer to the console
		sendFinalMessage();	// Send a final message with a cookie before the connection is close
		printf("\n");		// New line for console clarity
	}
	
	// NOT REACHED
}

void createSocket()
{
	// Create socket for incoming connections
	servSock; // Socket descriptor for server
	if ((servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		DieWithSystemMessage("socket() failed");
}

void constructServerAddress()
{
	// Construct local address structure
	memset(&servAddr, 0, sizeof(servAddr));       // Zero out structure
	servAddr.sin_family = AF_INET;                // IPv4 address family
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY); // Any incoming interface; host to network long[integer]
	servAddr.sin_port = htons(servPort);          // Local port; host to network short[integer]
}

void assignSocketToAddress()
{
	//cast servaddr as generic socket address structure
	if (bind(servSock, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0)
		DieWithSystemMessage("bind() failed");
}

void incomingConnection()
{
	// Listen for any incoming connection on the the server's socket
	if (listen(servSock, MAXPENDING) < 0)
		DieWithSystemMessage("listen() failed");
}

void acceptConnection()
{
	// Set length of client address structure (in-out parameter)
	socklen_t clntAddrLen = sizeof(clntAddr);
	clntSock = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
	if (clntSock < 0)
		DieWithSystemMessage("accept() failed");
	// clntSock is connected to a client!	
}

void printClientInfo()
{
	char findClientAddress[INET_ADDRSTRLEN]; // String to contain client address
	
	//Place this data into the clientAddress variable
	for(int i = 0; i < sizeof(findClientAddress); i++)
		clientAddress[i] = findClientAddress[i];
	
	//Conver numeric address into text string suitable for presentation
	if (inet_ntop(AF_INET, &clntAddr.sin_addr.s_addr, clientAddress, sizeof(clientAddress)) != NULL)
		printf("Handling Client: %s %s %s %s%s%d\n", serverType, requestType, buffer, clientAddress,"-", ntohs(clntAddr.sin_port));
	//If none is found, print an error
	else
		puts("Unable to get client address");
}

void printMessage()
{
	printf("Message Received: %s\n", buffer);
}

void receiveMessage()
{
	//Clear the buffer which is still holding the information from the last message
	for(int i = 0; i < BUFSIZE; i++)
		buffer[i] = resetBuffer[i];
		
	//If no message is receive, print and error message
	if(recv(clntSock, buffer, BUFSIZE, 0) < 0)
		printf("Failed to recieve message");
}

void sendGreeting()
{
	char message[100];
	char greeting[] = "hello";
	char port[10];
	char welcome [] = "welcome to the netsvr server";
	int messageLen;
	
	//Concatinating strings to form a greeting message
	strcpy(message, greeting);
	strcat(message, " ");
	strcat(message, clientAddress);
	strcat(message, "-");
	sprintf(port, "%d", ntohs(clntAddr.sin_port));	//This is used because the port is an integer
	strcat(message, port);
	strcat(message, ", ");	
	strcat(message, welcome);	
	strcat(message, "\r\n");

	//printf("%s", message);
	
	messageLen = strlen(message);					// Determine input length. Strlen() finds the length of a string.
	numBytes = send(clntSock, message, strlen(message), 0);	// Send the string to the server
	
	// If no message is found, print an erorr messaage
	if (numBytes < 0)
		DieWithSystemMessage("send() failed");
		
	//If a message is found but has the wrong length, print an error message
	else if (numBytes != messageLen)
		DieWithUserMessage("send()", "sent unexpected number of bytes");	
}

void sendPhrase()
{
	char shortPhrase[50];
	int messageLen;
	
	// Promt the used to input a message then scan for user input
	printf("Send message to %s-%d: ", clientAddress, ntohs(clntAddr.sin_port));
	scanf("%s", shortPhrase);
	
	messageLen = strlen(shortPhrase);	// Determine input length. Strlen() finds the length of a string.
	numBytes = send(clntSock, shortPhrase, strlen(shortPhrase), 0);	// Send the string to the server
	
	// If no message is found, print an erorr messaage
	if (numBytes < 0)
		DieWithSystemMessage("send() failed");
		
	//If a message is found but has the wrong length, print an error message
	else if (numBytes != messageLen)
		DieWithUserMessage("send()", "sent unexpected number of bytes");	
}

void sendFinalMessage()
{
	char finalMessage[100] = "Connection closed with cookie - ";
	char cookie[50];	//Used for verification
	int randomNumber = rand() + 10000000 & 999999999;	//Random number between x and y.
	int messageLen;
			
	//String concatination for final message
	sprintf(cookie, "%d", randomNumber);	// Store the randomNumber value in the cooke variable
	strcat(finalMessage, cookie);	
	strcat(finalMessage, "\r\n");
	
	printf("%s", finalMessage);	// Print the message to the console
	
	messageLen = strlen(finalMessage);	// Determine input length. Strlen() finds the length of a string.
	numBytes = send(clntSock, finalMessage, strlen(finalMessage), 0);	// Send the string to the server
	
	// If no message is found, print an erorr messaage
	if (numBytes < 0)
		DieWithSystemMessage("send() failed");
		
	//If a message is found but has the wrong length, print an error message
	else if (numBytes != messageLen)
		DieWithUserMessage("send()", "sent unexpected number of bytes");	
}
