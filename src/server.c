#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080                           // Porta su cui il server ascolta le connessioni
#define BUFFER_SIZE 1024                    // Dimensione BUFFER
#define EXAMS_FILE "exams.txt"              // File contenente gli esami
#define BOOKINGS_FILE "bookings.txt"        // File contenente le prenotazioni

void gestisciRichiesta(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};        // Lettura dei dati dal client
    char tipo_richiesta;                   // Tipi di richieste ('A', 'P', 'I')
    char nome_esame[BUFFER_SIZE] = {0};    // Nome esame
    char data_esame[BUFFER_SIZE] = {0};    // Data esame
    char linea[BUFFER_SIZE] = {0};         // Buffer per leggere la linea del file
    int esame_trovato = 0;                 // Flag per verificare se un esame è trovato

    // Leggi la richiesta dal client
    read(client_socket, buffer, BUFFER_SIZE);
    tipo_richiesta = buffer[0]; // La prima lettera indica il tipo di richiesta

    if (tipo_richiesta == 'A') { // Aggiunta di un esame
        // Prendi tutto il contenuto fino alla newline
        sscanf(buffer + 1, "%[^\n]", buffer); 
        
    // Trova la posizione dell'ultimo spazio, che separa il nome dell'esame dalla data
    char *space_pos = strrchr(buffer, ' ');
    
    if (space_pos != NULL) {
        // Calcola la lunghezza del nome dell'esame e copia il nome
        int nome_len = space_pos - buffer;                
        strncpy(nome_esame, buffer, nome_len);
        nome_esame[nome_len] = '\0'; // Aggiunge il terminatore
        
        // Copia la data dell'esame
        strcpy(data_esame, space_pos + 1);
    }

        // Apri il file degli esami sia per la lettura che per la scrittura
        FILE *file = fopen(EXAMS_FILE, "r+");
        if (file == NULL) {
            perror("Errore nell'apertura del file esami");
            exit(EXIT_FAILURE);
        }

        // Crea un file temporaneo per scrivere i dati aggiornati
        FILE *temp_file = fopen("temp.txt", "w");
        if (temp_file == NULL) {
            perror("Errore nell'apertura del file temporaneo");
            fclose(file);
            exit(EXIT_FAILURE);
        }

        // Leggi il file degli esami riga per riga
        while (fgets(linea, sizeof(linea), file)) {
            char esame[BUFFER_SIZE] = {0};
            sscanf(linea, "%[^\n]", esame); // Prendi il nome dell'esame

            if (strstr(esame, nome_esame) == esame) {
                // Esame già esistente, aggiungi la nuova data
                fprintf(temp_file, "%s %s\n", esame, data_esame);
                esame_trovato = 1;
            } else {
                // Copia la riga esistente nel file temporaneo
                fprintf(temp_file, "%s", linea);
            }
        }

        // Se l'esame non è stato trovato, aggiungilo
        if (!esame_trovato) {
            fprintf(temp_file, "%s %s\n", nome_esame, data_esame);
        }

        // Chiudi i file 
        fclose(file);
        fclose(temp_file);
        // Sostituisci il file originale con quello temporaneo
        remove(EXAMS_FILE);
        rename("temp.txt", EXAMS_FILE);

        // Invia al client una conferma che l'esam è stato aggiunto
        send(client_socket, "Esame aggiunto con successo.\n", 29, 0);
    } else if (tipo_richiesta == 'P') { // Se la richieta è quella di prenotazione
    
        sscanf(buffer + 1, "%[^\n]", buffer);

        // Apri il file delle prenotazioni
        FILE *file = fopen(BOOKINGS_FILE, "a");
        if (file == NULL) {
            perror("Errore nell'apertura del file prenotazioni");
            exit(EXIT_FAILURE);
        }

        // Genera un numero di prenotazione casuale tra 1 e 100
        int numero_prenotazione = rand() % 100 + 1;

        // Aggiungi la prenotazione al file
        fprintf(file, "%s \n", buffer);
        fclose(file);

        // Invia la risposta al client
        char risposta[BUFFER_SIZE] = {0};
        snprintf(risposta, sizeof(risposta), "Prenotazione - %s. Numero prenotazione: %d\n", buffer, numero_prenotazione);
        send(client_socket, risposta, strlen(risposta), 0);
        
    } else if (tipo_richiesta == 'I') { // Richiesta per infromazioni su un esame
        sscanf(buffer + 1, "%[^\n]", nome_esame);

        // Apri il file degli esami in modalitù lettura
        FILE *file = fopen(EXAMS_FILE, "r");
        if (file == NULL) {
            perror("Errore nell'apertura del file esami");
            exit(EXIT_FAILURE);
        }

        // Cerca l'esame nel file
        while (fgets(linea, sizeof(linea), file)) {
            if (strstr(linea, nome_esame) != NULL) {
                send(client_socket, linea, strlen(linea), 0); // Invia la riga al client
                esame_trovato = 1; // Segna che l'esame è stato trovato
            }
        }
        fclose(file);
        // Se l'esame non è stato trovato, invia un messaggio al client
        if (!esame_trovato) {
            send(client_socket, "Esame non trovato.\n", 19, 0);
        }
    }
}

// Funzione che genera un file se non esiste
int createFileIfNotExists(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        // Il file non esiste, quindi crealo
        file = fopen(filename, "w");
        if (file == NULL) {
            perror("Errore nella creazione del file");
            return -1;
        }
        fclose(file);
    } else {
        fclose(file);
    }
    return 0;
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    int addr_len = sizeof(client_addr);
    
    // Creazione dei file se non esistono
    if (createFileIfNotExists(EXAMS_FILE) != 0) {
        exit(EXIT_FAILURE);
    }
    if (createFileIfNotExists(BOOKINGS_FILE) != 0) {
        exit(EXIT_FAILURE);
    }

    // Inizializzazione del generatore di numeri casuali
    srand(time(NULL));

    // Creazione del socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Errore nella creazione del socket");
        exit(EXIT_FAILURE);
    }

    // Setup dell'indirizzo del server
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Binding del socket all'indirizzo e alla porta
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Binding fallito");
        exit(EXIT_FAILURE);
    }

    // Binding del socket all'indirizzo e alla porta
    if (listen(server_socket, 3) < 0) {
        perror("Errore nell'ascolto");
        exit(EXIT_FAILURE);
    }

    // Accetta connessioni in entrata in un ciclo infinito
    while (1) {
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, (socklen_t*)&addr_len)) < 0) {
            perror("Errore nell'accettare la connessione");
            exit(EXIT_FAILURE);
        }

        // Gestione della richiesta del client
        gestisciRichiesta(client_socket);
        close(client_socket);
    }

    return 0;
}
