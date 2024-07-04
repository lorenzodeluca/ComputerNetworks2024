f (argc != 2) { 
        printf("Devi specificare il dominio\n"); 
        return 1; 
    } 
 
    // Puntatore (facoltativo) su cui salviamo la status-line della response 
    char *statusline; 
 
    // Definiamo delle variabili temporanee utili per i socket e contatori 
    int socket_fd, temp, i, j, len; 
 
    // Puntatore a file 
    // Al momento è letteralmente inutile, non viene utilizzato 
    FILE *file; 
 
    // Buffer su cui salviamo la response del server 
    // Perché 1000001? 
    // Beh perché non conosciamo a priori la dimensione di tutto il body della response 
    // Potremmo però andare a definire la response subito dopo aver calcolato la dimensione dei chunk 
    // perché la somma delle dimensioni dei chunk in decimale è quella che effettivamente ci serve come response 
    // Questo però richiederrebbe un po' di lavoro per strutturarlo bene 
    char response[1000001]; 
 
    // Struttura dati che punta al server 
    struct sockaddr_in remote; 
 
    // Puntatore per gestire l'indirizzo IP del server a cui ci vogliamo connettere 
    unsigned char *p; 
 
    // Inizializzo la socket FD (File Descriptor) del client, potevamo chiamarla client_fd 
    socket_fd = socket(AF_INET, SOCK_STREAM, 0); 
    // Controllo degli errori nella creazione del socket 
    if (socket_fd == -1) { 
        // Se non va a buon fine terminiamo il programma 
        printf("errno = %d (%d)\n", errno, EAFNOSUPPORT); 
        perror("Socket fallita:"); 
        return 1; 
    } 
 
    // Compiliamo i campi della struct del socket del server 
    // Impostiamo IPv4 come formatp IP 
    remote.sin_family = AF_INET; 
 
    // htons() converte i valori tra host e networl in ordine di byte 
    // Impostiamo la porta PORT a 80 
    remote.sin_port = htons(80); 
 
    // Impostiamo l'IP del server con cui vogliamo comunicare 
    // Indirizzo IP del server (142.250.187.196) 
    char *hostname = argv[1]; 
    // Prendiamo solo l'hostname e non tutto l'URL 
    // strtok() divide la stringa in token 
    // strtok(stringa, delimitatore) 
    // Ora hostname punta alla stringa "www.domain.xx" 
    hostname = strtok(hostname + 7, "/"); 
 
    printf("Hostname: %s\n", hostname); 
 
    // Risoluzione dell'hostname 
    struct hostent *host_entity = gethostbyname(hostname); 
 
    // Controlliamo se la risoluzione 
    // dell'hostname ha avuto successo 
    if (host_entity == NULL) { 
        printf("Gethostbyname Fallita\n"); 
        return 1; 
    } 
 
    // Numero di indirizzi IP associati all'hostname 
    int addresses = 0; 
    while (host_entity->h_addr_list[addresses] != NULL) { 
        addresses++; 
    } 
 
    // Inizializza il generatore di numeri casuali 
    srand(time(NULL)); 
 
    // Genera un numero casuale tra 0 e X 
    // int address_index = (rand() % (addresses + 1)) ; 
    // Genera un numero casuale tra 0 e X - 1 
    int address_index = (rand() % (addresses)); 
 
    // Stampiamo l'indice dell'indirizzo IP scelto 
    printf("Indice scelto: %d, tra %d indirizzi\n", address_index, addresses); 
    printf("Indirizzo IP: %s\n", inet_ntoa(*(struct in_addr *)host_entity->h_addr_list[address_index])); 
 
    // remote.sin_addr.s_addr = *((unsigned int *)host_entity->h_addr_list[address_index]); 
    remote.sin_addr.s_addr = *((unsigned int *)host_entity->h_addr); 
 
    // Instauriamo una connessione con il server 
    if (connect(socket_fd, (struct sockaddr *)&remote, sizeof(struct sockaddr_in)) == -1) { 
        // Se non va a buon fine terminiamo il programma 
        perror("Connect fallita"); 
        return 1; 
    } 
 
    printf("Connessione stabilita\n"); 
 
    // Il primo \r\n termina l'header 
    // Il secondo \r\n termina il body 
    // Siccome non abbiamo scritto ne header ne body mettiamo \r\n\r\n 
    // Il compilatore mette in memoria questa stringa, terminatore e il puntatore che punta all'area di memoria 
    char request[1000]; 
 
    sprintf(request, "GET / HTTP/1.1\r\nHost:%s\r\n\r\n", inet_ntoa(remote.sin_addr)); 
 
    printf("Request: %s\n", request); 
 
    // FINE PARTE ESAME
