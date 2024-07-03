#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// Buffer su cui salviamo gli header che leggiamo dalla response
char hbuf[10000];

// Buffer in cui salviamo il contenuto del file aperto tramite request da un client
char entity[10000];

// Definiamo una struct header per rapprensetare un
// singolo header dalla response
// Nome dell'header
// Valore dell'header
struct headers {
    char *name;
    char *value;
} h[100];  // Inizializziamo un array di 100 struct header dove salvare gli header letti

// Strutture dati che puntano al server stesso (host, questo PC)
// e ad un remote (client)
struct sockaddr_in host, client;

int main() {
    // Puntatore al file
    FILE *fin;

    // Puntatori per metodo, nome del file e versione HTTP
    char *method, *filename, *ver;

    // Buffer per la richiesta
    char request[3001];

    // Buffer per la risposta
    char response[3001];

    // Variabili per i socket e la lunghezza degli indirizzi
    int socket_fd, client_fd, temp, len;

    // Variabile per l'opzione del socket
    int yes = 1;

    // Puntatore alla linea di comando
    char *command_line;

    // Variabili di ciclo
    int i, j;

    // Inizializzo la socket FD (File Descriptor) del client,
    // potevamo chiamarla client_fd
    // In questo esempio vogliamo un server in
    // passive open (da mettere in ascolto)
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    // Controllo degli errori nella creazione del socket
    if (socket_fd == -1) {
        // Se non va a buon fine terminiamo il programma
        printf("errno = %d (%d)\n", errno, EAFNOSUPPORT);
        perror("Socket fallita:");
        return 1;
    }

    // Aggiorniamo le impostazioni del server
    // Mettiamo il flag SO_REUSEADDR
    // Questo permette al server di riutilizzare immediatamente l'indirizzo IP
    // e la port del socket anche se sono in uno stato di TIME_WAIT
    // TIME_WAIT: uno stato di chiusura di connessione
    // Questa impostazione permette il binding multiplo
    // ovvero piu socket ascoltano sulla stessa porta
    temp = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    // Controllo degli errori nella creazione del socket
    if (temp == -1) {
        // Se non va a buon fine terminiamo il programma
        perror("setsockopt fallita");
        return 1;
    }

    // Compiliamo i campi della struct del socket del server
    // Impostiamo IPv4 come formatp IP
    host.sin_family = AF_INET;

    // htons() converte i valori tra host e networl in ordine di byte
    // Impostiamo la porta PORT a 8077
    host.sin_port = htons(25565);

    // s_addr is variable that holds the information about
    // the address we agree to accept.
    // So, in this case we put INADDR_ANY because we would like to
    // accept connections from ANY internet address.
    // This case is used about server example.
    // In a client example i could NOT accept connections from ANY ADDRESS.
    host.sin_addr.s_addr = INADDR_ANY;

    // Inizializziamo il Binding (Assegnazione indirizzo):
    // Con la funzione bind() il socket viene associato a
    // uno degli indirizzi IP ()casuale del SERVER e al port scelto (ES: 8080)
    temp = bind(socket_fd, (struct sockaddr *)&host, sizeof(struct sockaddr_in));
    // Controllo degli errori nella creazione del socket
    if (temp == -1) {
        // Se non va a buon fine terminiamo il programma
        perror("Bind fallita");
        return 1;
    }

    // Ascolto connessioni (Rendere il socket passivo):
    // Il socket viene messo in ascolto per le connessioni in arrivo.
    // Così il socket è in ascolto passivo di nuove connessioni
    // sul suo port da parte dei client
    // Impostiamo a 5 la dimensione della coda delle connessioni
    temp = listen(socket_fd, 5);
    // Controllo degli errori nella creazione del socket
    if (temp == -1) {
        // Se non va a buon fine terminiamo il programma
        perror("Listen fallita");
        return 1;
    }

    // Salviamo la lunghezza della struttura sockaddr
    len = sizeof(struct sockaddr);
    // Ora l'host attende all'infinito per nuove connessioni
    // sul socket passivo in ascolto (sono nel padre)
    while (1) {
        // ACCEPT BLOCCANTE:
        // se c'era già una connessione aperta prima (sono nel padre),
        // questa viene chiusa, perché sta venendo gestita nel figlio
        close(client_fd);

        // Accettazione connessioni:
        // Quando c’è una richiesta di connessione nella coda,
        // il server estrae e accetta la prima connessione nella coda FIFO
        // tramite la chiamata accept(), creando un nuovo socket
        // dedicato alla connessione con quel client.
        client_fd = accept(socket_fd, (struct sockaddr *)&client, &len);

        // Generazione del figlio:
        // Attraverso la funzione fork() il processo principale
        // crea un processo figlio uguale al processo padre.
        // Questo permette al server di continuare ad ascoltare
        // nuove connessioni, mentre il processo figlio gestisce
        // in parallelo la comunicazione con il client.
        //
        // La funzione fork() restituisce 0 se siamo nel processo figlio;
        // Altrimenti restituisce il PID_FIGLIO se siamo nel processo padre
        // Quindi se siamo nel padre (<-> fork()!=0),
        // allora passiamo alla iterazione successiva del while
        if (fork() != 0) continue;

        // Da qui in poi il programma viene eseguito solo dal figlio:
        // Se non c'era nessuna connessione in coda (accept() => -1),
        // allora termina il processo figlio
        if (client_fd == -1) {
            perror("Accept fallita");
            return 1;
        }
        // Altrimenti (se c'era una connessione in coda) serviamo la connessione

        // Da qui leggiamo la request fatta dal client

        // Leggiamo gli header dal client_fd, inseriamo ogni carattere
        // in hbuff e salviamo in h solo il nome e il valore dell'header
        // per intero, non spezzettato
        command_line = h[0].name = hbuf;
        for (i = 0, j = 0; read(client_fd, hbuf + i, 1); i++) {
            // printf("hbuffer char: %s\n", hbuf + i);      // Test per vedere il contenuto corrente di hbuffer
            // printf("hbuffer char: %s\n", &hbuf[i + 1]);  // Test per vedere il contenuto corrente di hbuffer

            // Se il carattere che stiamo valutando (del hbuffer) è uguale a :
            // E se non abbiamo ancora inizializzato
            // il valore dell'header corrente
            if ((hbuf[i] == ':') && (h[j].value == NULL)) {
                // Facciamo puntare il valore dell'header
                // all'indirizzo del buffer hbuffer successivo
                // a quello corrente (:)
                h[j].value = &hbuf[i + 1];

                // Sostituiamo gli : con \0 per indicare che
                // abbiamo finito il NOME dell'header corrente
                // (utile in fase di stampa)
                hbuf[i] = 0;
            }

            // Se il carattere corrente è uguale a n
            // e se quello precedente è uguale a r
            // (ovvero se termina l'header) (\r\n)
            if (hbuf[i] == '\n' && hbuf[i - 1] == '\r') {
                // Impostiamo il carattere i-1 a \O per terminare
                // il valore dell'header e avere gli header in
                // righe separate (utile in fase di stampa)
                hbuf[i - 1] = 0;

                // Se il nome dell'header è un carattere vuoto,
                // allora hbuffer vuoto, allora abbiamo finito gli header
                // e quindi ora inizia il body
                // (Questo è dovuto dalla riga di codice precedente)
                if (h[j].name[0] == 0) break;

                // Altrimenti impostiamo il puntatore
                // al nome del prossimo header
                h[++j].name = &hbuf[i + 1];
            }
        }

        // Iterazione sugli header letti per cercaregli header che ci interessano
        // La variabile j è rimasta impostata dal for-loop precedente
        // j ci indica il numero di header che abbiamo ricevuto
        char *etag = NULL;
        for (i = 0; i < j; i++) {
            // Stampiamo il nome dell'header ed il suo valore
            printf("%s ----> %s\n", h[i].name, h[i].value);

            // Se l'header è ETag allora salviamo il valore
            if (strcmp(h[i].name, "ETag") == 0) {
                etag = h[i].value;
            }
        }

        // Abbiamo tokenizzato la richiesta da parte del client

        // I seguenti for-loop prendono Method, URI e VER
        // dal body della request

        // METHOD
        // La variabile method ora punta all'inizio del body,
        // perché command_line punta all'inizio del body
        // La variabile method serve per capire quale
        // richiesta viene fatta (GET | POST | ...)
        // Es: GET /index.html HTTP/1.1
        method = command_line;
        for (i = 0; command_line[i] != ' '; i++);
        // Impostiamo il carattere i a \O per terminare la command_line
        command_line[i] = '\0';

        // FILENAME
        i = i + 1;
        filename = command_line + i;
        for (; command_line[i] != ' '; i++);
        // Impostiamo il carattere i a \O per terminare la command_line
        command_line[i] = 0;

        // VER
        i = i + 1;
        ver = command_line + i;
        for (; command_line[i] != 0; i++);
        // Impostiamo il carattere i a \O per terminare la command_line
        command_line[i] = 0;
        i = i + 1;
        // Method = GET
        // URI = path del file richiesto dal client nel server
        // VER = versione del protocollo
        printf("Method = %s, URI = %s, VER = %s \n", method, filename, ver);

        /* ESAME CON HEADER IF-MODIFIED-SINCE, NO ETAG
        // Controlliamo se il client ha richiesto un file con header If-Modified-Since
        // Se l'header If-Modified-Since è presente
        // Controlliamo se il file è stato modificato
        // Per farlo controlleremo se il suo sha256 è cambiato rispetto a quello
        // che abbiamo salvato in precedenza
        // Se non abbiamo mai salvato un sha256 allora lo calcoliamo e lo salviamo
        //
        // Se è cambiato allora invieremo il file al client
        // Altrimenti invieremo un messaggio 304 NOT MODIFIED
        for (i = 0; i < j; i++) {
            // Se l'header è If-Modified-Since
            if (strcmp(h[i].name, "If-Modified-Since") == 0) {
                // Variabile per il calcolo dell'hash sha256
                fin = popen("sha256sum %s", filename, "r");
                // Buffer per il valore sha256
                char sha256[64];
                // Leggiamo il primo valore sha256
                // 64 è la lunghezza di un sha256
                fgets(sha256, 64, fin);
                // Chiudiamo il file aperto
                pclose(fin);

                // Se il file non è stato modificato
                if (strcmp(sha256, h[1].value) == 0) {
                    // sprintf: scrive sul buffer response la seguente stringa
                    sprintf(response, "HTTP/1.1 304 NOT MODIFIED\r\nConnection:close\r\n\r\n");

                    // Scriviamo dal buffer response al socket server client_fd la response
                    write(client_fd, response, strlen(response));

                    // chiudi socket server client_fd e quindi la connessione con il client
                    close(client_fd);

                    // Usciamo il processo figlio senza errori
                    exit(1);
                }
            }
        }
        */

        // Define a buffer for the command
        char command[256];
        // Buffer per il valore sha256
        char sha256[64];

        // sprintf() copia il contenuto della stringa "sha256sum %s" in "command"ù
        // filename + 1 serve per saltare il primo carattere del filename, che è /
        sprintf(command, "sha256sum %s", filename + 1);

        // Stampa su terminale
        printf("eseguo comando %s\n", command);

        // Execute the command and open a pipe for reading its output
        FILE *fp = popen(command, "r");
        if (fp == NULL) {
            printf("Failed to run command\n");
            exit(1);
        }

        // Read the output (the SHA256 hash) from the command
        if (fscanf(fp, "%64s", sha256) == 1) {
            printf("SHA256: %s\n", sha256);
        } else {
            printf("Failed to read SHA256 hash\n");
        }

        // Close the pipe
        pclose(fp);

        // system() esegue il comando "command" su SHELL del SERVER,
        // restitituisce 0 se il comando non va a buon fine,
        // altrimenti un intero che identifica il comando nella shell
        // system(command);

        // Copia la stringa "/output" in "filename"
        // così (fuori da questo if) il programma,
        // invece di aprire il file indicato dall'URI originale,
        // cercherà di aprire il file "/output",
        // che contiene il risultato dell'esecuzione di "command"
        // e lo stamperà nella response
        // strcpy(sha256, "/output");

        // Se il file non è stato modificato
        if (strcmp(sha256, etag) == 0) {
            // sprintf: scrive sul buffer response la seguente stringa
            sprintf(response, "HTTP/1.1 304 NOT MODIFIED\r\nConnection:close\r\n\r\n");

            // Scriviamo dal buffer response al socket server client_fd la response
            write(client_fd, response, strlen(response));

            // chiudi socket server client_fd e quindi la connessione con il client
            close(client_fd);

            // Usciamo il processo figlio senza errori
            exit(1);
        }

        // Ora gestiamo la response del server al client
        // response: apriamo il file richiesto e restituiamo
        // il contenuto al client,
        // se il file non viene trovato restituiamo errore 404
        fin = fopen(filename + 1, "rt");
        if (fin == NULL) {
            // sprintf: scrive sul buffer response la seguente stringa
            sprintf(response, "HTTP/1.1 404 NOT FOUND\r\nConnection:close\r\n\r\n<html><h1>File %s non trovato</h1>i</html>", filename);

            // Scriviamo dal buffer response al socket server client_fd la response
            write(client_fd, response, strlen(response));

            // chiudi socket server client_fd e quindi la connessione con il client
            close(client_fd);

            // Usciamo il processo figlio con errore
            exit(1);
        }

        // sprintf: scrive sul buffer response la seguente stringa
        sprintf(response, "HTTP/1.1 200 OK\r\nConnection:close\r\n\r\n");

        // Scriviamo dal buffer response al socket server client_fd la response
        write(client_fd, response, strlen(response));

        // Leggiamo dal file aperto fino a che non raggiunge la fine
        // e salva sul buffer entity
        while (!feof(fin)) {
            // Leggiamo 1000 byte (char) e inseriamo il contenuto dal file fin al buffer entity
            // Stiamo "consumando" fin
            fread(entity, 1, 1000, fin);

            // Scriviamo 1000 byte (char) sul socket client_fd il contenuto del file
            write(client_fd, entity, 1000);
        }

        // Chiudiamo il file aperto dalla richiesta
        fclose(fin);

        // Chiudiamo la connessione con il client
        close(client_fd);

        // Usciamo dal processo figlio senza errori
        exit(-1);
    }

    // Chiudiamo la socket principale
    close(socket_fd);
    return 0;
}

