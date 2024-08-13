#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SECRETARY_PORT 8081
#define BUFFER_SIZE 1024

void inviaRichiestaAllaSegreteria(char tipoRichiesta, char *matricola, char *nomeEsame, char *dataEsame) {
    int sock_segreteria;
    struct sockaddr_in server_addr_segreteria;
    char buffer[BUFFER_SIZE] = {0};

    // Creazione del socket per la connessione con la segreteria
    if ((sock_segreteria = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Errore nella creazione del socket");
        exit(EXIT_FAILURE);
    }

    server_addr_segreteria.sin_family = AF_INET;
    server_addr_segreteria.sin_port = htons(SECRETARY_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr_segreteria.sin_addr) <= 0) {
        perror("Indirizzo non valido o non supportato");
        exit(EXIT_FAILURE);
    }

    // Connessione alla segreteria
    if (connect(sock_segreteria, (struct sockaddr *)&server_addr_segreteria, sizeof(server_addr_segreteria)) < 0) {
        perror("Connessione fallita");
        exit(EXIT_FAILURE);
    }

    // Invia la richiesta alla segreteria
    if (tipoRichiesta == 'P') {
        snprintf(buffer, sizeof(buffer), "%c%s %s %s", tipoRichiesta, matricola, nomeEsame, dataEsame);
    } else {
        snprintf(buffer, sizeof(buffer), "%c%s", tipoRichiesta, nomeEsame);
    }

    send(sock_segreteria, buffer, strlen(buffer), 0);

    // Ricevi la risposta
    int len = read(sock_segreteria, buffer, BUFFER_SIZE);
    if (len > 0) {
        buffer[len] = '\0';
        printf("%s\n", buffer);
    }

    close(sock_segreteria);
}

int main() {
    int scelta;
    char nomeEsame[BUFFER_SIZE] = {0};
    char dataEsame[BUFFER_SIZE] = {0};
    char matricola[11] = {0};
    
    printf("Inserisci la matricola: ");
    fgets(matricola, sizeof(matricola), stdin);
    matricola[strcspn(matricola, "\n")] = '\0'; // Rimuove il newline alla fine della stringa
    if (strlen(matricola) > 10) {
        fprintf(stderr, "La matricola non può superare i 10 caratteri.\n");
        exit(EXIT_FAILURE);
    }
    
    while (1) {
        printf("Cosa desideri fare?\n");
        printf("1) Richiedi informazioni su un esame\n");
        printf("2) Prenota un esame\n");
        printf("3) Esci\n");
        printf("Scelta: ");
        scanf("%d", &scelta);
        getchar(); // Assorbire il newline lasciato da scanf

        switch (scelta) {
            case 1:
                printf("Inserisci il nome dell'esame: ");
                fgets(nomeEsame, sizeof(nomeEsame), stdin);
                nomeEsame[strcspn(nomeEsame, "\n")] = 0; // Rimuove il newline alla fine della stringa

                inviaRichiestaAllaSegreteria('I', matricola, nomeEsame, NULL);
                break;
            case 2:
                printf("Inserisci il nome dell'esame: ");
                fgets(nomeEsame, sizeof(nomeEsame), stdin);
                nomeEsame[strcspn(nomeEsame, "\n")] = 0; // Rimuove il newline alla fine della stringa

                printf("Inserisci la data d'appello (formato: gg/mm/aaaa): ");
                fgets(dataEsame, sizeof(dataEsame), stdin);
                dataEsame[strcspn(dataEsame, "\n")] = 0; // Rimuove il newline alla fine della stringa

                inviaRichiestaAllaSegreteria('P', matricola, nomeEsame, dataEsame);
                break;
            case 3:
                printf("Uscita...\n");
                exit(0);
            default:
                printf("Scelta non valida. Riprova.\n");
        }
    }

    return 0;
}
