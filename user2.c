#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "msocket.h" // Include the MTP socket library header

#define FILE_PATH "e1.txt" // Path to the file to be transferred


int main(int argc, char* argv[]) {

    if (argc != 5) {
        printf("The correct usage is: ./user <src_ip> <src_port> <dest_ip> <dest_port>\n");
        exit(0);
    }

    char *src_ip = argv[1];
    char *src_port = argv[2];
    char *dest_ip = argv[3];
    char *dest_port = argv[4];

    printf("Source IP: %s\n", src_ip);
    printf("Source Port: %d\n", atoi(src_port));
    printf("Destination IP: %s\n", dest_ip);
    printf("Destination Port: %d\n", atoi(dest_port));

    // Create an MTP socket
    m_socket(AF_INET, SOCK_MTP, 0);

    // Bind the MTP socket to local and remote IP and port
    struct sockaddr_in src_addr, dest_addr;
    memset(&src_addr, 0, sizeof(src_addr));
    memset(&dest_addr, 0, sizeof(dest_addr));

    src_addr.sin_family = AF_INET;
    src_addr.sin_port = htons(atoi(src_port)); // Local port
    inet_pton(AF_INET, src_ip, &src_addr.sin_addr); // Local IP

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(atoi(dest_port)); // Remote port
    inet_pton(AF_INET, dest_ip, &dest_addr.sin_addr); // Remote IP

    int sockfd = m_bind(src_addr, sizeof(src_addr), dest_addr, sizeof(dest_addr));
    if (sockfd == -1) {
        perror("Error binding MTP socket");
        exit(EXIT_FAILURE);
    }

    // Open the file to be transferred
    FILE *file = fopen(FILE_PATH, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);

    }

    // Read the file and send its contents using m_sendto
    // char buffer[1024];
    // set_null(buffer,1024);
    // size_t bytes_read;
    // while (fscanf(file, "%[^\n]", buffer) != EOF) {
    //     // printf("user is moaning: %d\n", strlen(buffer));
    //     // printf("user is moaning again: %d\n", bytes_read);
    //     printf("www %s\n",buffer);
    //     int send_result = m_sendto(sockfd, buffer, 1023, 0,
    //                                (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    //     if (send_result == -1) {
    //         perror("Error sending message");
    //         // exit(EXIT_FAILURE);
    //         break;
    //     }
    // }

    // Close the file
    fclose(file);

    // Close the MTP socket
    // int close_result = m_close(sockfd);
    // if (close_result == -1) {
    //     perror("Error closing MTP socket");
    //     exit(EXIT_FAILURE);
    // }

    printf("Pid of receiver: %d\n",getpid());

    while(1){
        sleep(3);

        char *str=m_recvfrom(sockfd,NULL,0,0,NULL,NULL);

        printf("Recv msg at user = %s\n",str);

    }

    return 0;
}
