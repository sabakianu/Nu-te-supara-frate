#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define RESET "\x1b[0m"
#define RED "\x1b[31m"
#define BLUE "\x1b[34m"
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[93m"
#define GRAY "\x1b[90m"
#define BROWN "\x1b[38;5;94m"

char tabla[11][11];
extern int errno;
int port;
enum Culoare
{
    Galben = 1,
    Verde = 2,
    Albastru = 3,
    Rosu = 4,
};
void rotire()
{
    char rotita[11][11];
    for (int i = 0; i < 11; i++)
    {
        for (int j = 0; j < 11; j++)
        {
            rotita[11 - j - 1][i] = tabla[i][j];
        }
    }
    for (int i = 0; i < 11; i++)
    {
        for (int j = 0; j < 11; j++)
        {
            tabla[i][j] = rotita[i][j];
        }
    }
}
int aruncaZar()
{
    srand(time(NULL));
    return rand() % (6 - 1 + 1) + 1;
}
void printTabla()
{
    for (int i = 0; i < 11; i++)
    {
        for (int j = 0; j < 11; j++)
            switch (tabla[i][j])
            {
            case 'R':
            case 'r':
                printf(RED "%c " RESET, tabla[i][j]);
                break;
            case 'G':
            case 'g':
                printf(YELLOW "%c " RESET, tabla[i][j]);
                break;
            case 'A':
            case 'a':
                printf(BLUE "%c " RESET, tabla[i][j]);
                break;
            case 'V':
            case 'v':
                printf(GREEN "%c " RESET, tabla[i][j]);
                break;
            case '0':
                printf(GRAY "%c " RESET, tabla[i][j]);
                break;
            case 'x':
                printf(BROWN "%c " RESET, tabla[i][j]);
                break;
            default:
                printf("%c ", tabla[i][j]);
                break;
            }
        printf("\n");
    }
}
const char *GetCuloare(enum Culoare cul)
{
    switch (cul)
    {
    case Galben:
        return "Galben";
    case Verde:
        return "Verde";
    case Albastru:
        return "Albastru";
    case Rosu:
        return "Rosu";
    default:
        return "Necunoscut";
    }
}
void readTabla(int sd, enum Culoare cul)
{
    if (read(sd, tabla, 121) <= 0)
    {
        perror("[client]Eroare la read() de la server.\n");
        exit(1);
    }
    if (cul == Verde)
    {
        rotire();
    }
    if (cul == Rosu)
    {
        rotire();
        rotire();
    }
    if (cul == Albastru)
    {
        rotire();
        rotire();
        rotire();
    }
}
void Joc(enum Culoare cul, int sd)
{
    bool terminat = false;
    bool tu = false;
    char notificare[50];
    while (!terminat) 
    {
        if (read(sd, &tu, sizeof(tu)) < 0)
        {
            perror("[client] Eroare la read() (verif) catre client.\n");
            close(sd);
            exit(0);
        }
        if (read(sd, notificare, sizeof(notificare)) < 0)
        {
            perror("[client] Eroare la read() (notificare) catre client.\n");
            close(sd);
            exit(0);
        }
        printf("%s\n", notificare);
        if (tu)
        {
            readTabla(sd, cul);
            printTabla();
            int zar, nr_scos;
            char tastatura = 'a';
            while (tastatura != 'z')
            {
                do
                {
                    printf("Apasa z ca sa dai cu zarul.\n");
                    scanf(" %c", &tastatura);
                    if (tastatura == 'z')
                    {
                        zar = aruncaZar();
                        printf("Ai dat %d\n", zar);
                        if (write(sd, &zar, sizeof(zar)) < 0)
                        {
                            perror("[client] Eroare la write() (zar) catre client.\n");
                            close(sd);
                            exit(0);
                        }
                        if (read(sd, &nr_scos, sizeof(nr_scos)) < 0)
                        {
                            perror("[client] Eroare la read() (pioni_scosi) catre client.\n");
                            close(sd);
                            exit(0);
                        }
                        if (zar == 6 && nr_scos < 4)
                        {
                            char alegere, comanda[20], vara[50], varb[60];
                            int scos;
                            if (read(sd, &scos, sizeof(scos)) < 0)
                            {
                                perror("[client] Eroare la read() (pioni_scosi) catre client.\n");
                                close(sd);
                                exit(0);
                            }
                            if (read(sd, comanda, sizeof(comanda)) < 0)
                            {
                                perror("[client] Eroare la read() (alege-comanda) catre client.\n");
                                close(sd);
                                exit(0);
                            }
                            if (read(sd, vara, sizeof(vara)) < 0)
                            {
                                perror("[client] Eroare la read() (comanda A) catre client.\n");
                                close(sd);
                                exit(0);
                            }
                            printf("%s\n%s", comanda, vara);
                            if (scos > 0)
                            {
                                if (read(sd, varb, sizeof(varb)) < 0)
                                {
                                    perror("[client] Eroare la read() (comanda B) catre client.\n");
                                    close(sd);
                                    exit(0);
                                }
                                printf("%s\n", varb);
                            }
                            scanf(" %c", &alegere);
                            if (write(sd, &alegere, sizeof(alegere)) < 0)
                            {
                                perror("[client] Eroare la write() (alegere) catre client.\n");
                                close(sd);
                                exit(0);
                            }
                            while (1)
                            {
                                if (alegere == 'a')
                                {
                                    break;
                                }
                                else if (alegere == 'b' && scos > 0)
                                {
                                    break;
                                }
                                else
                                {
                                    printf("Alege o comanda buna\n");
                                    scanf(" %c", &alegere);
                                    if (write(sd, &alegere, sizeof(alegere)) < 0)
                                    {
                                        perror("[client] Eroare la write() (alegere) catre client.\n");
                                        close(sd);
                                        exit(0);
                                    }
                                }
                            }
                            if (alegere == 'b' && scos > 0)
                            {
                                char cnr, alege[50];
                                int nr;
                                if (read(sd, alege, sizeof(alege)) < 0)
                                {
                                    perror("[client] Eroare la read() (alege-nr-pioni) catre client.\n");
                                    close(sd);
                                    exit(0);
                                }
                                printf("%s\n", alege);
                                scanf(" %c", &cnr);
                                nr = cnr - '0';
                                if (write(sd, &nr, sizeof(nr)) < 0)
                                {
                                    perror("[client] Eroare la write() (nr-pioni-ales) catre client.\n");
                                    close(sd);
                                    exit(0);
                                }
                                while (nr > nr_scos)
                                {
                                    printf("Nu ai ales o piesa buna\n");
                                    scanf(" %c", &cnr);
                                    nr = cnr - '0';
                                    if (write(sd, &nr, sizeof(nr)) < 0)
                                    {
                                        perror("[client] Eroare la write() (nr-pioni-ales) catre client.\n");
                                        close(sd);
                                        exit(0);
                                    }
                                }
                            }
                        }
                        else
                        {
                            if (nr_scos > 0)
                            {
                                char cnr, alege[50];
                                int nr;
                                if (read(sd, alege, sizeof(alege)) < 0)
                                {
                                    perror("[client] Eroare la read() (alege-nr-pioni) catre client.\n");
                                    close(sd);
                                    exit(0);
                                }
                                printf("%s\n", alege);
                                scanf(" %c", &cnr);
                                nr = cnr - '0';
                                if (write(sd, &nr, sizeof(nr)) < 0)
                                {
                                    perror("[client] Eroare la write() (nr-pioni-ales) catre client.\n");
                                    close(sd);
                                    exit(0);
                                }
                                while (nr > nr_scos)
                                {
                                    printf("Nu ai ales o piesa buna\n");
                                    scanf(" %c", &cnr);
                                    nr = cnr - '0';
                                    if (write(sd, &nr, sizeof(nr)) < 0)
                                    {
                                        perror("[client] Eroare la write() (nr-pioni-ales) catre client.\n");
                                        close(sd);
                                        exit(0);
                                    }
                                }
                            }
                            else
                            {
                                char status[50];
                                if (read(sd, status, sizeof(status)) < 0)
                                {
                                    perror("[client] Eroare la read() (fara-pioni) catre client.\n");
                                    close(sd);
                                    exit(0);
                                }
                                printf("%s\n", status);
                            }
                        }
                        readTabla(sd, cul);
                        printTabla();
                    }
                    else
                    {
                        printf("Nu ai apasat o tasta valida!\n");
                        zar=6;
                    }
                } while (zar == 6);
                if (read(sd, &terminat, sizeof(terminat)) < 0)
                {
                    perror("[client] Eroare la read() (gata sau nu) catre client.\n");
                    close(sd);
                    exit(0);
                }
            }
            tu = false;
        }
    }
    if (terminat)
    {
        char clasament[50];
        if (read(sd, clasament, sizeof(clasament)) < 0)
        {
            perror("[client] Eroare la read() (loc in clasament) catre client.\n");
            close(sd);
            exit(0);
        }
        printf("%s\n", clasament);
    }
}

int main(int argc, char *argv[])
{
    char comanda;
    int conectat, conn, started, str, nr;
    int sd;
    enum Culoare cul;
    struct sockaddr_in server;
    if (argc != 3)
    {
        printf("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
        return -1;
    }

    port = atoi(argv[2]);

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Eroare la socket().\n");
        return errno;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons(port);

    if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("[client]Eroare la connect().\n");
        return errno;
    }
    printf("Bun venit!\n");
    if (read(sd, &nr, 4) < 0)
    {
        perror("[client]Eroare la read() de la server.\n");
        return errno;
    }
    conectat = nr;
    if (conectat < 5)
    {
        if (read(sd, &started, 4) < 0)
        {
            perror("[client]Eroare la read() de la server.\n");
            return errno;
        }
        printf("Jucatori prezenti momentan: %d\n", conectat);
        printf("Type start: s\nType quit: q\n");
        scanf("%c", &comanda);
        while (comanda != 's' && comanda != 'q')
        {
            printf("Scrie o comanda valida\n");
            scanf(" %c", &comanda);
        }
        if (write(sd, &comanda, 1) < 0)
        {
            perror("[client]Eroare la write() catre la server.\n");
            return errno;
        }
        while (comanda == 's')
        {
            if (read(sd, &conn, 4) < 0)
            {
                perror("[client]Eroare la read() de la server.\n");
                return errno;
            }
            if (read(sd, &str, 4) < 0)
            {
                perror("[client]Eroare la read() de la server.\n");
                return errno;
            }
            if (conn != conectat || started != str)
            {
                printf("Started: %d/%d\n", str, conn);
                started = str;
                conectat = conn;
            }
            if (conectat == started && started > 1)
            {
                char culJucator[100];
                if (read(sd, &cul, sizeof(cul)) < 0)
                {
                    perror("[client]Eroare la read() (culoarea jucator) de la server.\n");
                    return errno;
                }
                if (read(sd, culJucator, 100) < 0)
                {
                    perror("[client]Eroare la read() (culoarea jucator-text) de la server.\n");
                    return errno;
                }
                printf("%s", culJucator);
                Joc(cul, sd);
                break;
            }
        }
    }
    close(sd);
}
