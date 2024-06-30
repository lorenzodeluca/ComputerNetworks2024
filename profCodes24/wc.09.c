#include <unistd.h>
#include <arpa/inet.h>
#include<stdio.h>
#include <sys/socket.h>
#include <errno.h>
int main() {

int s,t,i;
FILE * f;
char response[2000000];
struct sockaddr_in server;
unsigned char * p;


s = socket(AF_INET, SOCK_STREAM, 0 );
if ( s == -1 ) {
	printf("Errno = %d (%d)\n",errno,EAFNOSUPPORT);
	perror("Socket fallita:");
	return 1;
}
server.sin_family = AF_INET;
server.sin_port = htons(80);
p= (unsigned char*) &server.sin_addr.s_addr ;
p[0]=142;p[1]=250;p[2]=187; p[3]=196; // 142.250.187.196  
//p[0]=127;p[1]=0;p[2]=0; p[3]=1; // 142.250.187.196  
t = connect(s,(struct sockaddr *) &server, sizeof(struct sockaddr_in));
if ( t == -1) {
	perror("Connect fallita");
	return 1;
}
write(s,"GET /\r\n",7);
//sleep(2);
for(i=0; t = read(s,response+i,1999999-i);i+=t);
	//printf("t=%d\n",t); 


response[i]=0;
//printf("%s",response);
//printf("\n\ni=%d\n",i);

}
