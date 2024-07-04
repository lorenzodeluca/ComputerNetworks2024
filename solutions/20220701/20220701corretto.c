#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>

char hbuf[10000];
char entity[1000];
struct headers {
    char *n;
    char *v;
} h[100];

struct sockaddr_in srvaddr, remote;

void handle_request(int s2, int *transaction_counter) {
    FILE *fin;
    char *method, *filename, *ver;
    char request[3001];
    char response[3001];
    char *commandline;
    int i, j;

    while (1) {
        bzero(hbuf, sizeof(hbuf));
        bzero(h, sizeof(h));

        commandline = h[0].n = hbuf;
        for (j = 0, i = 0; read(s2, hbuf + i, 1); i++) {
            if ((hbuf[i] == ':') && (h[j].v == NULL)) {
                h[j].v = &hbuf[i + 1];
                hbuf[i] = 0;
            }
            if (hbuf[i] == '\n' && hbuf[i - 1] == '\r') {
                hbuf[i - 1] = 0;
                if (h[j].n[0] == 0) break;
                h[++j].n = &hbuf[i + 1];
            }
        }

        if (h[0].n[0] == 0) {
            break; // No more data, close the connection
        }

        method = commandline;
        for (i = 0; commandline[i] != ' '; i++) {} commandline[i] = 0; i = i + 1;
        filename = commandline + i;
        for (; commandline[i] != ' '; i++) {} commandline[i] = 0; i = i + 1;
        ver = commandline + i;
        for (; commandline[i] != 0; i++) {} commandline[i] = 0; i = i + 1;

        // Print the requested file and transaction counter
        printf("%s Socket %d trans %d\n", filename, s2, *transaction_counter);

        fin = fopen(filename + 1, "rt");
        if (fin == NULL) {
            sprintf(response, "HTTP/1.1 404 NOT FOUND\r\nConnection:close\r\n\r\n<html><h1>File %s non trovato</h1></html>", filename);
            write(s2, response, strlen(response));
            continue;
        }

        sprintf(response, "HTTP/1.1 200 OK\r\nConnection:close\r\n\r\n");
        write(s2, response, strlen(response));
        while (!feof(fin)) {
            int bytesRead = fread(entity, 1, 1000, fin);
            if (bytesRead > 0) {
                write(s2, entity, bytesRead);
            }
        }
        fclose(fin);

        (*transaction_counter)++; // Increment the transaction counter after each request

        shutdown(s2, SHUT_WR); // Indicate that no more data will be sent but still can receive
    }
}

int main() {
    int s, t, len;
    int new_s2, max_fd;
    int transaction_counter[FD_SETSIZE];
    int yes = 1;
    fd_set master_set, read_fds;

    for (int i = 0; i < FD_SETSIZE; i++) {
        transaction_counter[i] = 0;
    }

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1) {
        perror("Socket failed");
        return 1;
    }

    t = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if (t == -1) {
        perror("setsockopt failed");
        return 1;
    }

    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(25565);
    srvaddr.sin_addr.s_addr = INADDR_ANY;

    t = bind(s, (struct sockaddr *)&srvaddr, sizeof(struct sockaddr_in));
    if (t == -1) {
        perror("Bind failed");
        return 1;
    }

    t = listen(s, 5);
    if (t == -1) {
        perror("Listen failed");
        return 1;
    }

    FD_ZERO(&master_set);
    FD_SET(s, &master_set);
    max_fd = s;

    printf("Server started, waiting for connections...\n");

    while (1) {
        read_fds = master_set;
        t = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (t == -1) {
            perror("select failed");
            return 1;
        }

        for (int i = 0; i <= max_fd; i++) {
            if (FD_ISSET(i, &read_fds)) {
                if (i == s) { 
                    // New connection
                    len = sizeof(struct sockaddr_in);
                    new_s2 = accept(s, (struct sockaddr *)&remote, &len);
                    if (new_s2 == -1) {
                        perror("Accept failed");
                    } else {
                        FD_SET(new_s2, &master_set);
                        if (new_s2 > max_fd) max_fd = new_s2;
                        printf("New connection on socket %d\n", new_s2);
                    }
                } else {
                    // Data to handle on an existing connection
                    handle_request(i, &transaction_counter[i]);
                    // If there's a disconnection
                    if (recv(i, hbuf, sizeof(hbuf), 0) <= 0) {
                        printf("Connection closed on socket %d\n", i);
                        close(i);
                        FD_CLR(i, &master_set);
                    }
                }
            }
        }
    }

    return 0;
}
