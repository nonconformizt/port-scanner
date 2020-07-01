#if !defined(HEADER)
#define HEADER

#include <arpa/inet.h>
#include <ctype.h>
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
#include <stdbool.h>

#include "port_association.h"


#define PORT_LO 1 
#define PORT_HI 65535 
#define BUF_SIZE 65536
#define DATAGRAM_SIZE 4096
// How long to wait for host to respond
#define WAIT_TIMEOUT 3
// Default string length
#define STR_SIZE 1024
// Maximum number of threads
#define MAX_THREADS 5



/**
 * Global variables
 */

int socket_fd,
    port_lo,
    port_hi;
// Services detection
bool detection_enabled;

// Stack for unresolved ip-adresses
pthread_mutex_t stack_lock;
struct in_addr * stack; // array of addresses
int stack_size;
int stack_top;


/**
 * Structures
 */

// Listening thread arguments
struct listen_thr_arg
{
    bool stop;
    int sock_raw;
    struct in_addr target;
};

// Needed for checksum calculation
struct pseudo_header {
    unsigned int source_address;
    unsigned int dest_address;
    unsigned char placeholder;
    unsigned char protocol;
    unsigned short tcp_length;

    struct tcphdr tcp;
};



#endif // HEADER
