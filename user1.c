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
    char buffer[1024];
    set_null(buffer,1024);
    size_t bytes_read;
    int opt;
    int flag = 1;

    printf("Pid of sender : %d\n",getpid());
    int ff=0;


    while(1) {
        printf("Enter your choice: 1-read ; else exit : ");
        scanf("%d", &opt);
        
        if(opt == 1) {
            while (1) {
                // printf("Here1\n");
                sleep(1);

                if(ff==1){
                    printf("File sent already\n");
                    break;;
                }

                if (flag && (fscanf(file, "%s", buffer) == EOF)) {
                    printf("File sent\n");
                    ff=1;
                    fclose(file);
                    break;
                }
                // printf("Here2\n");
                // sleep(1);
                int send_result = m_sendto(sockfd, buffer, 1023, 0,
                                        (struct sockaddr *)&dest_addr, sizeof(dest_addr));
                if (send_result == -1) {
                    printf("Error sending message\n");
                    flag = 0;
                    // exit(EXIT_FAILURE);
                    break;
                } else {
                    flag = 1;
                }
                set_null(buffer,1024);
            }
        } else {
            break;
        }
    }


    printf("Sender waiting for all data to be transferred to the receiver\n");

    while(1){
        sleep(5);
    }

    return 0;
}
