#include <unistd.h>
#include <arpa/inet.h>
#include<stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <malloc.h>
#include <sys/stat.h>   // stat to check if file exist
struct hostent * he; 

char hbuf[10000];
struct headers{
   char * n;
   char * v;
} h[100];

int main() {
   char * statusline;
   int s,t,i,j;
   FILE * f;
   char response[4000000];
   struct sockaddr_in server;
   unsigned char * p;
   char * cacheDir="./cache/";
 char ip_str[INET_ADDRSTRLEN]; 
   char * filename="/index.html"; 
   char * hostname="google.it";
   s = socket(AF_INET, SOCK_STREAM, 0 );
   if ( s == -1 ) {
      printf("Errno = %d (%d)\n",errno,EAFNOSUPPORT);
      perror("Socket fallita:");
      return 1;
   }
   server.sin_family = AF_INET;
   server.sin_port = htons(80);
   p= (unsigned char*) &server.sin_addr.s_addr;server.sin_family = AF_INET;

   server.sin_port = htons(80); 
   p= (unsigned char*) &server.sin_addr.s_addr ; 
   he = gethostbyname(hostname); 

   p[0]=(unsigned char) he->h_addr[0]; 
   p[1]=(unsigned char) he->h_addr[1]; 
   p[2]=(unsigned char) he->h_addr[2]; 
   p[3]=(unsigned char) he->h_addr[3]; 
   //p[0]=142;p[1]=251;p[2]=40; p[3]=227; // 142.250.187.196  
   //p[0]=127;p[1]=0;p[2]=0; p[3]=1; // 142.250.187.196  
   
   
    inet_ntop(AF_INET, &(server.sin_addr), ip_str, INET_ADDRSTRLEN); //convert ip inside server to str 
    printf("Indirizzo IP: %s\n", ip_str);

   t = connect(s,(struct sockaddr *) &server, sizeof(struct sockaddr_in));
   if ( t == -1) {
      perror("Connect fallita");
      return 1;
   }
   char * request = "GET /index.html HTTP/1.0\r\n\r\n";
   write(s,request,strlen(request));
   //sleep(2);
   statusline = h[0].n=hbuf;

   //parsing headers
   int lastModifiedIndex=-1; 
   for(j=0,i=0; read(s,hbuf+i,1);i++){
      if((hbuf[i]==':' ) && (h[j].v == NULL)){
         h[j].v=&hbuf[i+1];
         hbuf[i]=0;
      }
      if(hbuf[i]=='\n' && hbuf[i-1]=='\r'){
         hbuf[i-1]=0;
         if(h[j].n[0]==0) break;
         h[++j].n=&hbuf[i+1];
         if(strstr(h[j].n, "Last-Modified")!=NULL){//strstr look for the second param in the first and returns a pointer to the first occurence in the first:)
            lastModifiedIndex=j;
         }
      }
   }

   for(i=0;i<j;i++)
      printf("%s ----> %s\n",h[i].n,h[i].v);
   

   //caching 
   int len = snprintf(NULL, 0, "%s%s", hostname, filename);
   char *url = malloc(len + 1); // +1 per il carattere nullo terminatore
   sprintf(url,"%s%s\0",hostname,filename); 
   printf("\n\n-------\nURL: %s\nlast modified:%s",url,h[lastModifiedIndex].v);
   char cachename[len];
   sprintf(cachename,"%s",url);
   for(int i=0;cachename[i]!=0;i++)if(cachename[i]=='/')cachename[i]='_';
   printf("\nfilename ----> %s", cachename);
   struct stat buffer;
   if(stat(cachename, &buffer) == 0){
      
   }


   for(i=0; t = read(s,response+i,1999999-i);i+=t);
   //printf("t=%d\n",t); 

   response[i]=0;
   printf("\n---REPONSE----\n%s",response);

      //printf("\n\ni=%d\n",i);

}
