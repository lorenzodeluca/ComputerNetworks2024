//21:57
#include<stdlib.h>
#include<stdio.h>
#include<errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>


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
   int i,j;
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
      commandline = h[0].n=hbuf;

      int referrerIndex=-1;
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
         if(strstr(h[j].n, "Referer")!=NULL){//strstr look for the second param in the first and returns a pointer to the first occurence in the first:)
            referrerIndex=j;
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

      //check if requested Referrer blacklisted 
      if(referrerIndex != -1){
         FILE *fd;
         char buf[200];
         char *res;


         /* apre il file */
         fd=fopen("blacklist.txt", "r");
         if( fd==NULL ) {
            perror("Errore in apertura del file");
            exit(1);
         }
         /* legge ogni riga */
         while(1) {
            //printf("buf:-%s-, ref;-%s-,blk;-%d-\n",buf,h[referrerIndex].v,strstr(buf,h[referrerIndex].v+1));
            res=fgets(buf, 200, fd);
            if( res==NULL )
               break;
            if(strstr(buf,h[referrerIndex].v+1)!=NULL){
               printf("\n\n------------->BLACKLISTEEEEED<--------------\n\n");
               sprintf(response,"HTTP/1.1 302 Moved Temporarily\r\nLocation = %s\r\nConnection:close\r\n\r\n",h[referrerIndex].v);
               close(s2);
               exit(1);
            }

         }


         /* chiude il file */
         fclose(fd);
      }

      fin = fopen(filename+1,"rt");
      if (fin == NULL){
         sprintf(response,"HTTP/1.1 404 NOT FOUND\r\nConnection:close\r\n\r\n<html><h1>File %s non trovato</h1>i</html>",filename);
         write(s2,response,strlen(response));
         close(s2);
         exit(1);
      }
      sprintf(response,"HTTP/1.1 200 OK\r\nConnection:close\r\n\r\n");
      write(s2,response,strlen(response));
      while (!feof(fin)){
         fread(entity,1,1000,fin);
         write(s2,entity,1000);
      }
      fclose(fin);
      close(s2);
      exit(-1);
   }
}




