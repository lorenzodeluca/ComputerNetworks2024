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
struct hostent * he; 

char hbuf[10000];
struct headers{
   char * n;
   char * v;
} h[100];

int main() {
   char * statusline;
   int s,t,i,j,len=0;
   FILE * f;
   char response[4000000];
   struct sockaddr_in server;
   unsigned char * p;
   char * cacheDir="cache/";
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
    
   char * request;
   int reqLen=snprintf(NULL, 0, "GET %s HTTP/1.1\r\nHost:%s\r\n\r\n",filename,hostname);
   request=malloc(reqLen + 1);
   sprintf(request,"GET %s HTTP/1.1\r\nHost:%s\r\n\r\n",filename,hostname);
   printf("\nporcodiaz\n");
   printf("request ---> %s",request);
   write(s,request,strlen(request));
   //sleep(2);
   statusline = h[0].n=hbuf;

   //parsing headers
   int ETagIndex=-1; 
   for(j=0,i=0; read(s,hbuf+i,1);i++){
      if((hbuf[i]==':' ) && (h[j].v == NULL)){
         h[j].v=&hbuf[i+1];
         hbuf[i]=0;
      }
      if(hbuf[i]=='\n' && hbuf[i-1]=='\r'){
         hbuf[i-1]=0;
         if(h[j].n[0]==0) break;
         h[++j].n=&hbuf[i+1];
         if(strstr(h[j].n, "ETag")!=NULL){//strstr look for the second param in the first and returns a pointer to the first occurence in the first:)
            ETagIndex=j;
         }
      }
      if(!strcmp(h[i].n,"Content-Length")){
         len = atoi(h[i].v);
         printf("len = %d\n", len);
      }
   }
   if (len == 0)
      len = 10000;


   for(i=0;t=read(s,response+i,len-i);i+=t);
   printf("%s ----> %s\n",h[i].n,h[i].v);


   //caching 
   int lenUrl = snprintf(NULL, 0, "%s%s", hostname, filename);
   char *url = malloc(lenUrl + 1); // +1 per il carattere nullo terminatore
   sprintf(url,"%s%s\0",hostname,filename); 
   printf("\n\n-------\nURL: %s\netag:%s",url,h[ETagIndex].v);
   char cachename[lenUrl];
   sprintf(cachename,"%s",url);
   for(int i=0;cachename[i]!=0;i++)if(cachename[i]=='/')cachename[i]='_';
   printf("\nfilename ----> %s", cachename);

   int fileLen=snprintf(NULL, 0, "%s%s", cacheDir, filename);
   char *cachePath = malloc(lenUrl + 1); // +1 per il carattere nullo terminatore
   sprintf(cachePath,"%s%s\0",cacheDir,cachename); 
   printf("\ncache: %s",cachePath);
   struct stat buffer;



   if(stat(cachePath, &buffer) != 0){
      printf("\n\n---new url not cached---\n\n");
      // Apriamo il file in modalità di scrittura. Se il file sarà creato.
      FILE *file = fopen(cachePath, "w");

      // Controlliamo che il file sia stato aperto correttamente
      if (file == NULL) {
         printf("Errore nell'apertura del file.\n");
         return 1;
      }

      // Scriviamo il contenuto della variabile etag nel file
      fprintf(file, "%s\n", h[ETagIndex].v);

      //read response
      for(i=0; t =read(s,response+i,len-i);i+=t);
      //printf("t=%d\n",t); 

      // Scriviamo il contenuto della variabile str nel file
      fprintf(file, "%s", response);

      // Chiudiamo il file
      fclose(file);
   }else{ // file esiste già
      printf("\n\n--- url cached--\n\n");

      //ottengo l etag nella prima riga del file
      char firstLine[256];              // Buffer per la prima riga

      char *cachedVersion = NULL;       // Variabile per il resto del file
      FILE *file = fopen(cachePath, "r");
      if (file == NULL) {
         perror("Errore nell'apertura del file");
         return 1;
      }

      // Leggere la prima riga
      if (fgets(firstLine, sizeof(firstLine), file) == NULL) {
         perror("Errore nella lettura del file");
         fclose(file);
         return 1;
      }

      // Rimuovere l'eventuale carattere di fine riga
      firstLine[strcspn(firstLine, "\r\n")] = '\0';

      // Calcolare la lunghezza del resto del file
      fseek(file, 0, SEEK_END);   
      long fileSize = ftell(file);   
      fseek(file, strlen(firstLine) + 1, SEEK_SET); // Spostarsi dopo la prima riga

      // Leggere il resto del file
      long cachedSize = fileSize - (strlen(firstLine) + 1);
      cachedVersion = (char *)malloc(cachedSize + 1); // +1 per il terminatore NULL

      if (cachedVersion == NULL) {
         perror("Errore nell'allocazione della memoria");
         fclose(file);
         return 1;
      }
      fread(cachedVersion, 1, cachedSize, file);
      cachedVersion[cachedSize] = '\0'; // Aggiungere il terminatore NULL
      fclose(file);
      if(ETagIndex==-1 || h[ETagIndex].v!=firstLine){ //date=Mon, 03 Aug 2024 21:58:30 GMT
         printf("\n\n---NOT USING CACHED VERSION---\n\n");
         //read response from http call
         for(i=0; t = read(s,response+i,1999999-i);i+=t);
         //printf("t=%d\n",t); 

      }else{ //use cached
         printf("\n\n---using cached VERSIONNNNN---\n\n");
         sprintf(response,"%s",cachedVersion);
      }
   }




   response[i]=0;
   printf("\n---RESPONSE----\n%s",response);

   //printf("\n\ni=%d\n",i);

}
