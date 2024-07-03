#include<stdlib.h>
#include<stdio.h>
#include<errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define CHUNKED_SIZE 20

char hbuf[10000];
char entity[1000];
struct headers{
         char * n;
         char * v;
} h[100];



struct sockaddr_in srvaddr,remote;

int main(){
	FILE * fin;
	char * method , *filename, *ver;
	char request[3001];
	char response[3001];
	int s,t,s2,len; 
	int yes=1;
	char * commandline;
	int i,j,a,c;
	s = socket(AF_INET, SOCK_STREAM, 0);
	if ( s == -1) {
		perror("Socket fallita");
		return 1;
		}
	t= setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int));
		if ( t == -1 ){
			perror("setsockopt fallita");
			return 1;
		}

	srvaddr.sin_family=AF_INET;
	srvaddr.sin_port=htons(25565);
	srvaddr.sin_addr.s_addr=0;

	t = bind(s,(struct sockaddr *)&srvaddr,sizeof(struct sockaddr_in));
	if ( t == -1) {
		perror("Bind fallita");
		return 1;
		}

	t = listen(s,5);
	if ( t == -1) {
		perror("Listen fallita");
		return 1;
		}
	len=sizeof(struct sockaddr);
	while( 1 ){
		close(s2);
		s2 = accept(s,(struct sockaddr *) &remote, &len); 
		if ( fork() ) continue;
		if (s2 == -1){ 
			perror("Accept fallita");
			return 1;
		}
		commandline = h[0].n=hbuf; //hbuf = request
		for(j=0,i=0; read(s2,hbuf+i,1);i++){
				if((hbuf[i]==':' ) && (h[j].v == NULL)){
						h[j].v=&hbuf[i+1];
						hbuf[i]=0;
				}
				if(hbuf[i]=='\n' && hbuf[i-1]=='\r'){
						hbuf[i-1]=0;
						if(h[j].n[0]==0) break;
						h[++j].n=&hbuf[i+1];
				}
		}

		for(i=0;i<j;i++){
			printf("%s ----> %s\n",h[i].n,h[i].v);
		}
		method = commandline;
		for(i=0;commandline[i]!=' ';i++) {} commandline[i]=0; i=i+1;
		filename = commandline+i;
		for(;commandline[i]!=' ';i++) {} commandline[i]=0; i=i+1;
		ver = commandline +i;
		for(;commandline[i]!=0;i++) {} commandline[i]=0; i=i+1;
		printf("Method = %s, URI = %s, VER = %s \n", method, filename,ver);

		fin = fopen(filename+1,"rt");
		if (fin == NULL){
			sprintf(response,"HTTP/1.1 404 NOT FOUND\r\nConnection:close\r\n\r\n<html><h1>File %s non trovato</h1></html>",filename);
			write(s2,response,strlen(response));
			close(s2);
			exit(1);
		} else {
       // buffer of CHUNKED_SIZE size
       char buffer[CHUNKED_SIZE];

       // telling the client that the data will be sent chunked
       sprintf(response,"HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
       write(s2, response, strlen(response));
       do {
               // Inizializzo la variabile a al valore 0
               a = 0;
               // Se a < della dimensione del buffer e il carattere preso dal
               // file non è EOF
               while((a < CHUNKED_SIZE) && (c = fgetc(fin)) != EOF)
                       // adding a char to  the buffer and after incrementing a
                       buffer[a++] = c;
               // adding to the response the value of a in hexadecimal
               sprintf(response,"%X\r\n",a);
               // sending the response to the client
               write(s2, response, strlen(response));
               // sending the chunk to the client
               write(s2, buffer, a);
               // adding to the response response \r\n
               sprintf(response,"\r\n");
               // sending to the client
               write(s2, response, strlen(response));

       }while(!feof(fin));  //continue till eof

       // close the file
       fclose(fin);
       // add to the response end values \0\r\n
       sprintf(response,"%X\r\n",0);
       // send to client
       write(s2, response, strlen(response));
       // adding connection end \r\n
       sprintf(response,"\r\n");
       // Invialo al client
       write(s2, response, strlen(response));
     }

		close(s2);
		exit(-1);
	}
}



