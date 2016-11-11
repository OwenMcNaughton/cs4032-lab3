#include <arpa/inet.h>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "threadpool.h"
#include <unistd.h>

using namespace std;

map<string, int> clients_;
int room_count_ = 0;
map<string, pair<int, set<string>>> rooms_;

vector<string> SplitString(const string& s, char c) {
  string buff{""};
  vector<string> v;

  for (auto n : s) {
    if (n != c) {
      buff += n;
    } else if (n == c && buff != "") {
      v.push_back(buff);
      buff = "";
    }
  }

  if (buff != "") {
    v.push_back(buff);
  }

  return v;
}

void SendMessage(const string& message, const string& dest) {
  int client_id = clients_[dest];
  send(client_id, message.c_str(), strlen(message.c_str()), 0);
}

void Announce(const string& message, const string& client_name, const string& room_name) {
  int room_ref = rooms_[room_name].first;
  for (const string& user : rooms_[room_name].second) {
    string message2 = "CHAT: " + to_string(room_ref) + "\n"
      "CLIENT_NAME: " + client_name +
      "MESSAGE: " +  message + '\n';
    SendMessage(message2, user);
  }
}

void JoinRoom(string msg, int client) {
  size_t pos = msg.find('\n', 14);
  string room = msg.substr(15, pos - 15 + 1);
  size_t pos2 = msg.find("CLIENT_NAME", pos + 1);
  size_t pos3 = msg.find("\n", pos2 + 13);
  string client_name = msg.substr(pos2 + 13, pos3 - (pos2 + 13) + 1);

  if (clients_.count(client_name) == 0) {
    clients_.emplace(client_name, client);
  }

  bool joined = false;
  int room_ref = -1;
  if (rooms_.count(room) == 0) {
    set<string> clients;
    clients.insert(client_name);
    room_ref = room_count_;
    rooms_.emplace(room, make_pair(room_count_++, clients));
    joined = true;
  } else {
    if (rooms_[room].second.count(client_name) == 0) {
      room_ref = rooms_[room].first;
      rooms_[room].second.insert(client_name);
      joined = true;
    } else {
      string response = "ERROR_CODE: 0\n"
        "ERROR_DESCRIPTION: You're already in this room\n";
      send(client, response.c_str(), strlen(response.c_str()), 0);
    }
  }

  if (joined) {
    string response = "JOINED_CHATROOM: " + room + "\n"
      "SERVER_IP: 10.62.0.156\n"
      "PORT: 8000\n"
      "ROOM_REF: "+ to_string(room_ref) + "\n"
      "JOIN_ID: " + to_string(client) + "\n";
    send(client, response.c_str(), strlen(response.c_str()), 0);
    Announce("Hello", client_name, room);
  }
}

void LeaveRoom(string msg, int client) {
  size_t pos = msg.find('\n', 15);
  string room_ref = msg.substr(16, pos - 16 + 1);
  size_t pos2 = msg.find("JOIN_ID", pos + 1);
  size_t pos3 = msg.find("\n", pos2 + 1);
  string join_id = msg.substr(pos2 + 9, pos3 - (pos2 + 9) + 1);
  size_t pos4 = msg.find("CLIENT_NAME", pos3);
  string client_name = msg.substr(pos4 + 12);

  if (clients_.count(client_name) == 0) {
    string response = "ERROR_CODE: 1\n"
      "ERROR_DESCRIPTION: You can't leave because that client doesn't exist.\n";
    send(client, response.c_str(), strlen(response.c_str()), 0);
    return;
  }

  int room_ref_int = atoi(room_ref.c_str());
  string room_name = "";
  for (const auto& room : rooms_) {
    if (room.second.first == room_ref_int) {
      room_name = room.first;
    }
  }

  if (room_name == "") {
    string response = "ERROR_CODE: 2\n"
      "ERROR_DESCRIPTION: You can't leave because that room doesn't exist.\n";
    send(client, response.c_str(), strlen(response.c_str()), 0);
  } else {
    if (rooms_[room_name].second.count(client_name) == 0) {
      string response = "ERROR_CODE: 3\n"
        "ERROR_DESCRIPTION: You can't leave because you weren't in that room\n";
      send(client, response.c_str(), strlen(response.c_str()), 0);
    } else {
      rooms_[room_name].second.erase(client_name);
      string response = "LEFT_CHATROOM: " + room_ref + "\n"
        "JOIN_ID: " + join_id + "\n";
      send(client, response.c_str(), strlen(response.c_str()), 0);
      Announce("Bye", client_name, room_name);
    }
  }
}

bool Disconnect(string msg, int client) {
  size_t pos = msg.find("CLIENT_NAME");
  size_t pos2 = msg.find("\n", 13);
  string client_name = msg.substr(pos + 13, pos2 - (pos + 13) + 1);

  if (clients_.count(client_name) == 0) {
    string response = "ERROR_CODE: 4\n"
      "ERROR_DESCRIPTION: You can't disconnect because that client doesn't exist\n";
    send(client, response.c_str(), strlen(response.c_str()), 0);
    return false;
  }

  for (auto& room : rooms_) {
    if (room.second.second.count(client_name) != 0) {
      room.second.second.erase(client_name);
      Announce("Bye", client_name, room.first);
    }
  }

  return true;
}

void Chat(string msg, int client) {
  size_t pos = msg.find('\n', 6);
  string room_ref = msg.substr(6, pos - 6);
  size_t pos2 = msg.find("CLIENT_NAME", pos + 1);
  size_t pos3 = msg.find("\n", pos2);
  string client_name = msg.substr(pos2 + 13, pos3 - (pos2 + 13) + 1);
  size_t pos4 = msg.find("MESSAGE", pos3);
  string message = msg.substr(pos4 + 9);

  if (clients_.count(client_name) == 0) {
    string response = "ERROR_CODE: 5\n"
      "ERROR_DESCRIPTION: You can't send a message because that client name is not recognised\n";
    send(client, response.c_str(), strlen(response.c_str()), 0);
    return;
  }

  int room_ref_int = atoi(room_ref.c_str());
  string room_name = "";
  for (const auto& room : rooms_) {
    if (room.second.first == room_ref_int) {
      room_name = room.first;
    }
  }

  if (room_name == "") {
    string response = "ERROR_CODE: 6\n"
      "ERROR_DESCRIPTION: You can't send a message because that room_ref is not recognised\n";
    send(client, response.c_str(), strlen(response.c_str()), 0);
    return;
  }

  Announce(message, client_name, room_name);
}

bool HandleChatroomMessage(const string& msg, int client) {
  vector<string> line_split = SplitString(msg, '\n');

  if (msg.substr(0, 13) == "JOIN_CHATROOM") {
    JoinRoom(msg, client);
  } else if (msg.substr(0, 14) == "LEAVE_CHATROOM") {
    LeaveRoom(msg, client);
  } else if (msg.substr(0, 10) == "DISCONNECT") {
    if (Disconnect) {
      return true;
    }
  } else if (msg.substr(0, 4) == "CHAT") {
    Chat(msg, client);
  }

  return false;
}

void HandleClient(int client, sockaddr_in client_address, int port) {
  if (client < 0) {
    cout << "Connection failed" << endl;
    return;
  }

  cout << "Client " << client << " connected" << endl;

  char buffer[1000];
  for (;;) {
    bzero(buffer, 1000);
    read(client, buffer, 1000);

    string msg(buffer);

    if (msg.substr(0, 4) == "HELO") {
      char ip[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &(client_address.sin_addr), ip, INET_ADDRSTRLEN);
      string response = msg + "IP:" + "10.62.0.156" + "\nPort:" +
        to_string(port) + "\nStudentID:12308914\n";
      send(client, response.c_str(), strlen(response.c_str()), 0);
    } else if (msg == "KILL_SERVICE\\n" || msg == "KILL_SERVICE") {
      exit(0);
    } else if (msg == "") {
      break;
    } else {
      if (HandleChatroomMessage(msg, client)) {
        break;
      }
    }
  }
  cout << "Closing connection to client " << client << endl;
  close(client);
}

int main(int argc, char* argv[]) {
  struct sockaddr_in server_address;
  ThreadPool pool(1000);

  int flag = 1;
  int sckt = socket(AF_INET, SOCK_STREAM, 0);
  setsockopt(sckt, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int));
  int port = atoi(argv[1]);

  bzero((char*)&server_address, sizeof(server_address));

  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port = htons(port);

  char ip[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &(server_address.sin_addr), ip, INET_ADDRSTRLEN);

  if (bind(sckt, (sockaddr*)&server_address, sizeof(server_address)) < 0) {
    cout << "Couldn't bind" << endl;
    return 0;
  }

  listen(sckt, 1000);
  cout << "Listening" << endl;

  for (;;) {
    struct sockaddr_in client_address;
    socklen_t len = sizeof(client_address);
    int client = accept(sckt, (sockaddr *)&client_address, &len);

    pool.enqueue([client, client_address, port] () {
      HandleClient(client, client_address, port);
    });
  }
}
