#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>

int pid;
struct sockaddr_in local, remote, server;
char request[10000];
char request2[10000];
char response[1000];
char response2[10000];

struct header {
  char * n;
  char * v;
} h[100];

struct hostent * he;

// Function to send request and follow redirects if necessary
void handle_request(int s2, char *request_line);

int main() {
    char hbuffer[10000];
    char * reqline;
    char * method, *url, *ver;
    int s, s2;
    int yes = 1;
    int len;
    
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("errno = %d\n", errno); perror("Socket Fallita"); return -1;
    }
    local.sin_family = AF_INET;
    local.sin_port = htons(20161);
    local.sin_addr.s_addr = 0;

    int t = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if (t == -1) { perror("setsockopt fallita"); return 1; }

    if (bind(s, (struct sockaddr *)&local, sizeof(struct sockaddr_in)) == -1) {
        perror("Bind Fallita"); return -1;
    }

    if (listen(s, 10) == -1) { perror("Listen Fallita"); return -1; }

    remote.sin_family = AF_INET;
    remote.sin_port = htons(0);
    remote.sin_addr.s_addr = 0;
    len = sizeof(struct sockaddr_in);
    
    while (1) {
        s2 = accept(s, (struct sockaddr *)&remote, &len);
        printf("Remote address: %.8X\n", remote.sin_addr.s_addr);
        if (fork()) continue;
        if (s2 == -1) { perror("Accept fallita"); exit(1); }
        bzero(hbuffer, 10000);
        bzero(h, 100 * sizeof(struct header));
        reqline = h[0].n = hbuffer;
        
        int i;
        for (i = 0; read(s2, hbuffer + i, 1); i++) {
            printf("%c", hbuffer[i]);
            if (hbuffer[i] == '\n' && hbuffer[i - 1] == '\r') {
                hbuffer[i - 1] = 0; // Terminate the token
                if (!h->n[0]) break;
                h[++i].n = hbuffer + i + 1;
            }
            if (hbuffer[i] == ':' && !h->v && i > 0) {
                hbuffer[i] = 0;
                h->v = hbuffer + i + 1;
            }
        }

        printf("Request line: %s\n", reqline);
        method = reqline;
        for (i = 0; reqline[i] != ' '; i++);
        reqline[i++] = 0;
        url = reqline + i;
        for (; reqline[i] != ' '; i++);
        reqline[i++] = 0;
        ver = reqline + i;
        for (; reqline[i] != '\r'; i++);
        reqline[i++] = 0;

        if (strcmp(method, "GET") == 0) {
            handle_request(s2, reqline);
        } else if (strcmp(method, "CONNECT") == 0) {
            // Handle CONNECT method as per original code
            hostname = url;
            for (i = 0; url[i] != ':'; i++);
            url[i] = 0;
            port = url + i + 1;
            printf("hostname:%s, port:%s\n", hostname, port);
            he = gethostbyname(hostname);
            if (he == NULL) { printf("Gethostbyname Fallita\n"); return 1; }
            printf("Connecting to address = %u.%u.%u.%u\n", (unsigned char )he->h_addr[0], (unsigned char )he->h_addr[1], (unsigned char )he->h_addr[2], (unsigned char )he->h_addr[3]); 
            s3 = socket(AF_INET, SOCK_STREAM, 0);

            if (s3 == -1) { perror("Socket to server fallita"); return 1; }
            server.sin_family = AF_INET;
            server.sin_port = htons((unsigned short)atoi(port));
            server.sin_addr.s_addr = *(unsigned int*) he->h_addr;
            t = connect(s3, (struct sockaddr *)&server, sizeof(struct sockaddr_in));
            if (t == -1) { perror("Connect to server fallita"); exit(0); }
            sprintf(response, "HTTP/1.1 200 Established\r\n\r\n");
            write(s2, response, strlen(response));
            
            if (!(pid=fork())) { // child
                while (t = read(s2, request2, 2000)) {	
                    write(s3, request2, t);
                } 
                exit(0);
            } else { // parent
                while (t = read(s3, response2, 2000)) {
                    write(s2, response2, t);
                }
                kill(pid, SIGTERM);
                close(s3);
            }
        } else {
            sprintf(response, "HTTP/1.1 501 Not Implemented\r\n\r\n");
            write(s2, response, strlen(response));
        }
        close(s2);
        exit(1);
    }
    close(s);
    return 0;
}

void handle_request(int s2, char *request_line) {
    char hbuffer[10000];
    char buffer[2000];
    struct sockaddr_in server;
    struct hostent *he;
    int s3;
    char *method, *url, *ver, *scheme, *hostname, *filename;
    char *location;
    int i, j, t;
    
    method = strtok(request_line, " ");
    url = strtok(NULL, " ");
    ver = strtok(NULL, "\r");

    // Process URL
    scheme = url;
    for (i = 0; url[i] != ':' && url[i]; i++);
    if (url[i] == ':') url[i++] = 0;
    else { printf("Parse error, expected ':'"); exit(1); }
    if (url[i] != '/' || url[i+1] != '/') { printf("Parse error, expected '//'"); exit(1); }
    i = i + 2;
    hostname = url + i;
    for (; url[i] != '/' && url[i]; i++);
    if (url[i] == '/') url[i++] = 0;
    else { printf("Parse error, expected '/'"); exit(1); }
    filename = url + i;
    printf("Scheme: %s, hostname: %s, filename: %s\n", scheme, hostname, filename);

    he = gethostbyname(hostname);
    if (he == NULL) { printf("Gethostbyname Fallita\n"); exit(1); }

    if ((s3 = socket(AF_INET, SOCK_STREAM, 0)) == -1) { 
        printf("errno = %d\n", errno); perror("Socket Fallita"); exit(1);
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(80);
    server.sin_addr.s_addr = *(unsigned int *)(he->h_addr);

    if (connect(s3, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == -1) {
        perror("Connect Fallita"); exit(1);
    }

    sprintf(request, "GET /%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n\r\n", filename, hostname);
    printf("%s\n", request);
    write(s3, request, strlen(request));

    int status = 0;
    while ((t = read(s3, buffer, sizeof(buffer))) > 0) {
        buffer[t] = 0;
        printf("Server Response:\n%s\n", buffer);

        if (sscanf(buffer, "HTTP/1.1 %d", &status) == 1 && (status == 301 || status == 302)) {
            location = strstr(buffer, "Location:");
            if (location) {
                location += 10; // Skip "Location: "
                for (i = 0; location[i] != '\r' && location[i] != '\n'; i++);
                location[i] = 0;
                strncpy(url, location, sizeof(url) - 1);
                url[i] = 0;
                close(s3); // Close the current connection
                handle_request(s2, method + " " + url + " HTTP/1.1"); // Redirect to new location
                return;
            }
        } else if (status == 200) {
            write(s2, buffer, t);
        }
    }
    close(s3);
}
