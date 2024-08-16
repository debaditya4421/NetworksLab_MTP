#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>

#define P(s) semop(s, &pop, 1)
#define V(s) semop(s, &vop, 1)

#define SOCK_MTP SOCK_DGRAM

// Define constants
#define T 5
#define MAX_MTP_SOCKETS 25
#define MAX_TIME 5

#define MAX_RWND 5
#define MAX_SWND 5

#define MAX_RECV_BUF_SZ 5
#define MAX_SEND_BUF_SZ 10

#define KEY_SM 30
#define prob 0.3

// Define the maximum size of a message
#define MAX_MESSAGE_SIZE 1030

// Define MTP header structure
typedef struct {
    uint8_t sequence_number;    // 4-bit sequence number
    // Add other necessary fields
} MTPHeader;

// Define shared memory structure
typedef struct {
    int is_allocated;           // Indicates if the MTP socket is allocated
    pid_t process_id;           // Process ID for the process that created the MTP socket
    socklen_t udp_socket_id;          // Mapping from MTP socket ID to the corresponding UDP socket ID
    struct sockaddr_in dest_addr; // IP and port address of the other end of the MTP socket
    struct sockaddr_in source_addr; // IP and port address of the other end of the MTP socket
    // Add send buffer and receive buffer as fixed size arrays of messages
    char send_buffer[MAX_SEND_BUF_SZ][MAX_MESSAGE_SIZE]; // 10 messages of 1 KB each
    int send_ptr1, send_ptr2, send_ptr11;  // 0,0,0
    int send_size;  // no of messages in send buffer ; ptr1 to ptr2
    int send_size1;  // no of messages actually in send buffer ; ptr11 to ptr2
    int recv_ptr1, recv_ptr2;
    char recv_buffer[MAX_RECV_BUF_SZ][MAX_MESSAGE_SIZE];  // 5 messages of 1 KB each
    // Add swnd (sender window size) and rwnd (receiver window size) structures
    struct {
        int size;               // Size of the window
        int sequence_numbers[MAX_SWND]; // Sequence numbers of messages sent but not acknowledged
    } swnd;
    struct {
        int size;               // Size of the window
        int sequence_numbers[MAX_RWND]; // Sequence numbers of messages received but not acknowledged
    } rwnd;
    int timestamp; // last time of the data msg sent 
    int last_seq_no;  // next seq no to be read from m_recv_from ; initialize = 0
    int last_entry_rwnd; // 5
    int next_seq_no;  // next seq no to be sent by thread_S ; initialize = 0
    int send_max; // no of more msgs that can be sent by the sender = y
    int time_recv; // last time of the ack msg sent
    int wr1,wr2; // no of msgs written into the receive buf, no of msgs taken out of the recv buf

} SharedMemory;

// Define MTP socket structure
typedef struct {
    int sockfd;
    struct sockaddr_in local_addr;
    struct sockaddr_in dest_addr;
    // Add other necessary fields
} MTPSocket;

// Function prototypes
int m_socket(int domain, int type, int protocol);
int m_bind(struct sockaddr_in src_addr, int s1, struct sockaddr_in dest_addr, int s2);
int m_sendto(int sockfd, const char *buf, size_t len, int flags,const struct sockaddr *dest_addr, socklen_t addrlen);
char * m_recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
int m_close(int sockfd);

int dropMessage(float p);

void set_null(char *arr,int len);
