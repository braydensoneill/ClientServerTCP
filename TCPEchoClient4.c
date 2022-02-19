#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Practical.h"

// These are the methods used in this file
void createSocket();
void constructServerAddress();
void convertAddress();
void establishConnection();
void sendData();
void reverseString(char *string);
void sendPhraseReversed();
void receiveMessage();
void receiveFinalMessage();

//These are the variables used in this file
char *clientUsername;		// The username of the client
char *servIP;			// The Ip address of the server
in_port_t servPort;		// The port which the server is using
int sock;			// Socket descriptor for client
struct sockaddr_in servAddr;	// Local address; internet socket address structure
size_t clientUsernameLen;	// The length of the client's username
ssize_t numBytes;		// The length of the message
char reversedString[50];	// A string holding the values of a recieved message, but reversed
char buffer[BUFSIZE];		// Used to temporarily hold incoming messages
char resetBuffer[BUFSIZE];	// Used to clear the buffer before a new one is read
	
int main(int argc, char *argv[]) 
{
	if (argc < 3 || argc > 4)	// Test for correct number of arguments
		DieWithUserMessage("Parameter(s)", "<Client username> <Server Address> [<Server Port>]");
		
	clientUsername = argv[1];			// First arg: username of client
 	servIP = argv[2];				// Second arg: server IP address (dotted quad)
 	servPort = (argc == 4) ? atoi(argv[3]) : 7;	// Third arg (optional): server port.  7 is well-known echo port

	createSocket();		// Create a reliable, stream socket using TCP
	constructServerAddress();	// Construct the server address structure
	convertAddress();	// Convert address
	establishConnection();	// Establish the connection to the server
	
	sendData();		// Send the data to the server
	
	receiveMessage();	// Receive message from the server
	receiveMessage();	// Receive message from the server
	printf("\n");
	sendPhraseReversed();	// Reverse the message and send it to the server
	receiveFinalMessage();	// Receive on last message before the socket is closed
	close(sock);		//Close the socket	
	exit(0);
}

void createSocket()
{
	// Create a reliable, stream socket using TCP
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);	
	if (sock < 0)
		DieWithSystemMessage("socket() failed");
}

void constructServerAddress()
{
	// Construct the server address structure
	memset(&servAddr, 0, sizeof(servAddr));	// Zero out structure
	servAddr.sin_family = AF_INET;		// IPv4 address family
}

void convertAddress()
{
	// Convert address
	int rtnVal = inet_pton(AF_INET, servIP, &servAddr.sin_addr.s_addr);
	if (rtnVal == 0)
		DieWithUserMessage("inet_pton() failed", "invalid address string");
	else if (rtnVal < 0)
		DieWithSystemMessage("inet_pton() failed");
	servAddr.sin_port = htons(servPort);	// Server port
}

void establishConnection()
{
	// Establish the connection to the server
	if (connect(sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
		DieWithSystemMessage("connect() failed");
}

void sendData()
{
	// Send the data to the server
	clientUsernameLen = strlen(clientUsername);			// Determine input length. Strlen() finds the length of a string.
	numBytes = send(sock, clientUsername, clientUsernameLen, 0);	// Send the string to the server
	
	// If no message is found, print an erorr messaage
	if (numBytes < 0)
		DieWithSystemMessage("send() failed");
		
	//If a message is found but has the wrong length, print an error message
	else if (numBytes != clientUsernameLen)
		DieWithUserMessage("send()", "sent unexpected number of bytes");
}

void receiveMessage()
{
	//Clear the buffer which is still holding the information from the last message
	for(int i = 0; i < BUFSIZE; i++)
		buffer[i] = resetBuffer[i];
		
	//If no message is receive, print and error message
	if(recv(sock, buffer, BUFSIZE, 0) < 0)
		printf("Failed to read message");
		
	//Print the message received to the the console
	else
		printf("Message Recieved: %s", buffer);
}

void sendPhraseReversed()
{
	int messageLen; 
	
	reverseString(buffer);	//Call a method to reverse the contents of the current buffer
	
	printf("Sending Reversed Message: %s\n", buffer);	//Print the reversesd string to the console
	
	messageLen = strlen(buffer);	// Determine input length. Strlen() finds the length of a string.
	numBytes = send(sock, buffer, strlen(buffer), 0);	// Send the string to the server
	
	// If no message is found, print an erorr messaage
	if (numBytes < 0)
		DieWithSystemMessage("send() failed");
		
	//If a message is found but has the wrong length, print an error message
	else if (numBytes != clientUsernameLen)
		DieWithUserMessage("send()", "sent unexpected number of bytes");
	
}

//Reversing a string. I got help with this method from stack overflow
void reverseString(char *string)
{
	int stringLen = strlen(string); //Get the length of the string
	
	//Iterate through the string, while another variable iterates backwards
	for(int i = 0, j = stringLen - 1; i < j; i++, j--)
	{
		char ch = string[i];	//Get the index of current letter
		string[i] = string[j];	//Do an ol' switcheroo
		string[j] = ch;	//Set the index contents of the right index  to the first index
	}
		
}

void receiveFinalMessage()
{
	//Clear the buffer which is still holding the information from the last message
	for(int i = 0; i < BUFSIZE; i++)
		buffer[i] = resetBuffer[i];
		
	//If no message is receive, print and error message
	if(recv(sock, buffer, BUFSIZE, 0) < 0)
		printf("Failed to read message");
		
	//Print the message received to the the console
	else
		printf("%s", buffer);
}





