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

  pool.enqueue([sckt] () {
    for (;;) {
      char s[1000];
      bzero(s, 1000);
      cin.getline(s, 1000);
      send(sckt, s, strlen(s), 0);
    }
  });

  pool.enqueue([sckt] () {
    for (;;) {
      char s[1000];
      bzero(s, 1000);
      read(sckt, s, 1000);
      cout << s << endl;
    }
  });
}
