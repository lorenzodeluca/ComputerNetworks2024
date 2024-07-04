//iniziato 16:20->
#include <unistd.h>
#include <arpa/inet.h>
#include<stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <malloc.h>
#include <sys/stat.h>   // stat to check if file exist
#include <time.h>


char hbuf[10000];
struct headers{
   char * n;
   char * v;
} h[100];
struct hostent * he; 
int main(int argc, char* argv[]) {
   if(argc!=2){
     printf("Invalid parameters");
     return 1;
   }
   char * url=argv[1];
   printf("%s",url);

   char hostname[256];
   char filename[256]; 
   // Trova l'inizio della parte significativa dell'URL
   char *start = strstr(url, "://");
   start += 3; // salta "://"
   char *slash = strchr(start, '/');// Trova il primo '/'

   size_t hostname_length = slash - start;
   strncpy(hostname, start, hostname_length); // Copia l'hostname
   hostname[hostname_length] = '\0'; // Assicurati che sia NULL-terminato

   char *last_slash = strrchr(url, '/');     // Trova l'ultimo '/'
   strcpy(filename, last_slash);   // Copia il filename
   printf("\nhostname:%s\nfilename=%s\n",hostname,filename);

   char * statusline;
   int s,t,i,j,len;
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

   he = gethostbyname(hostname); 

   // Numero di indirizzi IP associati all'hostname 
    int addresses = 0; 
    while (he->h_addr_list[addresses] != NULL) { 
        addresses++; 
    } 
 
    // Inizializza il generatore di numeri casuali 
    srand(time(NULL)); 
 
    // Genera un numero casuale tra 0 e X 
    // int address_index = (rand() % (addresses + 1)) ; 
    // Genera un numero casuale tra 0 e X - 1 
    int address_index = (rand() % (addresses)); 
 
   p[0]=(unsigned char) he->h_addr_list[address_index][0]; 
   p[1]=(unsigned char) he->h_addr_list[address_index][1]; 
   p[2]=(unsigned char) he->h_addr_list[address_index][2]; 
   p[3]=(unsigned char) he->h_addr_list[address_index][3]; 
   
  printf("Indirizzo IP: %s\n", inet_ntoa(*(struct in_addr *)he->h_addr_list[address_index]));  

   //p[0]=142;p[1]=250;p[2]=187; p[3]=196; // 142.250.187.196  
   //p[0]=127;p[1]=0;p[2]=0; p[3]=1; // 142.250.187.196  
   t = connect(s,(struct sockaddr *) &server, sizeof(struct sockaddr_in));
   if ( t == -1) {
      perror("Connect fallita");
      return 1;
   }
   char request[100000];
   sprintf(request,"GET /%s HTTP/1.1\r\nHost:%s\r\n\r\n\0",filename,hostname);
   write(s,request,strlen(request));
   //sleep(2);
   statusline = h[0].n = hbuf;
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

   for(i=0;i<j;i++){
      printf("%s ----> %s\n",h[i].n,h[i].v);
      if(!strcmp(h[i].n,"Content-Length")){
         len = atoi(h[i].v);
         printf("len = %d\n", len);
      }
   }
   if (len == 0)
      len = 10000;

   for(i=0; t = read(s,response+i,len-i);i+=t);
   //printf("t=%d\n",t); 
   response[i]=0;
   printf("%s",response);
   //printf("\n\ni=%d\n",i);

}
