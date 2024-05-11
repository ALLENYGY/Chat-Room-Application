
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <process.h>
#include <vector>
#include <queue>
#include <mutex>
#include <algorithm>
#include <winsock2.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")
#define DEFAULT_PORT	5019

using namespace std;

struct terminal {
	int id;
	bool isLogged;
	string name;
	SOCKET sock;
	queue<string> messageQueue;  
	mutex queueMutex;            // 互斥锁保护消息队列
	condition_variable queueCond; // 条件变量用于通知消息可发送
};

struct group {
	int id;
	string name;
	vector<int> members;
};

vector<terminal*> clients;  
vector<group *> groups;
mutex clientsMutex;

void recv_message(terminal* client);
void send_message(terminal* client);
void handle_client(terminal* client);
void create_group(terminal* sender, string groupName);
void rename_client(terminal* sender, string newName);
void add_member_to_group(terminal* sender, string groupId, int clientId);
void remove_member_from_group(terminal* sender, string gname, int clientId);
void disconnect_client(terminal* sender);
void processMessage(terminal* sender, const char* buffer);

int main(int argc, char** argv) {
	struct sockaddr_in local;
	SOCKET sock;
	WSADATA wsaData;
	if (WSAStartup(0x202, &wsaData) == SOCKET_ERROR)
	{
		// stderr: standard error are printed to the screen.
		fprintf(stderr, "WSAStartup failed with error %d\n", WSAGetLastError());
		//WSACleanup function terminates use of the Windows Sockets DLL. 
		WSACleanup();
		return -1;
	}// end if

	// Fill in the address structure
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = INADDR_ANY;
	local.sin_port = htons(DEFAULT_PORT);
	sock = socket(AF_INET, SOCK_STREAM, 0);	//TCP socket


	if (sock == INVALID_SOCKET)
	{
		fprintf(stderr, "socket() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	// bind the socket
	if (bind(sock, (struct sockaddr*)&local, sizeof(local)) == SOCKET_ERROR)
	{
		fprintf(stderr, "bind() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	// waiting for connections
	if (listen(sock, 5) == SOCKET_ERROR)
	{
		fprintf(stderr, "listen() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}
	printf("Waiting for connections...\n");
	int clientId = 1;
	while (1) {
		sockaddr_in clientAddr;
		int clientSize = sizeof(clientAddr);
		SOCKET client_socket = accept(sock, (struct sockaddr*)&clientAddr, &clientSize);
		if (client_socket == INVALID_SOCKET) {
			cerr << "Accept failed with error: " << WSAGetLastError() << endl;
			continue;
		}
		// Create a name for the client append with clientId
		string name= "Anonymous" + to_string(clientId);
		terminal* newterminal = new terminal{ clientId++, true, name, client_socket };
		// Ensure safe access to the global clients list
		{
			lock_guard<mutex> guard(clientsMutex);
			clients.push_back(newterminal);
		}
		// Starting the client thread to handle incoming and outgoing messages
		thread clientThread(handle_client, newterminal);
		clientThread.detach();
		cout << "Connected to client: " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << endl;
	}
	closesocket(sock);
	WSACleanup();
	return 0;
}

void handle_client(terminal* client) {
	thread recvThread(recv_message, client);
	thread sendThread(send_message, client);
	recvThread.join();
	sendThread.join(); 
}

void recv_message(terminal* client) {
	char buffer[1024];
	while (true) {
		memset(buffer, 0, sizeof(buffer));
		int bytesReceived = recv(client->sock, buffer, sizeof(buffer), 0);
		//buffer[bytesReceived] = '\0';
		if (bytesReceived == SOCKET_ERROR) {
			cout << "Receive error for client " << client->name << ": " << WSAGetLastError() << endl;
			break;
		}
		else if (bytesReceived == 0) {
			cout << "Client " << client->name << " disconnected." << endl;
			break;
		}
		else {
			cout << "Receive From Client:" << client->name << ": " << buffer << endl;
		}
		processMessage(client, buffer);
	}
	client->isLogged = false;
	closesocket(client->sock);
}


void send_message(terminal* client) {
	unique_lock<mutex> lock(client->queueMutex);
	while (client->isLogged || !client->messageQueue.empty()) {
		client->queueCond.wait(lock, [&] { return !client->messageQueue.empty() || !client->isLogged; });
		while (!client->messageQueue.empty()) {
			string message = client->messageQueue.front();
			client->messageQueue.pop();
			send(client->sock, message.c_str(), message.length(), 0);
		}
	}
}

void sent_msg_to_specify_client(terminal* sender, int receiverID, const string& message) {
	bool isFound = false;
	terminal* targetClient = nullptr;
	// Find Receiver Client
	{
		lock_guard<mutex> guard(clientsMutex);  // 使用 lock_guard 来保护 clients 列表
		for (auto client : clients) {
			if (client->id == receiverID) {
				targetClient = client;
				isFound = true;
				break;
			}
		}
	}
	if (isFound && targetClient) {
		unique_lock<mutex> lock(targetClient->queueMutex);
		targetClient->messageQueue.push(message);
		targetClient->queueCond.notify_one();
	}
	else {
		unique_lock<mutex> lock(sender->queueMutex);
		sender->messageQueue.push("Client Not Found!");
		sender->queueCond.notify_one();
	}
}
void sent_msg_to_specify_Group(terminal* sender,string groupname,string message) {
	bool isFound = false;
	{
		lock_guard<mutex> guard(clientsMutex);  // 使用 lock_guard 保护对 groups 列表的访问
		for (auto& g : groups) {
			if (g->name == groupname) {
				isFound = true;
				for (auto& member : g->members) {
					for (auto& client : clients) {
						if (client->id == member) {
							unique_lock<mutex> lock(client->queueMutex);
							client->messageQueue.push(message);
							client->queueCond.notify_one();
						}
					}
				}	
				break;
			}
		}
	}
	if (!isFound) {
		unique_lock<mutex> lock(sender->queueMutex);
		sender->messageQueue.push("Group Not Found!");
		sender->queueCond.notify_one();
	}
}


vector<string> split(const string& s, char delimiter) {
	vector<string> tokens;
	string token;
	for (char ch : s) {
		if (ch == delimiter) {
			if (!token.empty()) {
				tokens.push_back(token);
				token.clear();
			}
		}
		else {
			token += ch;
		}
	}
	if (!token.empty()) {
		tokens.push_back(token);
	}
	return tokens;
}


void processMessage(terminal* sender, const char* buffer) {
	vector<string> com = split(buffer, '*');
	char op = com[0][0];
	switch (op) {
	case 'S': {
		// Send message to a specific client
		string msg= sender->name + ": " + com[2]+"\0";
		sent_msg_to_specify_client(sender, stoi(com[1]), msg);
		break;
	}
	case 'R':
		// Change name
		sender->name = com[1];
		sent_msg_to_specify_client(sender, sender->id, "Change Your Name Successfully！");
		break;
	case 'L':{ 
		// List all clients
		lock_guard<mutex> guard(clientsMutex);
		string clientList = "\nClient List:\n";
		for (auto client : clients) {
			clientList += to_string(client->id) + ": " + client->name + "\n";
		}
		unique_lock<mutex> lock(sender->queueMutex);
		sender->messageQueue.push(clientList);
		sender->queueCond.notify_one();
		break;
	}

	case 'C':
		create_group(sender, com[1]);
		break;
	case 'G': {
		string msg = com[1] + ": " + sender->name + ": " + com[2] + "\0";
		sent_msg_to_specify_Group(sender, com[1], msg);
		break;
	}
	case 'A': {
		// Add member to group
		int size = com.size();
		for(int i=2;i<size-1;i++)
			add_member_to_group(sender, com[1], stoi(com[i]));
		break;
	}
	case 'D': {
		int size = com.size();
		for (int i = 2; i < size - 1; i++)
			remove_member_from_group(sender, com[1], stoi(com[i]));
		break;
	}
	case 'E':
		disconnect_client(sender);
		break;
	default:
		unique_lock<mutex> lock(sender->queueMutex);
		sender->messageQueue.push("Invalid operation. Please try again.");
		sender->queueCond.notify_one();
		break;
	}
}

void create_group(terminal* sender, string groupName) {
	int id = groups.size();
	group* newgroup = new group{ id + 1, groupName, vector<int>{sender->id} };
	groups.push_back(newgroup);
}

void add_member_to_group(terminal* sender, string groupname, int clientId) {
	int size= groups.size();
	for (int i = 0; i < size; i++) {
		if (groups[i]->name == groupname){
			if (sender->id != groups[i]->members[0]) {
				string msg = "You are not the owner of the group (";
				msg += groups[i]->name;
				msg += ")\0";
				sender->messageQueue.push(msg);
				sender->queueCond.notify_one();
				return;
			}
			if (find(groups[i]->members.begin(), groups[i]->members.end(), clientId) != groups[i]->members.end()) {
				string msg = "The client is already in the group (";
				msg += groups[i]->name;
				msg += ")\0";
				sender->messageQueue.push(msg);
				sender->queueCond.notify_one();
				return;
			}
			groups[i]->members.push_back(clientId);
			string username;
			for (auto client : clients) {
				if (client->id == clientId) {
					username = client->name;
				}
			}
			string msg = "Add user " + username + " Successfully!\n\0";
			sender->messageQueue.push(msg);
			sender->queueCond.notify_one();
			break;
		}	
	}
}

void remove_member_from_group(terminal* sender, string gname, int clientId) {
	// Remove member from group
	int size = groups.size();
	for (int i = 0; i < size; i++) {
		if (groups[i]->name == gname) {
			if (sender->id != groups[i]->members[0]) {
				string msg = "You are not the owner of the group (";
				msg += groups[i]->name;
				msg += ")\0";
				sender->messageQueue.push(msg);
				sender->queueCond.notify_one();
				return;
			}
			// check if the clientID in the group
			int groupsize = groups[i]->members.size();
			bool isFounduser = false;
			for (auto iter = groups[i]->members.begin(); iter != groups[i]->members.end(); iter++)
			{
				if (clientId == *iter)
				{
					// Remove the client from group
					groups[i]->members.erase(iter);
					isFounduser = true;
					string msg = "Remove user Successfully!\0";
					sender->messageQueue.push(msg);
					sender->queueCond.notify_one();
					break;
				}
			}
			if (!isFounduser) {
				string msg = "User Not Found!\0";
				sender->messageQueue.push(msg);
				sender->queueCond.notify_one();
			}

		}
	}	
}

void disconnect_client(terminal* client) {
	client->isLogged = false;
	closesocket(client->sock);
}

