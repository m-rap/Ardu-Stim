#ifndef __TCP_H__
#define __TCP_H__

#ifdef __linux__

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class CircularArray {
private:
  uint8_t *buffer;
  int capacity;
  int head;  // Index for popping (front)
  int tail;  // Index for pushing (back)
  int size;  // Current number of elements
public:
  CircularArray(int capacity);
  virtual ~CircularArray();
  int pushArray(uint8_t* src, int count);
  int popArray(uint8_t* dest, int count);

  int getCapacity() { return capacity; }
  int getSize() { return size; }
};

class Tcp {
private:
  int sockfd;
  sockaddr_in serv_addr;
public:
  int listen(const char* ip, int port);
  int accept(Tcp& acceptor, char* clientIp, int& clientPort);
  int connect(const char* ip, int port);
  int close();
  uint8_t read();
  int readBytes(uint8_t* buff, int len);
  void write(uint8_t b);
  int writeBytes(uint8_t* buff, int len);

  virtual int available();
  void println(int);
  void println(const char*);
  void print(int);
  void print(const char*);
};

class BufferedTcp : public Tcp {
private:
  CircularArray* buff;
public:
  BufferedTcp() {
    buff = new CircularArray(64);
  }
  ~BufferedTcp() {
    delete buff;
  }
  virtual int available();
  uint8_t read();
  int readBytes(uint8_t* buff, int len);
  void write(uint8_t b);
  int writeBytes(uint8_t* buff, int len);
};

#endif

#endif