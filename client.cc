#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include "threadpool.h"
#include <unistd.h>

using namespace std;

int main (int argc, char** argv) {
  ThreadPool pool(2000);
  struct sockaddr_in server_address;

  int sckt = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

  bzero((char*)&server_address, sizeof(server_address));
  server_address.sin_family = AF_INET;

  struct hostent *server = gethostbyname(argv[1]);
  bcopy((char*)server->h_addr,
    (char*)&server_address.sin_addr.s_addr, server->h_length);

  server_address.sin_port = htons(atoi(argv[2]));

  if (connect(sckt, (sockaddr*)&server_address, sizeof(server_address)) < 0) {
    cout << "Can't connect... " << endl;
    return 1;
  } else {
    cout << "Connected" << endl;
  }


  string s = "JOIN_CHATROOM: buffy\n"
  "CLIENT_IP: [IP Address of client if UDP | 0 if TCP]\n"
  "PORT: [port number of client if UDP | 0 if TCP]\n"
  "CLIENT_NAME: tyler303\n";
  cout << "sending:     " << s << endl;
  send(sckt, s.c_str(), strlen(s.c_str()), 0);
  sleep(2);

  pool.enqueue([sckt] () {
    for (;;) {
      // char s[1000];
      // bzero(s, 1000);
      // cin.getline(s, 1000);

      // s = "LEAVE_CHATROOM: 0\n"
      //   "JOIN_ID: 4\n"
      //   "CLIENT_NAME: tyler303\n";
      // cout << "sending:     " << s << endl;
      // send(sckt, s.c_str(), strlen(s.c_str()), 0);
      // sleep(2);
      //
      // s = "DISCONNECT: 0\n"
      //   "PORT: 4\n"
      //   "CLIENT_NAME: tyler303\n";
      // cout << "sending:     " << s << endl;
      // send(sckt, s.c_str(), strlen(s.c_str()), 0);
      // sleep(2);

      string s = "CHAT: 0\n"
        "JOIN_ID: 4\n"
        "CLIENT_NAME: tyler303\n"
        "MESSAGE: whats ur fave episode?\n\n";
      send(sckt, s.c_str(), strlen(s.c_str()), 0);

      sleep(4);
    }
  });

  pool.enqueue([sckt] () {
    for (;;) {
      char s[1000];
      bzero(s, 1000);
      read(sckt, s, 1000);
      cout << s << "\n";
    }
  });
}
