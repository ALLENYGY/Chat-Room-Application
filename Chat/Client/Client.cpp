#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <thread>
#include <cstring>

#pragma comment(lib, "ws2_32.lib")
#define DEFAULT_PORT	5019

using namespace std;

string sendOperation();
string renameOperation();
string createGroupOperation(SOCKET sock);
string addMemberOperation();
string deleteMemberOperation();
string addnewMemberOperation(string groupname);
string deletenewMemberOperation(string groupname);
string showRoomMember();
string sendGroupOperation();
void printWechat();
void hintMessage();
void sendData(SOCKET sock);
void receiveData(SOCKET sock);
void sendToServer(SOCKET sock, string sendBuff);
bool checknumber(string num);
string username;
bool flag = true;

int main(int argc, char** argv) {
	WSADATA wsaData;
	SOCKET connect_sock;
	struct sockaddr_in server_addr;
	char* server_name = "localhost";
	unsigned short port = DEFAULT_PORT;

	if (WSAStartup(0x202, &wsaData) == SOCKET_ERROR) {
		fprintf(stderr, "WSAStartup failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Assume server is running on localhost

	connect_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (connect_sock == INVALID_SOCKET) {
		fprintf(stderr, "socket() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	if (connect(connect_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
		fprintf(stderr, "connect() failed with error %d\n", WSAGetLastError());
		closesocket(connect_sock);
		WSACleanup();
		return -1;
	}

	printf("Connected to server: %s:%d successfully!\n", server_name, port);
	printWechat();
	printf("Type 'E' to close the connection\n");
	string command=renameOperation();
	sendToServer(connect_sock, command);
	hintMessage();
	// Start the threads for sending and receiving data
	thread sendThread(sendData, connect_sock);
	thread recvThread(receiveData, connect_sock);
	// Wait for the send thread to finish before closing
	sendThread.join();
	recvThread.detach();  // Receive thread can be detached as it will exit on server close
	// Cleanup
	closesocket(connect_sock);
	WSACleanup();
	return 0;
}


void hintMessage() {
	printf("------------------------------------------------------------------\n");
	cout << "Hello," << username << "! Welcome to Chat Room!\n";
	printf("------------------------------------------------------------------\n");
	printf("Type 'R' to Rename yourself\n");						// Rename username
	printf("Type 'S' to Send messages to a specific member\n");		// Send messages to a specific member
	printf("Type 'E' to Exit\n");									// Quit the Chat Room
	printf("Type 'L' to List all members in the chat room\n");		// List all members in the chat room
	printf("Type 'C' to Create a new group\n");						// Create a new group
	printf("Type 'A' to Add a member to the group\n");				// Add a member to the group
	printf("Type 'D' to Delete a member from the group\n");			// Delete a member from the group
	printf("Type 'G' to Send messages to the group\n");				// Send messages to the group
	printf("------------------------------------------------------------------\n");
}

string sendOperation() {
	string prefix = "S";
	cout << "Enter the ID of the member you want to send a message to: \n";
	string id; cin >> id;
	if (!checknumber(id)) {
		printf("Invalid ID.\n");
		return "";
	}
	cout << "Enter the message you want to send:\n";
	string msg; cin.ignore();
	getline(cin,msg);
	string command = prefix + "*" + id + "*" + msg + "*\0";
	return command;
}

string sendGroupOperation() {
	string prefix = "G";
	cout << "Enter the Group Name: \n";
	string gname; cin >> gname;
	cout << "Enter the message you want to send:\n";
	string msg; cin.ignore();
	getline(cin, msg);
	string command = prefix + "*" + gname + "*" + msg + "*\0";
	return command;
}

string renameOperation() {
	string prefix = "R";
	printf("Enter your new name: ");
	cin >> username;
	string command = prefix + "*" + username + "*\0";
	return command;
}

string createGroupOperation(SOCKET sock) {
	string prefix = "C";
	printf("Enter your Group name: ");
	string groupname;
	cin >> groupname;
	string command = prefix + "*" + groupname+"*\0";
	sendToServer(sock, command);
	printf("Here is the member in the Chat room that You can invite!\n");
	sendToServer(sock, showRoomMember());
	sendToServer(sock, addnewMemberOperation(groupname));
	printf("Group %s has been created.\n", groupname.c_str());
	return command;
}

string addMemberOperation() {
	printf("Enter the Group name:\n");
	string groupname;
	cin >> groupname;
	string command= addnewMemberOperation(groupname);
	return command;
}

string addnewMemberOperation(string groupname) {
	string prefix = "A";
	prefix += "*" + groupname + "*";
	printf("Enter the UserID that you want to invite: (end with #)\n");
	string id;
	int count = 0;
	string command = prefix + "*";
	cin >> id;
	if (!checknumber(id)) {
		printf("Invalid ID.\n");
		return "";
	}
	while (id != "#") {
		command = command + id + "*";
		count++;
		cin >> id;
		if (!checknumber(id) && id!="#") {
			printf("Invalid ID.\n");
			return "";
		}
	}
	command += "*" + to_string(count) + "*\0";
	return command;
}

string deleteMemberOperation() {
	printf("Enter the Group name:\n");
	string groupname;
	cin >> groupname;
	string command = deletenewMemberOperation(groupname);
	return command;
}


string deletenewMemberOperation(string groupname) {
	string prefix = "D";
	prefix += "*"+ groupname + "*";
	printf("Enter the UserID that you want to delete: (end with #)\n");
	string id;
	int count = 0;
	string command = prefix + "*";
	cin>> id;
	if (!checknumber(id)) {
		printf("Invalid ID.\n");
		return "";
	}
	while (id != "#") {
		command = command + id + "*";
		count++;
		cin >> id;
		if (!checknumber(id) && id != "#") {
			printf("Invalid ID.\n");
			return "";
		}
	}	
	command += "*" + to_string(count) + "*\0";
	return command;
}

string showRoomMember() {
	string prefix = "L";
	string command = prefix + "\0";
	return command;
}

void sendToServer(SOCKET sock,string sendBuff) {
	int bytesSent;
	bytesSent = send(sock, sendBuff.c_str(), sendBuff.length(), 0);
	if (bytesSent == SOCKET_ERROR) {
		fprintf(stderr, "send() failed with error %d\n", WSAGetLastError());
		return;
	}

}

void sendData(SOCKET sock) {
	string sendBuff;
	while (true) {
		printf("Select your operation: Type 'H' to get help.\n");
		char command;
		cin >> command;
		switch (command) {
		case 'H':
			hintMessage();
			break;
		case 'R':
			sendBuff = renameOperation();
			if(sendBuff!="")
				sendToServer(sock, sendBuff);
			break;
		case 'S':
			sendBuff = sendOperation();
			if (sendBuff != "")
				sendToServer(sock, sendBuff);
			break;
		case 'L':
			sendBuff = "L\0";
			sendToServer(sock, sendBuff);
			break;
		case 'E':
			sendBuff = "E\0";
			sendToServer(sock, sendBuff);
			cout << "Bye!" << endl; flag = false;
			return;
		case 'C':
			sendBuff = createGroupOperation(sock);
			break;
		case 'A':
			if (sendBuff != "")
				sendBuff = addMemberOperation();
			sendToServer(sock, sendBuff);
			break;
		case 'D':
			if (sendBuff != "")
				sendBuff = deleteMemberOperation();
			sendToServer(sock, sendBuff);
			break;
		case 'G':
			sendBuff = sendGroupOperation();
			sendToServer(sock, sendBuff);
			break;
		default:
			printf("Invalid command.\n");
			hintMessage();
			break;
		}
	}
}

void receiveData(SOCKET sock) {
	char recvBuff[100];
	int bytesReceived;
	while (true) {
		memset(recvBuff, 0, sizeof(recvBuff));
		bytesReceived = recv(sock, recvBuff, sizeof(recvBuff), 0);
		if (bytesReceived > 0) {
			printf("%s\n", recvBuff);
		}
		else if (bytesReceived == 0) {
			printf("Server closed the connection\n");
			break;
		}
		else {
			if (flag) {
				fprintf(stderr, "recv() failed with error %d\n", WSAGetLastError());
				break;
			}
		}
	}
}

bool checknumber(string num) {
	for (int i = 0; i < num.length(); i++) {
		if (num[i] < '0' || num[i] > '9') {
			return false;
		}
	}
	return true;
}
void printWechat(){
    printf("                 ,EEEEEEi                         \n");
    printf("               EEEEt..tEEEE                       \n");
    printf("             LEE          GEE                     \n");
    printf("            EE              EE                    \n");
    printf("           EE                EE                   \n");
    printf("          GE     G     tj     EE                  \n");
    printf("          E     EEE    EEG     E                  \n");
    printf("         iE     EE.    EE      EL                 \n");
    printf("         EE                    LE                 \n");
    printf("         Ei                  .LEEi                \n");
    printf("         Et               iEEEEEEEEEG             \n");
    printf("         EE             .EEj       .EEG           \n");
    printf("         jE             EE           LEj          \n");
    printf("          E            EG              E.         \n");
    printf("          EE          EE   EEj    EE   tE         \n");
    printf("           EE         E    EEi    EE    Et        \n");
    printf("            EE       tE                 GE        \n");
    printf("             EE      EE                 iE        \n");
    printf("             EE EG   LE                 LE        \n");
    printf("             EEELEEEEEE                 EL        \n");
    printf("            EEE       EG                E         \n");
    printf("            G          E               EG         \n");
    printf("                       jEE            EE          \n");
    printf("                        .EEj        iEE           \n");
    printf("                          iEEEEEEEEE Ej           \n");
    printf("                             .LEGi EEEE           \n");
    printf("                                     tE           \n");
}