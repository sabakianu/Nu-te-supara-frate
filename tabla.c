#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/shm.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <semaphore.h>
#include <fcntl.h>

#define PORT 2729
#define KEY1 1025
#define KEY2 1026
#define KEY3 1027
#define KEY4 1028
#define KEY5 1029
#define KEY6 1030
#define KEY7 1031
#define KEY8 1032
#define KEY9 1033
#define KEY10 1034
extern int errno;
char *tabla;
sem_t *mutex;
int *finished, *conectat, *started;
bool *gata_galben, *gata_verde, *gata_rosu, *gata_albastru;
int shmid1, shmid2, shmid3, shmid4, shmid5, shmid6, shmid7, shmid8, shmid9, shmid10;
enum Culoare *tura;

enum Culoare
{
    Galben = 1,
    Verde = 2,
    Albastru = 3,
    Rosu = 4,
};
struct pion
{
    int x;
    int y;
};
struct jucator
{
    enum Culoare id;
    struct pion pioni[4];
    struct pion pozitii[4];
    int startx, starty, pioni_scosi;
    char simbol, loc;
};
struct jucator *init_jucatori;

enum Culoare nextTura(enum Culoare tura, int *started);
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
void writeTabla(int client)
{
    if (write(client, tabla, 121) < 0)
    {
        perror("[server] Eroare la write() catre client.\n");
        close(client);
        exit(0);
    }
}
char *conv_addr(struct sockaddr_in address)
{
    static char str[25];
    char port[7];
    strcpy(str, inet_ntoa(address.sin_addr));
    bzero(port, 7);
    sprintf(port, ":%d", ntohs(address.sin_port));
    strcat(str, port);
    return (str);
}
void desenVertical(int x, int y, int z, char c)
{
    for (int i = x; i <= y; i++)
    {
        tabla[i * 11 + z] = c;
    }
}
void desenOrizontal(int x, int y, int z, char c)
{
    for (int i = x; i <= y; i++)
    {
        tabla[z * 11 + i] = c;
    }
}
void init_tabla()
{
    for (int i = 0; i < 11; i++)
        for (int j = 0; j < 11; j++)
            tabla[i * 11 + j] = '0';
    desenOrizontal(4, 6, 0, 'x');
    desenOrizontal(4, 6, 10, 'x');
    desenVertical(4, 6, 0, 'x');
    desenVertical(4, 6, 10, 'x');
    desenVertical(0, 4, 4, 'x');
    desenVertical(0, 4, 6, 'x');
    desenVertical(6, 10, 4, 'x');
    desenVertical(6, 10, 6, 'x');
    desenOrizontal(0, 4, 4, 'x');
    desenOrizontal(0, 4, 6, 'x');
    desenOrizontal(6, 10, 4, 'x');
    desenOrizontal(6, 10, 6, 'x');
    desenVertical(1, 4, 5, 'v');
    desenVertical(6, 9, 5, 'a');
    desenOrizontal(1, 4, 5, 'g');
    desenOrizontal(6, 9, 5, 'r');
    desenOrizontal(1, 2, 1, 'G');
    desenOrizontal(1, 2, 2, 'G');
    desenOrizontal(8, 9, 1, 'V');
    desenOrizontal(8, 9, 2, 'V');
    desenOrizontal(1, 2, 8, 'A');
    desenOrizontal(1, 2, 9, 'A');
    desenOrizontal(8, 9, 8, 'R');
    desenOrizontal(8, 9, 9, 'R');
}

enum Culoare nextTura(enum Culoare tura, int *started)
{
    if (*started == 4)
    {
        switch (tura)
        {
        case Galben:
            if (!(*gata_verde))
            {
                return Verde;
            }
        case Verde:
            if (!(*gata_albastru))
            {
                return Albastru;
            }
        case Albastru:
            if (!(*gata_rosu))
            {
                return Rosu;
            }
        case Rosu:
            if (!(*gata_galben))
            {
                return Galben;
            }
        default:
            return tura;
        }
    }
    else if (*started == 3)
    {
        switch (tura)
        {
        case Galben:
            if (!(*gata_verde))
            {
                return Verde;
            }
        case Verde:
            if (!(*gata_albastru))
            {
                return Albastru;
            }
        case Albastru:
            if (!(*gata_galben))
            {
                return Galben;
            }
        default:
            return tura;
        }
    }
    else if (*started == 2)
    {
        switch (tura)
        {
        case Galben:
            if (!(*gata_verde))
            {
                return Verde;
            }
        case Verde:
            if (!(*gata_galben))
            {
                return Galben;
            }
        default:
            return tura;
        }
    }
    return tura;
}

bool path(int x, int y)
{
    if (strchr("xRGVA", tabla[x * 11 + y]) != NULL)
        return true;
    return false;
}
bool verif(int nr, struct jucator *j)
{
    if (j->id == Galben)
    {
        struct pion loc[10] = {
            {6, 4},
            {6, 3},
            {6, 2},
            {6, 1},
            {6, 0},
            {5, 0},
            {5, 1},
            {5, 2},
            {5, 3},
            {5, 4}};
        for (int i = 0; i < 10; i++)
        {
            if (loc[i].x == j->pioni[nr].x && loc[i].y == j->pioni[nr].y)
                return true;
        }
    }
    else if (j->id == Verde)
    {
        struct pion loc[10] = {
            {4, 4},
            {3, 4},
            {2, 4},
            {1, 4},
            {0, 4},
            {0, 5},
            {1, 5},
            {2, 5},
            {3, 5},
            {4, 5}};
        for (int i = 0; i < 10; i++)
        {
            if (loc[i].x == j->pioni[nr].x && loc[i].y == j->pioni[nr].y)
                return true;
        }
    }
    else if (j->id == Albastru)
    {
        struct pion loc[10] = {
            {6, 6},
            {7, 6},
            {8, 6},
            {9, 6},
            {10, 6},
            {10, 5},
            {9, 5},
            {8, 5},
            {7, 5},
            {6, 5}};
        for (int i = 0; i < 10; i++)
        {
            if (loc[i].x == j->pioni[nr].x && loc[i].y == j->pioni[nr].y)
                return true;
        }
    }
    else if (j->id == Rosu)
    {
        struct pion loc[10] = {
            {4, 6},
            {4, 7},
            {4, 8},
            {4, 9},
            {4, 10},
            {5, 10},
            {5, 9},
            {5, 8},
            {5, 7},
            {5, 6}};
        for (int i = 0; i < 10; i++)
        {
            if (loc[i].x == j->pioni[nr].x && loc[i].y == j->pioni[nr].y)
                return true;
        }
    }
    return false;
}
void create_temp_tabla(int client, char temp_tabla[11][11], struct jucator *j)
{
    for (int i = 0; i < 11; i++)
        for (int k = 0; k < 11; k++)
            temp_tabla[i][k] = tabla[i * 11 + k];
    for (int i = 0; i < j->pioni_scosi; i++)
    {
        temp_tabla[j->pioni[i].x][j->pioni[i].y] = i + 1 + '0';
    }
}
void inlocuire(int x, int y, char s)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (x == init_jucatori[i].pioni[j].x && y == init_jucatori[i].pioni[j].y && init_jucatori[i].simbol == s)
            {
                for (int k = j; k < 3; k++)
                {
                    init_jucatori[i].pioni[k] = init_jucatori[i].pioni[k + 1];
                }
                init_jucatori[i].pioni_scosi--;
                init_jucatori[i].pioni[init_jucatori[i].pioni_scosi] = init_jucatori[i].pozitii[init_jucatori[i].pioni_scosi];
                tabla[init_jucatori[i].pozitii[init_jucatori[i].pioni_scosi].x * 11 + init_jucatori[i].pozitii[init_jucatori[i].pioni_scosi].y] = s;
                return;
            }
        }
    }
}
void endgame_mechanics(int *miscare, struct jucator *j, int nr, char symbol_vechi, struct pion locuri[])
{
    int i;
    for (i = 0; i < 10; i++)
    {
        if (locuri[i].x == j->pioni[nr].x && locuri[i].y == j->pioni[nr].y)
            break;
    }
    if (i >= 6)
    {
        symbol_vechi = j->loc;
    }
    if (i == 10)
    {
        printf("%s: Nu a gasit in endgame!\n", GetCuloare(j->id));
        return;
    }
    int maxim = 10 - i - 1;
    if ((*miscare) > maxim)
    {
        printf("%s: Nu merge sa dai mai departe decat maximul!\n", GetCuloare(j->id));
        return;
    }
    else
    {
        int locatie = i + (*miscare);
        if (locatie >= 6 && tabla[locuri[locatie].x * 11 + locuri[locatie].y] == j->simbol)
        {
            printf("%s: Nu e liber locul\n", GetCuloare(j->id));
            return;
        }
        while (*miscare)
        {
            tabla[locuri[i].x * 11 + locuri[i].y] = symbol_vechi;
            i++;
            symbol_vechi = tabla[locuri[i].x * 11 + locuri[i].y];
            if ((*miscare) == 1 && strchr("GVRA", symbol_vechi) != NULL && symbol_vechi != j->simbol)
            {
                inlocuire(j->pioni[nr].x, j->pioni[nr].y, symbol_vechi);
            }
            j->pioni[nr].x = locuri[i].x;
            j->pioni[nr].y = locuri[i].y;
            tabla[j->pioni[nr].x * 11 + j->pioni[nr].y] = j->simbol;
            (*miscare)--;
        }
    }
}
void endgame(int *miscare, struct jucator *j, int nr, char symbol_vechi)
{
    if (j->id == Galben)
    {
        struct pion loc[10] = {
            {6, 4},
            {6, 3},
            {6, 2},
            {6, 1},
            {6, 0},
            {5, 0},
            {5, 1},
            {5, 2},
            {5, 3},
            {5, 4}};
        endgame_mechanics(miscare, j, nr, symbol_vechi, loc);
    }
    else if (j->id == Verde)
    {
        struct pion loc[10] = {
            {4, 4},
            {3, 4},
            {2, 4},
            {1, 4},
            {0, 4},
            {0, 5},
            {1, 5},
            {2, 5},
            {3, 5},
            {4, 5}};
        endgame_mechanics(miscare, j, nr, symbol_vechi, loc);
    }
    else if (j->id == Albastru)
    {
        struct pion loc[10] = {
            {6, 6},
            {7, 6},
            {8, 6},
            {9, 6},
            {10, 6},
            {10, 5},
            {9, 5},
            {8, 5},
            {7, 5},
            {6, 5}};
        endgame_mechanics(miscare, j, nr, symbol_vechi, loc);
    }
    else if (j->id == Rosu)
    {
        struct pion loc[10] = {
            {4, 6},
            {4, 7},
            {4, 8},
            {4, 9},
            {4, 10},
            {5, 10},
            {5, 9},
            {5, 8},
            {5, 7},
            {5, 6}};
        endgame_mechanics(miscare, j, nr, symbol_vechi, loc);
    }
}
void scoate_pion(struct jucator *j)
{
    if (strchr("GVRA", tabla[j->startx * 11 + j->starty]) != NULL && j->simbol != tabla[j->startx * 11 + j->starty])
    {
        inlocuire(j->startx, j->starty, tabla[j->startx * 11 + j->starty]);
    }
    j->pioni[j->pioni_scosi].x = j->startx;
    j->pioni[j->pioni_scosi].y = j->starty;
    tabla[j->startx * 11 + j->starty] = j->simbol;
    tabla[j->pozitii[j->pioni_scosi].x * 11 + j->pozitii[j->pioni_scosi].y] = j->loc;
    j->pioni_scosi++;
}
void miscare(int miscare, struct jucator *j, int nr)
{
    char symbol_vechi = 'x';
    for (int i = 0; i < j->pioni_scosi; i++)
    {
        if (j->pioni[nr - 1].x == j->pioni[i].x && j->pioni[nr - 1].y == j->pioni[i].y && i != nr - 1)
        {
            symbol_vechi = j->simbol;
        }
    }
    while (miscare)
    {
        int curentX = j->pioni[nr - 1].x;
        int curentY = j->pioni[nr - 1].y;
        bool final = verif(nr - 1, j);
        if (final)
        {
            endgame(&miscare, j, nr - 1, symbol_vechi);
            miscare = 0;
        }
        else
        {
            if (((curentX == 4 && curentY != 4) || curentX == 0) && path(curentX, curentY + 1) && curentY != 10)
            {
                tabla[curentX * 11 + curentY] = symbol_vechi;
                symbol_vechi = tabla[curentX * 11 + curentY + 1];
                tabla[curentX * 11 + curentY + 1] = j->simbol;
                j->pioni[nr - 1].y++;
            }
            else if (((curentY == 4 && curentX != 6) || curentY == 0) && path(curentX - 1, curentY))
            {
                tabla[curentX * 11 + curentY] = symbol_vechi;
                symbol_vechi = tabla[(curentX - 1) * 11 + curentY];
                tabla[(curentX - 1) * 11 + curentY] = j->simbol;
                j->pioni[nr - 1].x--;
            }
            else if (((curentY == 6 && curentX != 4) || curentY == 10) && path(curentX + 1, curentY) && curentX != 10)
            {
                tabla[curentX * 11 + curentY] = symbol_vechi;
                symbol_vechi = tabla[(curentX + 1) * 11 + curentY];
                tabla[(curentX + 1) * 11 + curentY] = j->simbol;
                j->pioni[nr - 1].x++;
            }
            else if (((curentX == 6 && curentY != 6) || curentX == 10) && path(curentX, curentY - 1))
            {
                tabla[curentX * 11 + curentY] = symbol_vechi;
                symbol_vechi = tabla[curentX * 11 + curentY - 1];
                tabla[curentX * 11 + curentY - 1] = j->simbol;
                j->pioni[nr - 1].y--;
            }
            if (miscare == 1 && strchr("GVRA", symbol_vechi) != NULL && symbol_vechi != j->simbol)
            {
                inlocuire(j->pioni[nr - 1].x, j->pioni[nr - 1].y, symbol_vechi);
            }
            miscare--;
        }
    }
}
void alege_pion(int client, int zar, struct jucator *j)
{
    int nr;
    char alegere[50];
    sprintf(alegere, "Alege o piesa sa o muti din cele %d scoase\n", j->pioni_scosi);
    if (write(client, alegere, sizeof(alegere)) < 0)
    {
        perror("[server] Eroare la write() (alege-nr-pioni) catre client.\n");
        close(client);
        exit(0);
    }
    if (read(client, &nr, sizeof(nr)) < 0)
    {
        perror("[server] Eroare la read() (nr-pioni-ales) catre client.\n");
        close(client);
        exit(0);
    }
    while (nr > j->pioni_scosi)
    {
        if (read(client, &nr, sizeof(nr)) < 0)
        {
            perror("[server] Eroare la read() (nr-pioni-ales) catre client.\n");
            close(client);
            exit(0);
        }
    }
    miscare(zar, j, nr);
}
void alege_comanda(int client, struct jucator *j)
{
    int scos = j->pioni_scosi;
    if (write(client, &scos, sizeof(scos)) < 0)
    {
        perror("[server] Eroare la write() (pioni_scosi) catre client.\n");
        close(client);
        exit(0);
    }
    char alegere, comanda[20], vara[50], varb[60];
    sprintf(comanda, "Alege o comanda:\n");
    if (write(client, comanda, sizeof(comanda)) < 0)
    {
        perror("[server] Eroare la write() (alege-comanda) catre client.\n");
        close(client);
        exit(0);
    }
    sprintf(vara, "A) Apasa (a) pentru a scoate un nou pion;\n");
    if (write(client, vara, sizeof(vara)) < 0)
    {
        perror("[server] Eroare la write() (comanda A) catre client.\n");
        close(client);
        exit(0);
    }
    if (j->pioni_scosi > 0)
    {
        sprintf(varb, "B) Apasa (b) pentru a muta un pion deja existent;\n");
        if (write(client, varb, sizeof(varb)) < 0)
        {
            perror("[server] Eroare la write() (comanda B) catre client.\n");
            close(client);
            exit(0);
        }
    }
    if (read(client, &alegere, sizeof(alegere)) < 0)
    {
        perror("[server] Eroare la read() (alegere) catre client.\n");
        close(client);
        exit(0);
    }
    while (1)
    {
        if (alegere == 'a')
        {
            scoate_pion(j);
            break;
        }
        else if (alegere == 'b' && j->pioni_scosi > 0)
        {
            alege_pion(client, 6, j);
            break;
        }
        else
        {
            if (read(client, &alegere, sizeof(alegere)) < 0)
            {
                perror("[server] Eroare la read() (alegere) catre client.\n");
                close(client);
                exit(0);
            }
        }
    }
}
void mutare(int client, struct jucator *j, int zar)
{
    int nr_scos = j->pioni_scosi;
    if (write(client, &nr_scos, sizeof(nr_scos)) < 0)
    {
        perror("[server] Eroare la write() (pioni_scosi) catre client.\n");
        close(client);
        exit(0);
    }
    if (zar == 6 && j->pioni_scosi < 4)
    {
        alege_comanda(client, j);
    }
    else
    {
        if (j->pioni_scosi > 0)
        {
            alege_pion(client, zar, j);
        }
        else
        {
            char status[50];
            sprintf(status, "Nu ai scos niciun pion (trebuie sa dai 6)\n");
            if (write(client, status, sizeof(status)) < 0)
            {
                perror("[server] Eroare la write() (fara-pioni) catre client.\n");
                close(client);
                exit(0);
            }
        }
    }
}
bool finish(struct jucator *j)
{
    struct pion terminare[4];
    int gata = 0;
    if (j->id == Galben)
    {
        terminare[0] = (struct pion){5, 1};
        terminare[1] = (struct pion){5, 2};
        terminare[2] = (struct pion){5, 3};
        terminare[3] = (struct pion){5, 4};
    }
    else if (j->id == Verde)
    {
        terminare[0] = (struct pion){1, 5};
        terminare[1] = (struct pion){2, 5};
        terminare[2] = (struct pion){3, 5};
        terminare[3] = (struct pion){4, 5};
    }
    else if (j->id == Albastru)
    {
        terminare[0] = (struct pion){9, 5};
        terminare[1] = (struct pion){8, 5};
        terminare[2] = (struct pion){7, 5};
        terminare[3] = (struct pion){6, 5};
    }
    else if (j->id == Rosu)
    {
        terminare[0] = (struct pion){5, 9};
        terminare[1] = (struct pion){5, 8};
        terminare[2] = (struct pion){5, 7};
        terminare[3] = (struct pion){5, 6};
    }
    for (int i = 0; i < 4; i++)
    {
        if (tabla[terminare[i].x * 11 + terminare[i].y] == j->simbol)
            gata++;
    }
    if (gata == 4)
    {
        switch (j->id)
        {
        case Galben:
            (*gata_galben) = true;
            break;
        case Verde:
            (*gata_verde) = true;
            break;
        case Albastru:
            (*gata_albastru) = true;
            break;
        case Rosu:
            (*gata_rosu) = true;
            break;
        }
        return true;
    }
    return false;
}

void start_joc(int client, enum Culoare *tura, enum Culoare cul, int *started, struct jucator *j)
{
    bool terminat = false;
    char temp_tabla[11][11];
    enum Culoare turaVeche = 0;
    enum Culoare localTura;
    bool tu = false;
    char notificare[50];
    int zar;
    while (!terminat)
    {
        sem_wait(mutex);
        localTura = (*tura);
        sem_post(mutex);
        if (cul == localTura)
        {
            tu = true;
            if (write(client, &tu, sizeof(tu)) < 0)
            {
                perror("[server] Eroare la write() (verif) catre client.\n");
                close(client);
                exit(0);
            }
            sprintf(notificare, "Acum este tura ta");
            if (write(client, notificare, sizeof(notificare)) < 0)
            {
                perror("[server] Eroare la write() (notificare) catre client.\n");
                close(client);
                exit(0);
            }
            if (tu)
            {
                create_temp_tabla(client, temp_tabla, j);
                if (write(client, temp_tabla, 121) < 0)
                {
                    perror("[server] Eroare la write() (tabla-alegere) catre client.\n");
                    close(client);
                    exit(0);
                }
            }
            do
            {
                if (read(client, &zar, sizeof(zar)) < 0)
                {
                    perror("[server] Eroare la read() (zar) catre client.\n");
                    close(client);
                    exit(0);
                }
                mutare(client, j, zar);
                create_temp_tabla(client, temp_tabla, j);
                if (write(client, temp_tabla, 121) < 0)
                {
                    perror("[server] Eroare la write() (tabla-alegere) catre client.\n");
                    close(client);
                    exit(0);
                }
            } while (zar == 6);
            if (finish(j) || (*finished) + 1 == (*started))
            {
                (*finished)++;
                terminat = true;
            }
            if (write(client, &terminat, sizeof(terminat)) < 0)
            {
                perror("[server] Eroare la write() (gata sau nu) catre client.\n");
                close(client);
                exit(0);
            }
            sem_wait(mutex);
            *tura = nextTura(*tura, started);
            sem_post(mutex);
            tu = false;
        }
        else
        {
            if (localTura != turaVeche)
            {
                if (write(client, &tu, sizeof(tu)) < 0)
                {
                    perror("[server] Eroare la write() (verif) catre client.\n");
                    close(client);
                    exit(0);
                }
                sprintf(notificare, "Acum este tura lui: %s", GetCuloare(*tura));
                if (write(client, notificare, sizeof(notificare)) < 0)
                {
                    perror("[server] Eroare la write() (notificare) catre client.\n");
                    close(client);
                    exit(0);
                }
            }
        }
        turaVeche = localTura;
    }
    if (terminat)
    {
        char clasament[50];
        sprintf(clasament, "Felicitari, ai terminat pe locul %d", (*finished));
        if (write(client, clasament, sizeof(clasament)) < 0)
        {
            perror("[server] Eroare la write() (loc in clasament) catre client.\n");
            close(client);
            exit(0);
        }
    }
}

void initializare_assets()
{
    init_tabla();
    struct jucator jGalben = {
        Galben,                           // Culoarea jucătorului
        {{1, 1}, {1, 2}, {2, 1}, {2, 2}}, // Inițializarea pioni[4]
        {{1, 1}, {1, 2}, {2, 1}, {2, 2}}, // Inițializarea pozitii[4]
        4,                                // startx
        0,                                // starty
        0,
        'G',
        'g'};
    struct jucator jVerde = {
        Verde,                            // Culoarea jucătorului
        {{1, 8}, {1, 9}, {2, 8}, {2, 9}}, // Inițializarea pioni[4]
        {{1, 8}, {1, 9}, {2, 8}, {2, 9}}, // Inițializarea pozitii[4]
        0,                                // startx
        6,                                // starty
        0,
        'V',
        'v'};
    struct jucator jAlbastru = {
        Albastru,                         // Culoarea jucătorului
        {{8, 1}, {8, 2}, {9, 1}, {9, 2}}, // Inițializarea pioni[4]
        {{8, 1}, {8, 2}, {9, 1}, {9, 2}}, // Inițializarea pozitii[4]
        10,                               // startx
        4,                                // starty
        0,
        'A',
        'a'};
    struct jucator jRosu = {
        Rosu,                             // Culoarea jucătorului
        {{8, 8}, {8, 9}, {9, 8}, {9, 9}}, // Inițializarea pioni[4]
        {{8, 8}, {8, 9}, {9, 8}, {9, 9}}, // Inițializarea pozitii[4]
        6,                                // startx
        10,                               // starty
        0,
        'R',
        'r'};
    init_jucatori[0] = jGalben;
    init_jucatori[1] = jVerde;
    init_jucatori[2] = jAlbastru;
    init_jucatori[3] = jRosu;
}
void initCommonMemory()
{
    mutex = sem_open("/mutex", O_CREAT, 0777, 1);
    if (mutex == SEM_FAILED)
    {
        perror("Nu  a mers sem_open");
        exit(1);
    }
    shmid1 = shmget(KEY1, sizeof(int), IPC_CREAT | 0666);
    if (shmid1 < 0)
    {
        perror("Eroare la shmget1");
        exit(1);
    }
    shmid2 = shmget(KEY2, sizeof(int), IPC_CREAT | 0666);
    if (shmid2 < 0)
    {
        perror("Eroare la shmget2");
        exit(1);
    }
    shmid3 = shmget(KEY3, sizeof(enum Culoare), IPC_CREAT | 0666);
    if (shmid3 < 0)
    {
        perror("Eroare la shmget3");
        exit(1);
    }
    shmid4 = shmget(KEY4, sizeof(char) * 11 * 11, IPC_CREAT | 0666);
    if (shmid4 < 0)
    {
        perror("Eroare la shmget4");
        exit(1);
    }
    shmid5 = shmget(KEY5, sizeof(struct jucator) * 4, IPC_CREAT | 0666);
    if (shmid5 < 0)
    {
        perror("Eroare la shmget5");
        exit(1);
    }
    shmid6 = shmget(KEY6, sizeof(int), IPC_CREAT | 0666);
    if (shmid6 < 0)
    {
        perror("Eroare la shmget6");
        exit(1);
    }
    shmid7 = shmget(KEY7, sizeof(bool), IPC_CREAT | 0666);
    if (shmid7 < 0)
    {
        perror("Eroare la shmget7");
        exit(1);
    }
    shmid8 = shmget(KEY8, sizeof(bool), IPC_CREAT | 0666);
    if (shmid8 < 0)
    {
        perror("Eroare la shmget8");
        exit(1);
    }
    shmid9 = shmget(KEY9, sizeof(bool), IPC_CREAT | 0666);
    if (shmid9 < 0)
    {
        perror("Eroare la shmget9");
        exit(1);
    }
    shmid10 = shmget(KEY10, sizeof(bool), IPC_CREAT | 0666);
    if (shmid10 < 0)
    {
        perror("Eroare la shmget10");
        exit(1);
    }
    conectat = (int *)shmat(shmid1, NULL, 0);
    started = (int *)shmat(shmid2, NULL, 0);
    tura = (enum Culoare *)shmat(shmid3, NULL, 0);
    tabla = (char *)shmat(shmid4, NULL, 0);
    init_jucatori = (struct jucator *)shmat(shmid5, NULL, 0);
    finished = (int *)shmat(shmid6, NULL, 0);
    gata_galben = (bool *)shmat(shmid7, NULL, 0);
    gata_verde = (bool *)shmat(shmid8, NULL, 0);
    gata_albastru = (bool *)shmat(shmid9, NULL, 0);
    gata_rosu = (bool *)shmat(shmid10, NULL, 0);
    if (conectat == (int *)-1)
    {
        perror("Eroare la shmat1");
        exit(1);
    }
    if (started == (int *)-1)
    {
        perror("Eroare la shmat2");
        exit(1);
    }
    if (tura == (enum Culoare *)-1)
    {
        perror("Eroare la shmat3");
        exit(1);
    }
    if (tabla == (char *)-1)
    {
        perror("Eroare la shmat4");
        exit(1);
    }
    if (init_jucatori == (struct jucator *)-1)
    {
        perror("Eroare la shmat5");
        exit(1);
    }
    if (finished == (int *)-1)
    {
        perror("Eroare la shmat6");
        exit(1);
    }
    if (gata_galben == (bool *)-1)
    {
        perror("Eroare la shmat7");
        exit(1);
    }
    if (gata_verde == (bool *)-1)
    {
        perror("Eroare la shmat8");
        exit(1);
    }
    if (gata_albastru == (bool *)-1)
    {
        perror("Eroare la shmat9");
        exit(1);
    }
    if (gata_rosu == (bool *)-1)
    {
        perror("Eroare la shmat10");
        exit(1);
    }
    *conectat = 0;
    *started = 0;
    *tura = Galben;
    *finished = 0;
    *gata_galben = false;
    *gata_verde = false;
    *gata_albastru = false;
    *gata_rosu = false;
}
int main()
{
    struct sockaddr_in server;
    struct sockaddr_in from;
    int sd;
    int optval = 1;

    initCommonMemory();
    initializare_assets();
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("[server] Eroare la socket().\n");
        return errno;
    }
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    bzero(&server, sizeof(server));
    bzero(&from, sizeof(from));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);

    if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("[server] Eroare la bind().\n");
        return errno;
    }

    if (listen(sd, 4) == -1)
    {
        perror("[server] Eroare la listen().\n");
        return errno;
    }

    while (1)
    {
        int client;
        unsigned int length = sizeof(from);

        printf("[server] Asteptam la portul %d...\n", PORT);
        fflush(stdout);

        client = accept(sd, (struct sockaddr *)&from, &length);
        sem_wait(mutex);
        (*conectat)++;
        sem_post(mutex);
        if (client < 0)
        {
            perror("[server] Eroare la accept().\n");
            continue;
        }
        if (fork() == 0) // reminder aici incepe cu fiecare client
        {
            if (write(client, conectat, 4) < 0)
                {
                    perror("[server] Eroare la write() (nr de conectati) catre client.\n");
                    close(client);
                    exit(0);
                }
            if ((*conectat) < 5)
            {
                char comanda;
                enum Culoare cul;
                char culJucator[100];
                int local_conectat, local_started;
                if (write(client, started, 4) < 0)
                {
                    perror("[server] Eroare la write() (nr de started) catre client.\n");
                    close(client);
                    exit(0);
                }
                if (read(client, &comanda, 1) < 0)
                {
                    perror("[server] Eroare la read() (comanda) catre client.\n");
                    close(client);
                    exit(0);
                }
                if (comanda == 's')
                {
                    sem_wait(mutex);
                    (*started)++;
                    cul = (*started);
                    sem_post(mutex);
                    struct jucator *j = &init_jucatori[(*started) - 1];
                    sprintf(culJucator, "Jocul a inceput!Tu vei avea culoarea: %s\n", GetCuloare(cul));
                    while (1)
                    {
                        sem_wait(mutex);
                        local_conectat = (*conectat);
                        local_started = (*started);
                        sem_post(mutex);
                        if (write(client, &local_conectat, 4) < 0)
                        {
                            perror("[server] Eroare la write() (conectat-irl) catre client.\n");
                            close(client);
                            exit(0);
                        }
                        if (write(client, &local_started, 4) < 0)
                        {
                            perror("[server] Eroare la write() (started-irl) catre client.\n");
                            close(client);
                            exit(0);
                        }
                        if (local_conectat == local_started && (*started) > 1)
                        {
                            if (write(client, &cul, sizeof(cul)) < 0)
                            {
                                perror("[server] Eroare la write() (culoarea jucator) catre client.\n");
                                close(client);
                                exit(0);
                            }
                            if (write(client, culJucator, 100) < 0)
                            {
                                perror("[server] Eroare la write() (culoarea jucator-text) catre client.\n");
                                close(client);
                                exit(0);
                            }
                            start_joc(client, tura, cul, started, j);
                            close(client);
                            (*conectat)--;
                            break;
                        }
                    }
                }
                else if (comanda == 'q')
                {
                    sem_wait(mutex);
                    (*conectat)--;
                    sem_post(mutex);
                    close(client);
                    close(sd);
                    exit(0);
                }
            }
            else
            {
                sem_wait(mutex);
                (*conectat)--;
                sem_post(mutex);
                close(client);
                close(sd);
                exit(0);
            }
        }
        if ((*conectat) == 0)
        {
            (*started) = 0;
        }
    }
}
