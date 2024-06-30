#include <unistd.h>
#include <arpa/inet.h>
#include<stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>

char hbuf[10000];
struct headers{
	 char * n;
	 char * v;
} h[100];

int main() {
char * statusline;
int s,t,i,j;
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
char * request = "GET /pppp.html HTTP/1.0\r\n\r\n";
write(s,request,strlen(request));
//sleep(2);
statusline = h[0].n=hbuf;
for(j=0,i=0; read(s,hbuf+i,1);i++){
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

for(i=0;i<j;i++)
	printf("%s ----> %s\n",h[i].n,h[i].v);

for(i=0; t = read(s,response+i,1999999-i);i+=t);
	//printf("t=%d\n",t); 

response[i]=0;
printf("%s",response);
//printf("\n\ni=%d\n",i);

}
