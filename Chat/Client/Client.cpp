#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <thread>
#include <cstring>

#pragma comment(lib, "ws2_32.lib")
#define DEFAULT_PORT	5019

string wechatIcon = R"(
                 ,EEEEEEi                         
               EEEEt..tEEEE                       
             LEE          GEE                     
            EE              EE                    
           EE                EE                   
          GE     G     tj     EE                  
          E     EEE    EEG     E                  
         iE     EE.    EE      EL                 
         EE                    LE                 
         Ei                  .LEEi                
         Et               iEEEEEEEEEG             
         EE             .EEj       .EEG           
         jE             EE           LEj          
          E            EG              E.         
          EE          EE   EEj    EE   tE         
           EE         E    EEi    EE    Et        
            EE       tE                 GE        
             EE      EE                 iE        
             EE EG   LE                 LE        
             EEELEEEEEE                 EL        
            EEE       EG                E         
            G          E               EG         
                       jEE            EE          
                        .EEj        iEE           
                          iEEEEEEEEE Ej           
                             .LEGi EEEE           
                                     tE           
)";

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
void red_print(string out);
void green_print(string out);
void blue_print(string out);
void printWechat();


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

	// printf("Connected to server: %s:%d successfully!\n", server_name, port);
	printWechat();
	// printf("Type 'E' to close the connection\n");
	string command=renameOperation();
	sendToServer(connect_sock, command);
	hintMessage();
	// Start the threads for sending and receiving data
	thread sendThread(sendData, connect_sock);
	thread recvThread(receiveData, connect_sock);
	// Wait for the send thread to finish before closing
	sendThread.join();
	recvThread.join();  
	// Cleanup
	closesocket(connect_sock);
	WSACleanup();
	return 0;

}


void hintMessage() {
	string msg="Hello, " + username + "! Welcome to Chat Room!\n";
	blue_print("------------------------------------------------------------------\n");
	blue_print(msg);
	blue_print("------------------------------------------------------------------\n");
	blue_print("Type 'R' to Rename yourself\n");						// Rename username
	blue_print("Type 'S' to Send messages to a specific member\n");		// Send messages to a specific member
	blue_print("Type 'E' to Exit\n");									// Quit the Chat Room
	blue_print("Type 'L' to List all members in the chat room\n");		// List all members in the chat room
	blue_print("Type 'C' to Create a new group\n");						// Create a new group
	blue_print("Type 'A' to Add a member to the group\n");				// Add a member to the group
	blue_print("Type 'D' to Delete a member from the group\n");			// Delete a member from the group
	blue_print("Type 'G' to Send messages to the group\n");				// Send messages to the group
	blue_print("------------------------------------------------------------------\n");
}

string sendOperation() {
	string prefix = "S";
	blue_print("Enter the ID of the member you want to send a message to: \n");
	string id; cin >> id;
	if (!checknumber(id)) {
		red_print("Invalid ID.\n");
		return "";
	}
	blue_print("Enter the message you want to send:\n");
	string msg; cin.ignore();
	getline(cin,msg);
	string command = prefix + "*" + id + "*" + msg + "*\0";
	return command;
}

string sendGroupOperation() {
	string prefix = "G";
	blue_print("Enter the Group Name: \n");
	string gname; cin >> gname;
	blue_print("Enter the message you want to send:\n");
	string msg; cin.ignore();
	getline(cin, msg);
	string command = prefix + "*" + gname + "*" + msg + "*\0";
	return command;
}

string renameOperation() {
	string prefix = "R";
	blue_print("Enter your new name: ");
	cin >> username;
	string command = prefix + "*" + username + "*\0";
	return command;
}

string createGroupOperation(SOCKET sock) {
	string prefix = "C";
	blue_print("Enter your Group name: ");
	string groupname;
	cin >> groupname;
	string command = prefix + "*" + groupname+"*\0";
	sendToServer(sock, command);
	blue_print("Here is the member in the Chat room that You can invite!\n");
	sendToServer(sock, showRoomMember());
	sendToServer(sock, addnewMemberOperation(groupname));
	// printf("", groupname.c_str());
	string msg="Group "+ groupname.c_str() +" has been created.\n";
	blue_print(msg);
	return command;
}

string addMemberOperation() {
	blue_print("Enter the Group name:\n");
	string groupname;
	cin >> groupname;
	string command= addnewMemberOperation(groupname);
	return command;
}

string addnewMemberOperation(string groupname) {
	string prefix = "A";
	prefix += "*" + groupname + "*";
	blue_print("Enter the UserID that you want to invite: (end with #)\n");
	string id;
	int count = 0;
	string command = prefix + "*";
	cin >> id;
	if (!checknumber(id)) {
		red_print("Invalid ID.\n");
		return "";
	}
	while (id != "#") {
		command = command + id + "*";
		count++;
		cin >> id;
		if (!checknumber(id) && id!="#") {
			red_print("Invalid ID.\n");
			return "";
		}
	}
	command += "*" + to_string(count) + "*\0";
	return command;
}

string deleteMemberOperation() {
	blue_print("Enter the Group name:\n");
	string groupname;
	cin >> groupname;
	string command = deletenewMemberOperation(groupname);
	return command;
}


string deletenewMemberOperation(string groupname) {
	string prefix = "D";
	prefix += "*"+ groupname + "*";
	blue_print("Enter the UserID that you want to delete: (end with #)\n");
	string id;
	int count = 0;
	string command = prefix + "*";
	cin>> id;
	if (!checknumber(id)) {
		red_print("Invalid ID.\n");
		return "";
	}
	while (id != "#") {
		command = command + id + "*";
		count++;
		cin >> id;
		if (!checknumber(id) && id != "#") {
			red_print("Invalid ID.\n");
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
		blue_print("Select your operation: Type 'H' to get help.\n");
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
			blue_print("Bye!\n"); flag = false;
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
			red_print("Invalid command.\n");
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
			string msg=recvBuff.c_str()+"\n";
			blue_print(msg);
			// blue_print("%s\n", recvBuff);
		}
		else if (bytesReceived == 0) {
			red_print("Server closed the connection\n");
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


void red_print(string out)
{
    cout << "\033[31;1m" << out << "\033[0m" << endl;
}

void green_print(string out)
{
    cout << "\033[32;1m" << out << "\033[0m" << endl;
}

void blue_print(string out)
{
    cout << "\033[34;1m" << out << "\033[0m" << endl;
}

void printWechat()
{
    green_print(wechatIcon);
}


