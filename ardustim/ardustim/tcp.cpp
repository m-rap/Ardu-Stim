#include "tcp.h"

#ifdef __linux__

// Initialize the circular array
CircularArray::CircularArray(int capacity) {
    capacity = capacity;
    buffer = new uint8_t[capacity];
    head = 0;
    tail = 0;
    size = 0;
}

CircularArray::~CircularArray() {
  delete[] buffer;
}

// Push an array into the circular array (returns actual pushed count)
int CircularArray::pushArray(uint8_t* src, int count) {
    int available_space = capacity - size;

    if (available_space <= 0 || count <= 0) {
        return 0;
    }

    // Only push what fits
    if (count > available_space) {
        count = available_space;
    }

    int space_to_end = capacity - tail;

    if (count <= space_to_end) {
        // Single copy
        memcpy(buffer + tail, src, count * sizeof(int));
    } else {
        // Wrap-around copy
        memcpy(buffer + tail, src, space_to_end * sizeof(int));
        memcpy(buffer, src + space_to_end, (count - space_to_end) * sizeof(int));
    }

    tail = (tail + count) % capacity;
    size += count;

    return count;
}

// Pop from the circular array into a destination (returns actual popped count)
int CircularArray::popArray(uint8_t* dest, int count) {
    if (size <= 0 || count <= 0) {
        return 0;
    }

    // Only pop what is available
    if (count > size) {
        count = size;
    }

    int space_to_end = capacity - head;

    if (count <= space_to_end) {
        // Single copy
        memcpy(dest, buffer + head, count * sizeof(int));
    } else {
        // Wrap-around copy
        memcpy(dest, buffer + head, space_to_end * sizeof(int));
        memcpy(dest + space_to_end, buffer, (count - space_to_end) * sizeof(int));
    }

    head = (head + count) % capacity;
    size -= count;

    return count;
}

int Tcp::listen(const char* ip, int port) {
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  int reuse = 1;
  int err = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
  if (err != 0) {
    return err;
  }

	/* Initialize socket structure */
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(ip);
  serv_addr.sin_port = htons(port);

  err = bind(sockfd, (sockaddr *) &serv_addr, sizeof(serv_addr));
  if (err < 0)
  {
    ::close(sockfd);
    return err;
  }

  err = ::listen(sockfd, 5);

  return err;
}

int Tcp::accept(Tcp& acceptor, char* clientIp, int& clientPort) {
  socklen_t clilen;
  sockaddr_in cli_addr;

  clilen = sizeof(cli_addr);

  int newsockfd = ::accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

  if (newsockfd < 0)
  {
    return -1;
  }

  if (clientIp != 0)
  {
    clientPort = cli_addr.sin_port;
    sprintf(clientIp, "%d.%d.%d.%d",
      int(cli_addr.sin_addr.s_addr&0xFF),
      int((cli_addr.sin_addr.s_addr&0xFF00)>>8),
      int((cli_addr.sin_addr.s_addr&0xFF0000)>>16),
      int((cli_addr.sin_addr.s_addr&0xFF000000)>>24));
  }

  acceptor.sockfd = newsockfd;

  return 0;
}

int Tcp::connect(const char* ip, int port) {
  int rc = 0;
  sockaddr_in serveraddr;
  memset(&serveraddr, 0x00, sizeof(serveraddr));

  serveraddr.sin_family = AF_INET;
  serveraddr.sin_port = htons(port);
  serveraddr.sin_addr.s_addr = inet_addr(ip);

  rc = ::connect(sockfd, (sockaddr *)&serveraddr, sizeof(serveraddr));
  if (rc < 0)
  {
    return rc;
  }

  struct timeval tv;
  tv.tv_sec = 3;
  tv.tv_usec = 0;
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

  return rc;
}

int Tcp::close() {
  shutdown(sockfd, SHUT_RDWR);
  ::close(sockfd);
}

uint8_t Tcp::read() {
  uint8_t b;
  recv(sockfd, &b, 1, 0);
  return b;
}

int Tcp::readBytes(uint8_t* buff, int len) {
  return recv(sockfd, buff, len, 0);
}

void Tcp::write(uint8_t b) {
  send(sockfd, &b, 1, 0);
}

int Tcp::writeBytes(uint8_t* buff, int len) {
  return send(sockfd, buff, len, 0);
}

int Tcp::available() {
  return 0;
}

void Tcp::println(int n) {
  uint8_t buff[128];
  int nstr = snprintf((char*)buff, 127, "%d\n", n);
  writeBytes(buff, nstr);
}

void Tcp::println(const char* s) {
  uint8_t buff[4096];
  strcpy((char*)buff, s);
  strcat((char*)buff, "\n");
  writeBytes(buff, strlen(s)+1);
}
void Tcp::print(int n) {
  uint8_t buff[128];
  int nstr = snprintf((char*)buff, 127, "%d", n);
  writeBytes(buff, nstr);
}
void Tcp::print(const char* s) {
  writeBytes((uint8_t*)s, strlen(s)+1);
}

int BufferedTcp::available() {
  if (buff->getSize() > 0) {
    return buff->getSize();
  }
  uint8_t tmpbuff[64];
  int nread = readBytes(tmpbuff, buff->getCapacity());
  buff->pushArray(tmpbuff, nread);
  return buff->getSize();
}

uint8_t BufferedTcp::read() {
  uint8_t b;
  buff->popArray(&b, 1);
  return b;
}

int BufferedTcp::readBytes(uint8_t* tmpbuff, int len) {
  return buff->popArray(tmpbuff, len);
}


#endif
