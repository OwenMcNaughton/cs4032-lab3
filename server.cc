#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include "threadpool.h"
#include <arpa/inet.h>

using namespace std;

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
    cout << msg << endl;

    if (msg.substr(0, 4) == "HELO") {
      char ip[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &(client_address.sin_addr), ip, INET_ADDRSTRLEN);
      string response = msg + "IP:" + "10.62.0.156" + "\nPort:" +
        to_string(port) + "\nStudentID:12308914\n";
      send(client, response.c_str(), strlen(response.c_str()), 0);
    }

    if (msg == "KILL_SERVICE\\n" || msg == "KILL_SERVICE") {
      exit(0);
    }

    // Indicates client was killed.
    if (msg == "") {
      break;
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
  cout << ip << endl;

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
