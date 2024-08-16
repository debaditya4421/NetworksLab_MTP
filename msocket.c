#include "msocket.h"


int m_socket(int domain, int type, int protocol) {
    struct sembuf pop, vop;
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1; 
    vop.sem_op = 1;

    int key1 = ftok(".", 15);
    int mtx = semget(key1, 1, 0777|IPC_CREAT);

    V(mtx);

    // int sockfd = socket(domain, type, protocol);
    // if (sockfd == -1) {
    //     perror("socket");
    //     return -1;
    // }
    // // Find a free entry in the mtp_sockets array
    // int i;
    // for (i = 0; i < MAX_MTP_SOCKETS; i++) {
    //     if (mtp_sockets[i].sockfd == 0) {
    //         mtp_sockets[i].sockfd = sockfd;
    //         return i; // Return the index as the socket ID
    //     }
    // }
    // errno = ENOBUFS;
    // return -1;
}

void set_null(char *arr,int len){
    for(int i=0;i<len;i++){
        arr[i] = '\0';
    }
}

int dropMessage(float p){
    srand(time(0));
    float genprob = (float)rand() / (float)RAND_MAX;
    printf("genprob : %f p : %f\n",genprob,p);
    if (genprob < p) return 1;
    // printf("genprob1 : %f p : %f\n",genprob,p);
    // printf("");
    return 0;
}

int m_bind(struct sockaddr_in src_addr, int s1, struct sockaddr_in dest_addr, int s2) {
    struct sembuf pop, vop;
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1; 
    vop.sem_op = 1;

    int key2 = ftok(".", 16);
    int mtx_while = semget(key2, 1, 0777|IPC_CREAT);

    int key3 = ftok(".", 17);
    int csmutex = semget(key3, 1, 0777|IPC_CREAT);

    int key4 = ftok(".", 18);
    int mtx_bind = semget(key4, 1, 0777|IPC_CREAT);
    
    P(mtx_while);

    printf("Reached here\n");

    key_t shm_key = ftok(".", KEY_SM);
    int shm_id = shmget(shm_key, sizeof(SharedMemory)*25, IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("Error creating shared memory segment in m_bind");
        exit(EXIT_FAILURE);
    }
    SharedMemory *shared_memory = (SharedMemory *)shmat(shm_id, NULL, 0);

    int i2=-99;
    P(csmutex);
    for(int i=0;i<25;i++){
        // printf("abcd");
        // printf("%d", shared_memory[i].is_allocated);
        if(shared_memory[i].is_allocated == 1){

            // printf("jjo\n");

            shared_memory[i].is_allocated = 2;
            shared_memory[i].dest_addr = dest_addr;
            shared_memory[i].source_addr = src_addr;
            // for (int k=0; k<10; k++) sprintf(shared_memory[i].send_buffer[k], "");
            shared_memory[i].timestamp = time(NULL);
            shared_memory[i].process_id = getpid();

            // printf("I'm here %d\n",shared_memory[i].process_id);

            i2=i;
            // V(csmutex);

            // struct sockaddr_in my_addr1 ; // source IP,Port
            // my_addr1.sin_family = AF_INET; 
            // inet_aton(src_ip , my_addr1.sin_addr);
            // my_addr1.sin_port = htons(src_ip);

            printf("Udp socket id in m_bind: %d \n" , shared_memory[i].udp_socket_id);
            // if ( bind(shared_memory[i].udp_socket_id, (struct sockaddr *)&src_addr, sizeof(src_addr)) < 0 ) { 
            //     perror("bind failed"); 
            //     exit(EXIT_FAILURE); 
            // }

            // struct sockaddr_in serv_addr1 ; // destination IP,Port
            // serv_addr1.sin_family = AF_INET; 
            // inet_aton(dest_ip , serv_addr1.sin_addr);
            // serv_addr1.sin_port = htons(dest_port);


            break;
        }
    }

    V(csmutex);
    V(mtx_bind);

    usleep(100);
    printf("Returning in m_bind i2 = %d\n",i2);

    return i2;
}

int m_sendto(int sockfd, const char *buf, size_t len, int flags,const struct sockaddr *dest_addr, socklen_t addrlen)
{    
    int key3 = ftok(".", 17);
    int csmutex = semget(key3, 1, 0777|IPC_CREAT);
    struct sembuf pop, vop;
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1; 
    vop.sem_op = 1;

    P(csmutex);

    int sockfd1=sockfd;
    key_t shm_key = ftok(".", KEY_SM);
    int shm_id = shmget(shm_key, sizeof(SharedMemory)*25, IPC_CREAT | 0666);
    if (shm_id == -1) {
        printf("Error creating shared memory segment in m_sendto\n");
        V(csmutex);
        exit(EXIT_FAILURE);
    }
    SharedMemory *shared_memory = (SharedMemory *)shmat(shm_id, NULL, 0);

    
    struct sockaddr_in * servaddr = (struct sockaddr_in *)dest_addr ; 
    int fl=0;
    if(servaddr->sin_port != shared_memory[sockfd].dest_addr.sin_port){
        printf("Wrong port in m_sendto\n");
        fl=1;
    }
    
    char *ipclient = inet_ntoa(servaddr->sin_addr); 
    char *ipclient1 = inet_ntoa(shared_memory[sockfd].dest_addr.sin_addr); 

    if(strcmp(ipclient,ipclient1)!=0){
        fl=1;
        printf("Wrong ip in m_sendto\n");
    }

    if(fl==1){
        printf("Bounded ip,port doesnt match in m_sendto \n");
        // set global err var to ENOTBOUND
        V(csmutex);
        return -1;
    }

    fl=0;

    // printf("ptr1 : %d\n",shared_memory[sockfd].send_ptr1);
    // printf("ptr2 : %d\n",shared_memory[sockfd].send_ptr2);
    // printf("send_size1 : %d\n",shared_memory[sockfd].send_size1);
    // printf("send_size : %d\n",shared_memory[sockfd].send_size);
    if((shared_memory[sockfd].send_size1 == MAX_SEND_BUF_SZ)) {
        fl=1;
    }



    if(fl==1){
        printf("Send buffer is full in m_sendto \n");
        // set global err var to ENOBUFS
        V(csmutex);
        return -1;
    }

    
    int ptr2 = shared_memory[sockfd].send_ptr2;
    // insert the msg at ptr2
    printf("strlen of msg: %d\n",strlen(buf));
    strcpy(shared_memory[sockfd].send_buffer[ptr2],buf);
    printf("MSG written to buffer: %s\n",(shared_memory[sockfd].send_buffer[ptr2]));

    // then +1 ptr2 and +1 send_size
    shared_memory[sockfd].send_ptr2 = (shared_memory[sockfd].send_ptr2+1)%MAX_SEND_BUF_SZ;
    shared_memory[sockfd].send_size++; 
    // shared_memory[sockfd].send_size1++; 

    int kk = 0;
    for(int s=0;s<MAX_SEND_BUF_SZ;s++){
        if(strlen(shared_memory[sockfd].send_buffer[s]) != 0) kk++;
    }
    shared_memory[sockfd].send_size1 = kk;

    printf("send_size1 after upd: %d @%ld\n",shared_memory[sockfd].send_size1, time(NULL)-1711100847);
    printf("Socket is %d\n", sockfd);
    sleep(1);


    printf("Written successfully in send buffer in m_sendto\n");

    V(csmutex);

    // printf("Sent : %s\n", shared_memory[sockfd].send_buffer[ptr2]);
}


char * m_recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen){
    key_t shm_key = ftok(".", KEY_SM);
    int shm_id = shmget(shm_key, sizeof(SharedMemory)*25, IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("Error creating shared memory segment in m_bind");
        exit(EXIT_FAILURE);
    }
    SharedMemory *shared_memory = (SharedMemory *)shmat(shm_id, NULL, 0);

    struct sembuf pop, vop;
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1; 
    vop.sem_op = 1;

    int key3 = ftok(".", 17);
    int csmutex = semget(key3, 1, 0777|IPC_CREAT);
    

    for (int w = 0; w < MAX_RECV_BUF_SZ; w++)
    {
        if(strlen(shared_memory[sockfd].recv_buffer[w]) > 0){

            // printf("ee : %s\n",shared_memory[sockfd].recv_buffer[w]);

            char *str = (char *)malloc(sizeof(char)*MAX_MESSAGE_SIZE);
            strcpy(str,shared_memory[sockfd].recv_buffer[w]);
            int len = strlen(str);
            int seq_no=-1;

            if(str[len-2] == '-'){
                // single digit seq no
                char ch = str[len-1];
                seq_no=ch-48;
                str[len-1]='\0';len--;
            }
            else if(str[len-3] == '-'){
                // double digit seq no
                char ch1 = str[len-1];
                char ch2 = str[len-2];
                seq_no=(ch2-48)*10+ch1-48;
                str[len-1]='\0';len--;
                str[len-1]='\0';len--;
            }

            if(seq_no == shared_memory[sockfd].last_seq_no){
                P(csmutex);
                set_null(shared_memory[sockfd].recv_buffer[w],MAX_MESSAGE_SIZE);
                str[len-1]='\0';len--;
                // shared_memory[0].send_max++;
                shared_memory[sockfd].wr2++;
                // printf("send_max after rcv %d\n",shared_memory[sockfd].send_max);
                shared_memory[sockfd].last_seq_no = (shared_memory[sockfd].last_seq_no+1)%16;

                // printf("::\n");
                // for(int y=0;y<MAX_RECV_BUF_SZ;y++){
                //     printf("%s %d\n",shared_memory[sockfd].recv_buffer[y],strlen(shared_memory[sockfd].recv_buffer[y]));
                // }

                // int *ab = shared_memory[sockfd].rwnd.sequence_numbers;
                // for(int k=0;k<MAX_RWND;k++){
                //     if(ab[k]==-2){
                //         ab[k] = shared_memory[sockfd].last_entry_rwnd;
                //         shared_memory[sockfd].last_entry_rwnd = (shared_memory[sockfd].last_entry_rwnd+1)%16;
                //         break;
                //     }
                // }

                V(csmutex);

                return str;
            }
        }
    }
}

int m_close(int sockfd){
    struct sembuf pop, vop;
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1; 
    vop.sem_op = 1;
    int key3 = ftok(".", 17);
    int csmutex = semget(key3, 1, 0777|IPC_CREAT);

    key_t shm_key = ftok(".", KEY_SM);
    int shm_id = shmget(shm_key, sizeof(SharedMemory)*25, IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("Error creating shared memory segment in m_bind");
        exit(EXIT_FAILURE);
    }
    SharedMemory *shared_memory = (SharedMemory *)shmat(shm_id, NULL, 0);

    P(csmutex);
    printf("Cleared up the MTP socket : %d PID = %d\n",sockfd,shared_memory[sockfd].process_id);
    shared_memory[sockfd].process_id = -1;
    shared_memory[sockfd].is_allocated = 0;
    V(csmutex);
}