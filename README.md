# Nu te supăra, frate! (Ludo)

Implementare Client-Server a jocului "Nu te supăra, frate!" în limbajul C, utilizând primitive de sistem Linux (System V Shared Memory, Semafoare, Sockets).

Proiectul este containerizat cu **Docker** pentru a rula identic pe Windows și Linux, rezolvând problemele de compatibilitate și rețelistică.

---

## Arhitectură

* **Server (`tabla.c`):** Gestionează logica jocului, memoria partajată și sincronizarea proceselor.
* **Client (`jucator.c`):** Interfață grafică în terminal (CLI) și comunicare TCP.
* **Rețea:** Containerele comunică într-o rețea virtuală izolată (`bridge network`) pentru a evita firewall-ul sistemului gazdă.

---

## Ghid de Instalare și Rulare (Docker)

Urmează acești pași pentru a porni jocul.

### 1. Build (Compilare)
Construiește imaginea Docker care conține executabilele server și client:

```bash
docker build -t nu-te-supara .
```
### 2. Pornirea Serverului
Deschide un terminal și rulează comanda de mai jos. Serverul va rămâne pornit în această fereastră:

```bash
docker run -it --rm --init --network retea-joc --name game-server nu-te-supara
```
### 3. Aflarea IP-ului Serverului
Deschide un terminal nou și rulează comanda asta ca să vezi ce IP a primit serverul:

```bash
docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' game-server
```
### 4. Conectarea Jucătorilor
Deschide câte un terminal separat pentru fiecare jucător.

```bash
docker run -it --rm --network retea-joc nu-te-supara ./client IP_SERVER 2729
```
