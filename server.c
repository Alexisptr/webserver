#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netdb.h>
#include <netinet/in.h>

#define BUFFER_SIZE 4096

void *method(void *socket_desc)
{
    int sock = *(int*)socket_desc;
    int fd, read_value, bytes_received, i, file_size = 0;
    char buffer[BUFFER_SIZE], client_message[BUFFER_SIZE];
    bytes_received = read(sock, client_message, BUFFER_SIZE);
    FILE *fp;

    char delim[] = " ";
    char *ptr = strtok(client_message, delim);
    char *method, *file_name = "";

    // Parsing client_message from navigator
    for (i = 0; ptr != NULL; i++) {
        if (i == 0) {
            method = ptr;
        }
        else if (i == 1) {
            file_name = ptr;
        }
        ptr = strtok(NULL, delim);
    }

    // Redirection
    if (!strcmp(file_name, "/")) {
        file_name = "index.html";
    }
    else {
        memmove(file_name, file_name + 1, strlen(file_name));
    }

    if (!strcmp(method, "GET")) {
        if ((fd = open(file_name, O_RDONLY)) != -1) {
            char header[] =
            "HTTP/1.1 200 OK\n"
            "Content-Type: text/html; charset=UTF-8\n\n";
            write(sock, header, strlen(header));
            while ((read_value = read(fd, buffer, BUFFER_SIZE)) > 0) {
                buffer[read_value] = '\0';
                write(sock, buffer, read_value);
            }

            close(fd);
        }
        else {
            if ((fd = open("./404.html", O_RDONLY)) != -1) {
                char header[] =
                "HTTP/1.1 404 NOT FOUND\n"
                "Content-Type: text/html; charset=UTF-8\n\n";
                write(sock, header, strlen(header));
                while ((read_value = read(fd, buffer, BUFFER_SIZE)) > 0) {
                    buffer[read_value] = '\0';
                    write(sock, buffer, read_value);
                }

                close(fd);
            }
        }
    }
    else if (!strcmp(method, "PUT")) {
        read_value = read(sock, buffer, BUFFER_SIZE);
        buffer[read_value] = '\0';
        if (!(fp = fopen(file_name , "r"))) {
            fp = fopen(file_name , "w");
            fwrite(buffer, 1, sizeof(buffer), fp);
            char header[] =
            "HTTP/1.1 201 Created\n"
            "Content-Type: text/html; charset=UTF-8\n\n";
            write(sock, header, strlen(header));
        }
        else {
            char header[] =
            "HTTP/1.1 204 Existing\n"
            "Content-Type: text/html; charset=UTF-8\n\n";
            write(sock, header, strlen(header));
        }
        fclose(fp);
    }

    close(sock);
    pthread_exit(NULL);
}

int main(int argc , char *argv[])
{
    int master_socket, addrlen, new_socket, i, sock, max_sd, max_clients = 10;
    struct sockaddr_in address;
    char buffer[BUFFER_SIZE];
    int socket_tab[10] = {0};
    fd_set rdfs;
    pthread_t threads[10];

    master_socket = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(8080);

    bind(master_socket, (struct sockaddr *)&address, sizeof(address));
    printf("Listener on port 8080\n");

    listen(master_socket, max_clients);

    addrlen = sizeof(address);

    while (1) {
            FD_ZERO(&rdfs);

            FD_SET(master_socket, &rdfs);
            max_sd = master_socket;

            for (i = 0; i < max_clients; i++) {
                sock = socket_tab[i];

                if (sock > 0)
                    FD_SET(sock, &rdfs);

                if (sock > max_sd)
                    max_sd = sock;
            }

            select(max_sd + 1, &rdfs, NULL, NULL, NULL);

            if (FD_ISSET(master_socket, &rdfs)) {
                new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen);

                pthread_create(&threads[i], NULL, method, (void*) &new_socket);
                printf("New connection from port %d with thread id %d\n", ntohs(address.sin_port), threads[i]);

                for (i = 0; i < max_clients; i++) {
                    if (socket_tab[i] == 0) {
                        socket_tab[i] = new_socket;
                        break;
                    }
                }
            }
        }

    return EXIT_SUCCESS;
}
