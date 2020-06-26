#if !defined(HEADER)
#define HEADER

#include <arpa/inet.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>


#define PORT_LO 1 
#define PORT_HI 65535 

#define BUF_SIZE 65536
#define DATAGRAM_SIZE 4096

// how long to wait for host to respond
#define WAIT_TIMEOUT 3

struct thread_arg {
    struct in_addr target;
    int port_lo,
        port_hi,
        socket_fd,
        index;
};

struct listen_host_arg {
    int index;
    struct in_addr * target_in;
};

// needed for checksum calculation
struct pseudo_header { 
    unsigned int source_address;
    unsigned int dest_address;
    unsigned char placeholder;
    unsigned char protocol;
    unsigned short tcp_length;

    struct tcphdr tcp;
};

char ** results;

#endif // HEADER
