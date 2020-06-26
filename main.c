#include "header.h"
#include "functions.c"


int main (int argc, char * argv[])
{
    struct in_addr addr;
    int port_lo, port_hi, num_hosts;

    parse_args(argc, argv, &addr, &port_lo, &port_hi, &num_hosts);

    int socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (socket_fd < 0)
        fatal_err("Error creating socket!");

    // Set IP_HDRINCL socket option to tell the kernel 
    // that headers are included in the packet
    int oneVal = 1;
    if (setsockopt(socket_fd, IPPROTO_IP, IP_HDRINCL, &oneVal, sizeof(oneVal)) < 0)
        fatal_err("Error setting IP_HDRINCL!");

    pthread_t threads[num_hosts];
    struct thread_arg thr_arguments[num_hosts];
    results = malloc(num_hosts * sizeof(char*));

    for (int i = 0; i < num_hosts; i++)
    {
        results[i] = malloc(BUF_SIZE);
        fill_thread_arg(&thr_arguments[i], addr, port_lo, port_hi, socket_fd, i);
        pthread_create(&threads[i], NULL, scan_host, &thr_arguments[i]);
        addr.s_addr = htonl(ntohl(addr.s_addr) + 1);
    }

    for (int i = 0; i < num_hosts; i++)
        pthread_join(threads[i], NULL);


    printf("===> Results:\n");
    for (int i = 0; i < num_hosts; i++)
        printf("%s", results[i]);

    // while (num_hosts > 0)
    // {
    //     scan_host(addr, port_lo, port_hi, socket_fd);
    //     addr.s_addr = htonl(ntohl(addr.s_addr) + 1);
    //     num_hosts--;
    // }

}

