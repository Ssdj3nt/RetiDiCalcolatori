#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define EXAMS_FILE "exams.txt"
#define BOOKINGS_FILE "bookings.txt"

void gestisciRichiesta(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    char tipo_richiesta;
    char nome_esame[BUFFER_SIZE] = {0};
    char data_esame[BUFFER_SIZE] = {0};
    char linea[BUFFER_SIZE] = {0};
    int esame_trovato = 0;

    // Leggi la richiesta dal client
    read(client_socket, buffer, BUFFER_SIZE);
    tipo_richiesta = buffer[0]; // La prima lettera indica il tipo di richiesta

    if (tipo_richiesta == 'A') { // Aggiunta di un esame
        sscanf(buffer + 1, "%[^\n]", buffer); // Prendi tutto il contenuto fino alla newline
        
    char *space_pos = strrchr(buffer, ' ');
    
    if (space_pos != NULL) {
        // Trova la posizione della data
        int nome_len = space_pos - buffer;
        
        // Copia il nome dell'esame
        strncpy(nome_esame, buffer, nome_len);
        nome_esame[nome_len] = '\0'; // Aggiunge il terminatore null
        
        // Copia la data dell'esame
        strcpy(data_esame, space_pos + 1);
    }

        FILE *file = fopen(EXAMS_FILE, "r+");
        if (file == NULL) {
            perror("Errore nell'apertura del file esami");
            exit(EXIT_FAILURE);
        }

        // File temporaneo per scrivere i dati aggiornati
        FILE *temp_file = fopen("temp.txt", "w");
        if (temp_file == NULL) {
            perror("Errore nell'apertura del file temporaneo");
            fclose(file);
            exit(EXIT_FAILURE);
        }

        // Leggi il file riga per riga
        while (fgets(linea, sizeof(linea), file)) {
            char esame[BUFFER_SIZE] = {0};
            sscanf(linea, "%[^\n]", esame); // Prendi il nome dell'esame

            if (strstr(esame, nome_esame) == esame) {
                // Esame trovato, aggiungi la nuova data
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

        fclose(file);
        fclose(temp_file);

        // Sostituisci il file originale con il file temporaneo
        remove(EXAMS_FILE);
        rename("temp.txt", EXAMS_FILE);

        send(client_socket, "Esame aggiunto con successo.\n", 29, 0);
    } else if (tipo_richiesta == 'P') { // Prenotazione di un esame.
    
        sscanf(buffer + 1, "%[^\n]", buffer);

        FILE *file = fopen(BOOKINGS_FILE, "a");
        if (file == NULL) {
            perror("Errore nell'apertura del file prenotazioni");
            exit(EXIT_FAILURE);
        }

        // Genera un numero di prenotazione casuale tra 1 e 100
        int numero_prenotazione = rand() % 100 + 1;

        fprintf(file, "%s \n", buffer);
        fclose(file);

        char risposta[BUFFER_SIZE] = {0};
        snprintf(risposta, sizeof(risposta), "Prenotazione - %s. Numero prenotazione: %d\n", buffer, numero_prenotazione);
        send(client_socket, risposta, strlen(risposta), 0);
        
    } else if (tipo_richiesta == 'I') { // Informazioni su un esame
        sscanf(buffer + 1, "%[^\n]", nome_esame);

        FILE *file = fopen(EXAMS_FILE, "r");
        if (file == NULL) {
            perror("Errore nell'apertura del file esami");
            exit(EXIT_FAILURE);
        }
        while (fgets(linea, sizeof(linea), file)) {
            if (strstr(linea, nome_esame) != NULL) {
                send(client_socket, linea, strlen(linea), 0);
                esame_trovato = 1; // Segna che l'esame è stato trovato
            }
        }
        fclose(file);
        if (!esame_trovato) {
            send(client_socket, "Esame non trovato.\n", 19, 0);
        }
    }
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    int addr_len = sizeof(client_addr);

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

    // Binding del socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Binding fallito");
        exit(EXIT_FAILURE);
    }

    // In ascolto su una porta specifica
    if (listen(server_socket, 3) < 0) {
        perror("Errore nell'ascolto");
        exit(EXIT_FAILURE);
    }

    // Accettazione delle connessioni in ingresso
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
