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
    int scelta; //Variabile usata per contenere la scelta dell'utente, in base alla scelta si effettueranno specifiche operazioni.
    char nomeEsame[BUFFER_SIZE] = {0}; //Vettore che conterrà il nome dell'esame.
    char dataEsame[BUFFER_SIZE] = {0}; //Vettore che conterrà la data dell'esame.
    char matricola[11] = {0}; //Vettore che conterrà la matricola dello studente.
    
    printf("\n----------------------------------------------------------------------------\n");
    printf("CLIENT STUDENTE");
    printf("\n----------------------------------------------------------------------------\n\n");
    
    printf("Inserisci la matricola: ");
    fgets(matricola, sizeof(matricola), stdin);//Login simulato attraverso l'inserimento della matricola.
    printf("\n----------------------------------------------------------------------------\n");
    matricola[strcspn(matricola, "\n")] = '\0'; // Rimuove il newline alla fine della stringa.
    if (strlen(matricola) > 10) {
        fprintf(stderr, "La matricola non può superare i 10 caratteri.\n");
        exit(EXIT_FAILURE);
    }

    //Menù grafico.
    while (1) {
        printf("\nCosa desideri fare?\n\n");
        printf("1) Richiedi informazioni su un esame\n");
        printf("2) Prenota un esame\n");
        printf("3) Esci\n");
        printf("\nScelta: ");
        scanf("%d", &scelta);
        getchar(); // Assorbire il newline lasciato da scanf.

        switch (scelta) {
            case 1:
                printf("\n----------------------------------------------------------------------------\n");
                printf("\nInserisci il nome dell'esame: ");
                fgets(nomeEsame, sizeof(nomeEsame), stdin); // Inserimento del nome dell'esame.
                nomeEsame[strcspn(nomeEsame, "\n")] = 0; // Rimuove il newline alla fine della stringa.
                printf("\n");
                printf("Appelli: ");
                inviaRichiestaAllaSegreteria('I', matricola, nomeEsame, NULL);// Richiesta informazioni sugli appelli disponibili.
                printf("----------------------------------------------------------------------------\n");
                break;
            case 2:
                printf("\n----------------------------------------------------------------------------\n\n");
                printf("Inserisci il nome dell'esame: ");
                fgets(nomeEsame, sizeof(nomeEsame), stdin); // Inserimento del nome dell'esame.
                nomeEsame[strcspn(nomeEsame, "\n")] = 0; // Rimuove il newline alla fine della stringa.
                printf("\n");

                printf("Inserisci la data d'appello (formato: gg/mm/aaaa): ");
                fgets(dataEsame, sizeof(dataEsame), stdin); // Inserimento della data dell'appello a cui ci si vuole prenotare.
                dataEsame[strcspn(dataEsame, "\n")] = 0; // Rimuove il newline alla fine della stringa.
                printf("\n");
                inviaRichiestaAllaSegreteria('P', matricola, nomeEsame, dataEsame); // Invio della prenotazione alla Segreteria.
                printf("----------------------------------------------------------------------------\n");
                break;
            case 3:
                printf("Uscita...\n");
                exit(0); // Chiusura del client.
            default:
                printf("Scelta non valida. Riprova.\n");
        }
    }

    return 0;
}
