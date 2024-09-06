#include <stdio.h> // Fornisce funzioni per l'input/output.
#include <stdlib.h> // Libreria che include funzioni per la gestione della memoria dinamica.
#include <string.h> // Libreria per la manipolazione di stringhe.
#include <unistd.h> // Fornisce l'accesso alle system-call su sistemi UNIX.
#include <arpa/inet.h> // Libreria per la gestione delle connessioni di rete su sistemi UNIX.
#include <pthread.h> // Fornisce funzioni per la creazione, gestione e sincronizzazione di thread.

#define SERVER_IP "127.0.0.1" // Indirizzo IP del server universitario.
#define SERVER_PORT 8080 // Porta del server universitario.
#define SECRETARY_PORT 8081 // Porta della segreteria.
#define BUFFER_SIZE 1024 // Size del buffer.

void* gestisciRichiesta(void* arg) { // Funzione che gestisce le richieste effettuate dallo Studente.
    int client_socket = *(int*)arg; // Assegnazione della socket client.
    free(arg);

    char buffer[BUFFER_SIZE] = {0}; // Buffer per l'inoltro delle richieste.
    char risposta[BUFFER_SIZE] = {0}; // Buffer che conterrà la risposta del server Universitario per lo Studente.
    int sock_universita; // Socket server universitario.
    struct sockaddr_in server_addr_universita; // Definisce una struttura sockaddr_in per memorizzare l'indirizzo del server dell'università.

    // Leggi la richiesta dallo studente.
    if (read(client_socket, buffer, BUFFER_SIZE) < 0) {
        perror("Errore nella lettura della richiesta");
        close(client_socket);
        pthread_exit(NULL);
    }

    // Creazione del socket per la connessione con il server universitario.
    if ((sock_universita = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Errore nella creazione del socket per il server universitario");
        close(client_socket);
        pthread_exit(NULL);
    }

    server_addr_universita.sin_family = AF_INET; // Imposta la famiglia di indirizzi su AF_INET, specificando che si utilizzerà il protocollo IPv4.
    server_addr_universita.sin_port = htons(SERVER_PORT); // Imposta il numero di porta del server, convertendolo in network byte order con htons.

    if (inet_pton(AF_INET, SERVER_IP, &server_addr_universita.sin_addr) <= 0) { // Converte l'indirizzo IP del server da stringa a formato binario e lo memorizza nella struttura sockaddr_in.
        perror("Indirizzo non valido o non supportato");
        close(client_socket);
        pthread_exit(NULL);
    }

    // Connessione al server universitario.
    if (connect(sock_universita, (struct sockaddr *)&server_addr_universita, sizeof(server_addr_universita)) < 0) {
        perror("Connessione al server universitario fallita");
        close(client_socket);
        pthread_exit(NULL);
    }

    // Invia la richiesta al server universitario.
    if (send(sock_universita, buffer, strlen(buffer), 0) < 0) {
        perror("Errore nell'invio della richiesta al server universitario");
        close(sock_universita);
        close(client_socket);
        pthread_exit(NULL);
    }

    // Ricevi la risposta dal server universitario.
    int len = read(sock_universita, risposta, BUFFER_SIZE);
    if (len < 0) {
        perror("Errore nella lettura della risposta dal server universitario");
        risposta[0] = '\0'; // Risposta vuota in caso di errore.
    } else {
        risposta[len] = '\0';
    }

    // Invia la risposta del server universitario allo studente.
    if (send(client_socket, risposta, strlen(risposta), 0) < 0) {
        perror("Errore nell'invio della risposta allo studente");
    }

    close(sock_universita); // Chiusura delle socket.
    close(client_socket);
    
    pthread_exit(NULL); // Termina il thread corrente e restituisce un valore di uscita NULL.
}

void gestisciInputTerminale() { //Questa funzione permette alla Segreteria di inviare richieste al server Universitario.
    char buffer[BUFFER_SIZE] = {0}; // Buffer per le richieste.
    int sock_universita; // Socket server università.
    struct sockaddr_in server_addr_universita; // Definisce una struttura sockaddr_in per memorizzare l'indirizzo IP e la porta del server dell'università.

    while (1) {
        // Chiede all'utente di inserire un comando.
        printf("\nInserisci un esame (formato: <nome_esame> <data_esame>): ");
        buffer[0] = 'A';
        buffer[1] = ' ';
        if (fgets(&buffer[2], sizeof(buffer) - 2, stdin) == NULL) {
            perror("Errore nella lettura dell'input");
            continue;
        }

        // Rimuovi il newline finale
        buffer[strcspn(buffer, "\n")] = 0;

        // Creazione del socket per la connessione con il server universitario.
        if ((sock_universita = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Errore nella creazione del socket per il server universitario");
            continue;
        }

        server_addr_universita.sin_family = AF_INET; // Imposta la famiglia di indirizzi su AF_INET, specificando che si utilizzerà il protocollo IPv4.
        server_addr_universita.sin_port = htons(SERVER_PORT); // Imposta il numero di porta del server, convertendolo in network byte order con htons.

        if (inet_pton(AF_INET, SERVER_IP, &server_addr_universita.sin_addr) <= 0) { // Converte l'indirizzo IP del server da stringa a formato binario e lo memorizza nella struttura sockaddr_in.
            perror("Indirizzo non valido o non supportato");
            close(sock_universita);
            continue;
        }

        // Connessione al server universitario.
        if (connect(sock_universita, (struct sockaddr *)&server_addr_universita, sizeof(server_addr_universita)) < 0) {
            perror("Connessione al server universitario fallita");
            close(sock_universita);
            continue;
        }

        // Invia il comando al server universitario.
        if (send(sock_universita, buffer, strlen(buffer), 0) < 0) {
            perror("Errore nell'invio del comando al server universitario");
            close(sock_universita);
            continue;
        }

        // Ricevi la risposta dal server universitario.
        int len = read(sock_universita, buffer, BUFFER_SIZE);
        if (len < 0) {
            perror("Errore nella lettura della risposta dal server universitario");
        } else {
            buffer[len] = '\0';
            printf("\n---------------------------------------------------------------\n");
            printf("Risposta dal server universitario: %s", buffer);
            printf("---------------------------------------------------------------\n");
        }

        close(sock_universita); // Chiusura della socket.
    }
}

int main() {
    int server_socket, client_socket; // Creazione delle socket lato client e lato server.
    struct sockaddr_in server_addr, client_addr; // Strutture dati necessarie per memorizzare indirizzi del server e del client.
    int addr_len = sizeof(client_addr); // Size dell address.
    pthread_t thread_id; // La variabile verrà utilizzata per identificare e gestire un thread POSIX.

    // Creazione/Associazione della socket per la segreteria.
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Errore nella creazione del socket per la segreteria");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET; // Si specifica che l'indirizzo del server utilizzerà l'indirizzamento ipv4.
    server_addr.sin_addr.s_addr = INADDR_ANY; // Assegna l'indirizzo IP a INADDR_ANY per permettere al server di accettare connessioni su tutte le interfacce di rete disponibili.
    server_addr.sin_port = htons(SECRETARY_PORT); // Imposta il numero di porta del server, convertendolo in network byte order con htons.

    // Binding del socket della segreteria
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Binding fallito");
        exit(EXIT_FAILURE);
    }

    // In ascolto su una porta specifica
    if (listen(server_socket, 3) < 0) {
        perror("Errore nell'ascolto");
        exit(EXIT_FAILURE);
    }

    printf("\n---------------------------------------------------------------\n");
    printf("SEGRETERIA\n");
    printf("---------------------------------------------------------------\n");

    // Creazione di un thread per gestire l'input da terminale.
    pthread_t terminal_thread;
    if (pthread_create(&terminal_thread, NULL, (void*)gestisciInputTerminale, NULL) != 0) {
        perror("Errore nella creazione del thread per l'input da terminale");
        exit(EXIT_FAILURE);
    }
    pthread_detach(terminal_thread);

    // Accettazione delle connessioni in ingresso.
    while (1) {
        int* new_sock = malloc(sizeof(int));
        if ((*new_sock = accept(server_socket, (struct sockaddr *)&client_addr, (socklen_t*)&addr_len)) < 0) {
            perror("Errore nell'accettare la connessione");
            free(new_sock);
            continue;
        }

        // Creazione di un nuovo thread per gestire la richiesta dello studente.
        if (pthread_create(&thread_id, NULL, gestisciRichiesta, (void*)new_sock) != 0) {
            perror("Errore nella creazione del thread");
            close(*new_sock);
            free(new_sock);
        }
        pthread_detach(thread_id);
    }

    return 0;
}
