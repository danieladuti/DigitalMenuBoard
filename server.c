/*
server.c

modificari :
- verifica comenzile daca sunt valide sau nu si afiseaza un mesaj corespunzator
- respinge :conectare : ,afisare , deconectare , si modificare daca nu are destule argumente si
 daca vrei sa te conectezi de mai multe ori
- verifica numele restaurantelor in fisierul txt: restaurante.txt
- am introdus comenzile inserare si stergere(valideaza daca sunt corecte scris sintactic si daca exista produsul cu numele respectiv)
- am modificat la modificare sa aiba numai modificare : <denumire produs> <atribut produs(ingrediente/pret)>, fara clasa
pt facilitarea cautarii produselor dupa nume

imbunatatiri ulterioare:
- bonus: securitate pentru username + parola (macar bulinute cand se pune parola)
--> bonus de securitate avansata(vezi cum se poate -- poate sa nu afiseze deloc in terminal)

instructiuni acceptate :
// /*1.*/
// conectare : <username>
// /*2.*/ afisare
// /*3.*/ creeaza meniu\n
// /*4.*/ inserare : <denumire produs>=<ingredient 1,ingredient 2>=<pret>
// /*5.*/ modificare : <denumire produs>=<atribut produs(ingrediente/pret)
// /*6.*/ stergere : <denumire produs>
// /*7.*/ deconectare

// @daniela dutescu
// */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/wait.h>
#include <fcntl.h> // O_RDONLY

#define dim 10000  // pt dimensiunea mesajelor + buffer => trebuie sa aiba aceeasi dimensiune cand facem citirea din mesaj in buffer
int conectat[100]; // verificam daca clientul este connectat sau nu
char msg[dim];     // mesajul de la client

/* portul folosit */
#define PORT 2783

extern int errno; /* eroarea returnata de unele apeluri */

/* functie de convertire a adresei IP a clientului in sir de caractere */
char *conv_addr(struct sockaddr_in address)
{
  static char str[25];
  char port[7];

  /* adresa IP a clientului */
  strcpy(str, inet_ntoa(address.sin_addr));
  /* portul utilizat de client */
  bzero(port, 7);
  sprintf(port, ":%d", ntohs(address.sin_port));
  strcat(str, port);
  return (str);
}

/*-------------------------------------------------------------------------------------------------


                                      CONECTARE


---------------------------------------------------------------------------------------------------*/
char *string_tolower(char mesaj[])
{
  for (int i = 0; mesaj[i] != '\0'; i++)
  {
    mesaj[i] = tolower(mesaj[i]);
  }
  return mesaj;
}

char username[] = ""; // variabila globala pt salvarea username-ului din fisierul restaurante
int username_gasit(char nume[])
{
  char buffer[dim] = "";
  int bytesf;
  int flag = 0;
  int fd_username;

  // elimin newline-ul adaugat de la read
  int i = 0;
  while (i < strlen(nume))
  {
    if (nume[i] == '\n')
    {
      break;
    }
    i++;
  }
  nume[i] = '\0';
  printf("[s-username]username-ul a fost modificat...%s\n", nume);

  // deschid fisierul cu restaurante
  if ((fd_username = open("restaurante.txt", O_RDONLY)) == -1)
  {
    printf("[s-username]eroare la deschiderea fisierului");
  }

  char *user = &nume[0]; // fac o copie pt nume pt ca dupa aceea il modific
  // if ((fd_username = open("restaurante.txt", O_RDONLY)) == -1)
  // {
  //   printf("[s-username]eroare la deschiderea fisierului cu usernames.");
  // }

  // bzero(buffer,dim);
  bytesf = read(fd_username, buffer, dim);
  buffer[bytesf] = '\0';

  printf("[s-username]fiserul contine: %s\n", buffer);
  // printf("%s", buffer);

  // verif daca exista vreun username in fisier
  char copie_gasit[dim] = ""; // il folosesc in while ca nu stiu daca il va gasi sau nu pe username-ul introdus
  char *token = strtok(buffer, "\n");
  while (token != NULL)
  {
    printf("[s-username]am intrat in while!!!!!\n");
    fflush(stdout);
    printf("[s-username]token: %s \n", token);
    strcpy(copie_gasit, token);
    fflush(stdout);
    if (strcmp(string_tolower(token), string_tolower(user)) == 0)
    {
      flag = 1;
      break;
    }
    token = strtok(NULL, "\n");
  }

  if (flag == 1)
  {
    strcpy(username, "");          // resetez username-ul pt ca username e variabila globala si sa nu ramana cu resturi dinainte
    strcpy(username, copie_gasit); // daca am gasit usernameul il salvez in copie_gasit
  }

  return flag;
}

int conectare(char *msg, int fd, int bytes)
{
  char raspuns[dim];
  char nume[dim] = "";

  for (int i = 12; i < dim; i++)
    nume[i - 12] = msg[i]; // conectare : --> 12

  printf("[s-conn]am primit comanda..%s, cu numele %s\n", msg, nume);
  if (username_gasit(nume) == 1) // am gasit username-ul
  {
    printf("[s-conn]salutare, %s!\n", nume);

    /*pregatim mesajul de raspuns */
    bzero(raspuns, dim);
    strcat(raspuns, "s-a conectat cu succes ");
    strcat(raspuns, nume);
    strcat(raspuns, ".");
    printf("[s-conn]trimitem mesajul inapoi..%s\n", raspuns);

    if (bytes && write(fd, raspuns, bytes) == -1)
    {
      perror("[s-conn] Eroare la write() catre client.\n");
      return 0;
    }
    conectat[fd] = 1; // clientul de la descriptorul client s-a conectat
  }
  else
  {
    printf("[s-conn]username-ul nu e valid!\n");

    /*pregatim mesajul de raspuns */
    bzero(raspuns, dim);
    strcat(raspuns, "username-ul ");
    strcat(raspuns, nume);
    strcat(raspuns, " nu e valid!");
    printf("[s-conn]trimitem mesajul inapoi..%s\n", raspuns);

    if (bytes && write(fd, raspuns, bytes) == -1)
    {
      perror("[s-conn] Eroare la write() catre client.\n");
      return 0;
    }
    conectat[fd] = 0; // clientul de la descriptorul client NU e conectat
  }

  return bytes;
}

/*-------------------------------------------------------------------------------------------------


                                      AFISARE


---------------------------------------------------------------------------------------------------*/

int afisare(char *msg, int fd, int bytes)
{
  char raspuns[dim];
  printf("[s-afisare]am primit comanda..%s\n", msg);

  // fac fisierul meniu --> trebuie sa aiba numele restaurantului
  char nume_meniu[dim] = "";
  strcat(nume_meniu, username); // username contine numele resturantului
  strcat(nume_meniu, ".txt");

  // deschid meniul restaurantului pt a citi tot meniul din el
  int fd_meniu_restaurant;
  if ((fd_meniu_restaurant = open(nume_meniu, O_RDWR)) == -1)
  {
    printf("[s-afisare]eroare la deschiderea meniului pentru CITIRE\n");
  }

  // salvez in buffer tot ce e in meniu
  char buffer[dim] = "";
  bzero(buffer, dim);
  int bytesfile = read(fd_meniu_restaurant, buffer, dim);
  buffer[bytesfile] = '\0';

  printf("[s-afisare]fisierul contine: %s\n", buffer);

  /*pregatim mesajul de raspuns */
  bzero(raspuns, dim);
  strcat(raspuns, "afisam in terminal continutul meniului..\n meniul contine:\n");
  strcat(raspuns, buffer);
  printf("[s-afisare]trimitem mesajul inapoi..%s\n", raspuns);

  if (bytes && write(fd, raspuns, bytes) == -1)
  {
    perror("[s-afisare] Eroare la write() catre client.\n");
    return 0;
  }
  return bytes;
}

/*-------------------------------------------------------------------------------------------------


                                      CREEAZA MENIU


---------------------------------------------------------------------------------------------------*/

int creeaza_meniu(char *msg, int fd, int bytes)
{
  char raspuns[dim];
  printf("[s-creeaza_meniu]am primit comanda..%s\n", msg);

  // fac fisierul meniu --> trebuie sa aiba numele restaurantului
  char nume_restaurant[dim] = "";
  strcat(nume_restaurant, username);
  strcat(nume_restaurant, ".txt");

  int fd_meniu;
  if ((fd_meniu = open(nume_restaurant, O_RDWR)) == -1)
  {
    printf("[s-creeaza-meniu]eroare la deschiderea fisierului, deci il cream\n");
    if ((fd_meniu = open(nume_restaurant, O_CREAT | O_RDWR | O_TRUNC, 0666)) == -1)
    {
      printf("[s-creeaza-meniu]eroare la crearea meniului");
      /*pregatim mesajul de raspuns */
      bzero(raspuns, dim);
      strcat(raspuns, "meniul nu a putu fi creat, mai incearca o data!");
    }
    else // meniul a fost creat cu succes
    {
      printf("[s-creeaza-meniu]meniul a fost creat cu succes");
      /*pregatim mesajul de raspuns */
      bzero(raspuns, dim);
      strcat(raspuns, "meniului a fost creat cu succes!");
      close(fd_meniu);
    }
  }
  else // fisierul exista deja
  {
    printf("[s-creeaza-meniu]meniul exista deja!");
    /*pregatim mesajul de raspuns */
    bzero(raspuns, dim);
    strcat(raspuns, "meniul exista deja!");
    close(fd_meniu);
  }

  printf("[s-creeaza_meniu]trimitem mesajul inapoi..%s\n", raspuns);
  if (bytes && write(fd, raspuns, bytes) == -1)
  {
    perror("[s-creeaza_meniu] Eroare la write() catre client.\n");
    return 0;
  }
  close(fd_meniu);
  return bytes;
}

/*-------------------------------------------------------------------------------------------------


                                      INSERARE


---------------------------------------------------------------------------------------------------*/
int verif_ins(char *msg, int fd, int bytes) // verificam daca au fost introduse toate argumentele pt comanda inserare
{
  int nr_eg = 0;

  for (int i = 0; i < strlen(msg); i++)
  {
    if (msg[i] == '=')
    {
      nr_eg++;
    }
  }

  if (nr_eg == 2)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}

/**
 * functia returneaza 1 daca a fost gasit produsul in meniul restaurantului respectiv, 0 daca nu e.
 * numele produsului pt a fi afisat la tastatura ce nume de produs a fost gasit in fisier se gaseste la numeProdus[dim]
 */
int produs_gasit(char mesaj[])
{
  char numeProdus[dim] = ""; // variabila locala, in functia produs_gasit pt numele produsului din fisierul cu meniu
  int bytesfile;
  int flag = 0;
  int fd_username;
  char buffer[dim];

  // elimin newline-ul adaugat de la read
  // int i = 0;
  // while (i < strlen(mesaj))
  // {
  //     if (mesaj[i] == '\n')
  //     {
  //         break;
  //     }
  //     i++;
  // }
  // mesaj[i] = '\0';
  // printf("[server]Mesajul a fost modificat...%s\n", mesaj);

  // fac fisierul meniu --> trebuie sa aiba numele restaurantului
  char nume_restaurant[dim] = "";
  strcat(nume_restaurant, username);
  strcat(nume_restaurant, ".txt");

  // deschid fisierul cu meniul restaurantului
  if ((fd_username = open(nume_restaurant, O_RDWR)) == -1)
  {
    printf("[s-prod-gasit]eroare la deschiderea fisierului");
  }

  char *PRODUS = &mesaj[0]; // iau numele produsului din mesaj pt ca dupa aceea il "stric"

  bytesfile = read(fd_username, buffer, dim);
  buffer[bytesfile] = '\0';

  printf("[s-prod-gasit]fiserul contine: %s\n", buffer);
  // printf("%s", buffer);

  close(fd_username); // am terminat cu citirea din meniul restaurantului

  // verif daca exista vreun produs cu numele furnizat in fisier
  strcpy(numeProdus, ""); // fac numele produsului null ca sa nu interfereze cu celelalte comenzi
  char copienumeProdus[dim] = "";
  char *token = strtok(buffer, "\n=");
  while (token != NULL)
  {
    printf("[s-prod-gasit]am intrat in while!!!!!\n");
    fflush(stdout);
    printf("[s-prod-gasit]token: %s \n", token);
    strcpy(copienumeProdus, token);
    if (strcmp(string_tolower(token), string_tolower(PRODUS)) == 0)
    {
      flag = 1;
      break;
    }
    token = strtok(NULL, "\n=");
  }

  if (flag == 1)
  {
    strcpy(numeProdus, copienumeProdus); // daca produsul a fost gasit, salvam numele
  }

  return flag;
}

int insereaza_produs(char nume_produs[], char ingrediente[], char pret[]) // functie pt inserarea unui produs nou in meniu, nu mai facem verificarea existentei produsului in meniu pt ca asta se testeaza in functia inserare
{
  int flag = 0; // daca se creeaza bufferul cu elemente care sunt de sters, presupunem ca s-a sters produsul cu succes
  // fac fisierul meniu --> trebuie sa aiba numele restaurantului
  char nume_meniu[dim] = "";
  strcat(nume_meniu, username); // username contine numele resturantul
  strcat(nume_meniu, ".txt");
  // char nume_meniu[dim]="mamma mia.txt";

  // deschid meniul restaurantului pt a citi tot meniul din el
  int fd_meniu_restaurant;
  if ((fd_meniu_restaurant = open(nume_meniu, O_RDWR)) == -1)
  {
    printf("[s-ins-prod]eroare la deschiderea meniului pentru CITIRE\n");
  }

  // salvez in buffer tot ce e in meniu
  char buffer[dim] = "";
  bzero(buffer, dim);
  int bytes = read(fd_meniu_restaurant, buffer, dim);
  buffer[bytes] = '\0';

  printf("[s-ins-prod]fisierul contine: %s\n", buffer);

  // copiez intr-un alt buffer temporar toate produsele din meniu
  char copie_buffer[dim] = "";
  char *token = strtok(buffer, "\n");
  while (token != NULL)
  {
    printf("[s-ins-prod]am intrat in while!!!!!\n");
    fflush(stdout);
    printf("[s-ins-prod]token:%s\n", token);
    strcat(copie_buffer, token);
    strcat(copie_buffer, "\n");
    flag = 1;
    token = strtok(NULL, "\n");
  }
  // adaugam si noul produs
  strcat(copie_buffer, nume_produs);
  strcat(copie_buffer, "=");
  strcat(copie_buffer, ingrediente);
  strcat(copie_buffer, "=");
  strcat(copie_buffer, pret);
  strcat(copie_buffer, "\n");
  int lungime_copie_buffer = strlen(copie_buffer);
  copie_buffer[lungime_copie_buffer] = '\0';

  // printf("teoretic aici am terminat de modificat bufferul. ar trebui rescris fisierul in continuare..\n");

  if (ftruncate(fd_meniu_restaurant, 0) == -1)
  {
    printf("[s-ins-prod]eroare la stergerea continutului fisierului\n");
  }

  if (lseek(fd_meniu_restaurant, 0, SEEK_SET) == -1)
  {
    printf("[s-ins-prod]eroare la mutarea cursorului la inceputul programului\n");
  }

  if (bytes && write(fd_meniu_restaurant, copie_buffer, /*lungime_buffer*/ lungime_copie_buffer) == -1)
  {
    printf("[s-ins-prod]eroare la scriere in fisier a bufferului modificat\n");
  }

  // close(fd_meniu_restaurant); // am terminat cu rescrierea meniului de restaurant
  return flag;
}

int inserare(char *msg, int fd, int bytes)
{
  char raspuns[dim];
  char nume_produs[dim] = "";
  char ingrediente[dim] = "";
  char pret[dim] = "";
  int i, j, k;

  for (i = 11; msg[i] != '='; i++)
  {
    nume_produs[i - 11] = msg[i]; // "inserare : "--> 11
    j = i;
  }
  j = j + 2;
  for (i = j; msg[i] != '='; i++)
  {
    ingrediente[i - j] = msg[i];
    k = i;
  }
  k = k + 2;
  for (i = k; msg[i] != '\0'; i++)
  {
    pret[i - k] = msg[i];
  }
  printf("[s-ins]produs..%s\n", nume_produs);
  printf("[s-ins]ingrediente..%s\n", ingrediente);
  printf("[s-ins]pret..%s\n", pret);
  fflush(stdout);
  printf("[s-ins]am primit comanda..%s\n", msg);
  fflush(stdout);

  if (produs_gasit(nume_produs) == 1) // exista deja produsul cu numele acesta
  {
    printf("[s-ins]produsul %s exista deja! nu-l poti insera din nou!", nume_produs);
    bzero(raspuns, dim);
    strcat(raspuns, "produsul ");
    strcat(raspuns, nume_produs);
    strcat(raspuns, " exista deja! nu-l poti insera din nou!");
  }
  else // produsul nu exista in meniu
  {
    if (insereaza_produs(nume_produs, ingrediente, pret) == 1) // URGENT functie pt inserare produs in meniu
    {
      printf("[s-ins]produsul %s a fost adaugat cu succes!", nume_produs);
      bzero(raspuns, dim);
      strcat(raspuns, "produsul ");
      strcat(raspuns, nume_produs);
      strcat(raspuns, " a fost adaugat cu succes!");
    }
    else
    {
      printf("[s-ins]eroare la adaugarea produsului %s", nume_produs);
      bzero(raspuns, dim);
      strcat(raspuns, "produsul ");
      strcat(raspuns, nume_produs);
      strcat(raspuns, " nu s-a adaugat, mai incearca o data!");
    }
  }

  printf("[s-ins]trimitem mesajul inapoi..%s\n", raspuns);

  if (bytes && write(fd, raspuns, bytes) == -1)
  {
    perror("[s-ins] Eroare la write() catre client.\n");
    return 0;
  }

  return bytes;
}

/*-------------------------------------------------------------------------------------------------


                                      MODIFICARE


---------------------------------------------------------------------------------------------------*/
int verif_modificare(char *msg, int fd, int bytes) // verificam daca au fost introduse toate argumentele pt comanda modificare
{
  int nr_eg = 0;

  for (int i = 0; i < strlen(msg); i++)
  {
    if (msg[i] == '=')
    {
      nr_eg++;
    }
  }

  if (nr_eg == 1)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}

/**
 * returneaza 1 daca a reusit sa modifice atributul, 0 altfel
 */
int modifica_produs(char produs[], char atribut[], char mesaj_modif[])
{ // fac fisierul meniu --> trebuie sa aiba numele restaurantului
  char nume_meniu[dim] = "";
  strcat(nume_meniu, username); // username contine numele resturantului
  strcat(nume_meniu, ".txt");
  // char nume_meniu[dim]="mamma mia.txt";

  // deschid meniul restaurantului pt a citi tot meniul din el
  int fd_meniu_restaurant;
  if ((fd_meniu_restaurant = open(nume_meniu, O_RDWR)) == -1)
  {
    printf("[s-modif-prod]eroare la deschiderea meniului pentru CITIRE\n");
  }

  // salvez in buffer tot ce e in meniu
  char buffer[dim] = "";
  bzero(buffer, dim);
  int bytesfile = read(fd_meniu_restaurant, buffer, dim);
  buffer[bytesfile] = '\0';

  printf("[s-modif-prod]fisierul contine: %s\n", buffer);

  // verific atributul sa vad ce e
  int val_atribut = 0;
  if (strcmp(atribut, "nume") == 0)
  {
    val_atribut = 1;
  }
  else if (strcmp(atribut, "ingrediente") == 0)
  {
    val_atribut = 2;
  }
  else if (strcmp(atribut, "pret") == 0)
  {
    val_atribut = 3;
  }

  int flag = 0; // modificarea a avut succes --> 1

  // copiez intr-un alt buffer temporar produsele care nu trebuie sterse si apoi il trimit la write catre meniu
  int lungime_produs = strlen(produs);
  char copie_buffer[dim] = "";
  char copie_token[dim] = "";
  char temp[dim]; // variabila care stocheaza diferite parti din token la un moment dat
  bzero(temp, dim);
  char *token = strtok(buffer, "\n");
  while (token != NULL)
  {
    printf("[s-modif-prod]am intrat in while!!!!!\n");
    fflush(stdout);
    strcpy(copie_token, token);
    if (strncmp(string_tolower(token), string_tolower(produs), lungime_produs) != 0) // produsele care nu trebuiesc sterse
    {
      printf("[s-modif-prod]token:%s\n", copie_token);
      strcat(copie_buffer, copie_token);
      strcat(copie_buffer, "\n");
    }
    else // produsul care trebuie modificat
    {
      if (val_atribut == 1) // numele trebuie modificat
      {
        int nr = 0;
        strcat(copie_buffer, mesaj_modif);
        strcat(copie_buffer, "=");
        char *tok = strtok(copie_token, "=");
        while (tok != NULL)
        {
          if (nr == 1)
          {
            strcat(copie_buffer, tok);
            strcat(copie_buffer, "=");
          }
          if (nr == 2)
          {
            strcat(copie_buffer, tok);
          }
          nr++;
          tok = strtok(NULL, "=");
        }
        strcat(copie_buffer, "\n");
        flag = 1;
      }
      if (val_atribut == 2) // ingredientele trebuie modificate
      {
        int nr = 0;
        char *tok = strtok(copie_token, "=");
        while (tok != NULL)
        {
          if (nr == 0)
          {
            strcat(copie_buffer, tok);
            strcat(copie_buffer, "=");
          }
          else if (nr == 2)
          {
            strcat(copie_buffer, tok);
          }
          else
          {
            strcat(copie_buffer, mesaj_modif);
            strcat(copie_buffer, "=");
          }
          nr++;
          tok = strtok(NULL, "=");
        }
        strcat(copie_buffer, "\n");
        flag = 1;
      }
      if (val_atribut == 3) // pretul trebuie modificat
      {
        int nr = 0;
        char *tok = strtok(copie_token, "=");
        while (tok != NULL)
        {
          if (nr == 0 || nr == 1)
          {
            strcat(copie_buffer, tok);
            strcat(copie_buffer, "=");
          }
          else
          {
            strcat(copie_buffer, mesaj_modif);
          }
          nr++;
          tok = strtok(NULL, "=");
        }
        strcat(copie_buffer, "\n");
        flag = 1;
      }
    }
    token = strtok(NULL, "\n");
  }
  int lungime_copie_buffer = strlen(copie_buffer);
  copie_buffer[lungime_copie_buffer] = '\0';

  printf("[s-modif-prod]teoretic aici am terminat de modificat bufferul. ar trebui rescris fisierul in continuare..\n");

  if (ftruncate(fd_meniu_restaurant, 0) == -1)
  {
    printf("[s-modif-prod]eroare la stergerea continutului fisierului\n");
  }

  if (lseek(fd_meniu_restaurant, 0, SEEK_SET) == -1)
  {
    printf("[s-modif-prod]eroare la mutarea cursorului la inceputul programului\n");
  }

  if (bytesfile && write(fd_meniu_restaurant, copie_buffer, /*lungime_buffer*/ lungime_copie_buffer) == -1)
  {
    printf("[s-modif-prod]eroare la scriere in fisier a bufferului modificat\n");
  }

  // close(fd_meniu_restaurant); // am terminat cu rescrierea meniului de restaurant
  return flag;
}

int modificare(char *msg, int fd, int bytes)
{
  char raspuns[dim];
  char produs[dim] = "";
  char atribut[dim] = "";
  int i, j;

  for (i = 13; msg[i] != '='; i++)
  {
    produs[i - 13] = msg[i]; // modificare : --> 13
    j = i;
  }
  j = j + 2;
  for (i = j; msg[i] != '\0'; i++)
  {
    atribut[i - j] = msg[i];
  }
  printf("[s-modif]produs..%s\n", produs);
  printf("[s-modif]atribut..%s\n", atribut);

  printf("[s-modif]am primit comanda..%s\n", msg);
  fflush(stdout);

  // verificam daca produsul exista
  if (produs_gasit(produs) == 1) // produsul a fost gasit
  {
    /*pregatim mesajul de raspuns */
    bzero(raspuns, dim);
    strcat(raspuns, "cu ce vrei sa modifici la ");
    strcat(raspuns, atribut);
    strcat(raspuns, "?");
    printf("[s-modif]trimitem mesajul inapoi..%s\n", raspuns);
    fflush(stdout);

    if (bytes && write(fd, raspuns, bytes) == -1)
    {
      perror("[s-modif] Eroare la write() catre client.\n");
      return 0;
    }

    wait(1);

    // asteptam mesajul de la client
    char mesaj_modif[dim] = "";
    char buffer[dim];
    bytes = read(fd, mesaj_modif, sizeof(buffer));
    if (bytes < 0)
    {
      perror("[s-modif]Eroare la read() de la client.\n");
      return 0;
    }
    printf("[s-modif]Mesajul a fost receptionat...%s\n", mesaj_modif);

    // elimin newline
    for (int i = 0; i < dim; i++)
      if (mesaj_modif[i] == '\n')
        mesaj_modif[i] = '\0';

    printf("[s-modif]am modificat...%s\n", mesaj_modif);

    // modificam atributul corespunzator cu informatiile noi primite
    if (modifica_produs(produs, atribut, mesaj_modif) == 1) // modificarea produsului a avut succes
    {                                                       // trimitem confirmare la client
      bzero(raspuns, dim);
      strcat(raspuns, "am actualizat atributul ");
      strcat(raspuns, atribut);
      strcat(raspuns, " cu ");
      strcat(raspuns, mesaj_modif);
      strcat(raspuns, ".");
      printf("[s-modif]trimitem mesajul inapoi..%s\n", raspuns);
    }
    else
    {
      bzero(raspuns, dim);
      strcat(raspuns, "atributul ");
      strcat(raspuns, atribut);
      strcat(raspuns, " nu a putut fi actualizat cu ");
      strcat(raspuns, mesaj_modif);
      strcat(raspuns, ". mai incearca o data!");
      printf("[s-modif]trimitem mesajul inapoi..%s\n", raspuns);
    }

    if (bytes && write(fd, raspuns, bytes) == -1)
    {
      perror("[s-modif] Eroare la write() catre client.\n");
      return 0;
    }
  }
  else
  {
    // trimitem mesaj la client
    bzero(raspuns, dim);
    strcat(raspuns, "produsul ");
    strcat(raspuns, produs);
    strcat(raspuns, " nu exista!");
    printf("[s-modif]trimitem mesajul inapoi..%s\n", raspuns);

    if (bytes && write(fd, raspuns, bytes) == -1)
    {
      perror("[s-modif] Eroare la write() catre client.\n");
      return 0;
    }
  }

  return bytes;
}

/*-------------------------------------------------------------------------------------------------


                                      STERGERE


---------------------------------------------------------------------------------------------------*/

int verif_sterg(char *msg, int fd, int bytes) // verificam daca au fost introduse toate argumentele pt comanda stergere
{
  int nr_spatii = 0;

  for (int i = 0; i < strlen(msg); i++)
  {
    if (msg[i] == ' ')
    {
      nr_spatii++;
    }
  }

  if (nr_spatii == 2)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}

/**
 * returneaza 1 daca produsul a fost sters cu succes, 0 altfel
 */
int sterge_produs(char Nume_Produs[])
{
  int flag = 0; // daca se creeaza bufferul cu elemente care sunt de sters, presupunem ca s-a sters produsul cu succes
  // fac fisierul meniu --> trebuie sa aiba numele restaurantului
  char nume_meniu[dim] = "";
  strcat(nume_meniu, username); // username contine numele resturantul
  strcat(nume_meniu, ".txt");
  // char nume_meniu[dim]="mamma mia.txt";

  // deschid meniul restaurantului pt a citi tot meniul din el
  int fd_meniu_restaurant;
  if ((fd_meniu_restaurant = open(nume_meniu, O_RDWR)) == -1)
  {
    printf("[s-sterg-prod]eroare la deschiderea meniului pentru CITIRE\n");
  }

  // salvez in buffer tot ce e in meniu
  char buffer[dim] = "";
  bzero(buffer, dim);
  int bytes = read(fd_meniu_restaurant, buffer, dim);
  buffer[bytes] = '\0';

  printf("[s-sterg-prod]fisierul contine: %s\n", buffer);

  // incerc sa modific continutul bufferului
  //  int j = 0;           // iterez in numele produsului
  //  int copie_i = 0;     // salvez unde e finalul tokenului(descrierea produsului) in buffer
  //  int flag_buffer = 0; // flagul asta e pt a testa daca gasesc produsul in buffer. daca e 1 inseamna ca am gasit produsul --> produsul trebuie sa-l gasesc oricum in buffer pt ca am testat deja in functia produs_gasit care e in main
  //  for (int i = 0; buffer[i] != '\0'; i++) //iterez in meniu sa vad unde e produsul
  //  {
  //      if (buffer[i] == Nume_Produs[j] && j < lungime_produs)
  //      {
  //          flag_buffer = 1;
  //          j++;
  //          copie_i = i;
  //      }
  //  }
  //  if (flag_buffer == 1)
  //  {
  //      printf("flag_buffer este eg cu 1\n");
  //  }
  //  int k;                                  // iterez in buffer de la inceputul tokenului
  //  for (k = copie_i - lungime_produs; k < lungime_produs; k++)
  //  {
  //      if (k == copie_i - lungime_produs)
  //      {
  //          buffer[k] = '\n';
  //      }
  //      else
  //      {
  //          buffer[k] = ' ';
  //      }
  //  }

  // copiez intr-un alt buffer temporar produsele care nu trebuie sterse si apoi il trimit la write catre meniu
  int lungime_produs = strlen(Nume_Produs);
  char copie_buffer[dim] = "";
  char copie_token[dim] = "";
  char *token = strtok(buffer, "\n");
  while (token != NULL)
  {
    printf("[s-sterg-prod]am intrat in while!!!!!\n");
    fflush(stdout);
    strcpy(copie_token, token);
    if (strncmp(string_tolower(token), string_tolower(Nume_Produs), lungime_produs) != 0) // produsele care nu trebuie sterse
    {
      printf("[s-sterg-prod]token:%s\n", copie_token);
      strcat(copie_buffer, copie_token);
      strcat(copie_buffer, "\n");
      flag = 1;
    }
    token = strtok(NULL, "\n");
  }
  int lungime_copie_buffer = strlen(copie_buffer);
  copie_buffer[lungime_copie_buffer] = '\0';

  // printf("teoretic aici am terminat de modificat bufferul. ar trebui rescris fisierul in continuare..\n");

  if (ftruncate(fd_meniu_restaurant, 0) == -1)
  {
    printf("[s-sterg-prod]eroare la stergerea continutului fisierului\n");
  }

  if (lseek(fd_meniu_restaurant, 0, SEEK_SET) == -1)
  {
    printf("[s-sterg-prod]eroare la mutarea cursorului la inceputul programului\n");
  }

  // if((fd_meniu_restaurant=open(nume_meniu,O_RDWR|O_TRUNC))==-1)
  // {
  //     printf("[s-sterg-prod]eroare la deschiderea meniului pentru scriere\n");
  // }
  // //int lungime_buffer=strlen(buffer);

  if (bytes && write(fd_meniu_restaurant, copie_buffer, /*lungime_buffer*/ lungime_copie_buffer) == -1)
  {
    printf("[s-sterg-prod]eroare la scriere in fisier a bufferului modificat\n");
  }

  // close(fd_meniu_restaurant); // am terminat cu rescrierea meniului de restaurant
  return flag;
}

int stergere(char *msg, int fd, int bytes)
{
  char raspuns[dim];
  char produs[dim] = "";
  int i;

  for (i = 11; msg[i] != '\0'; i++)
  {
    produs[i - 11] = msg[i]; // "stergere : "--> 11
  }

  printf("[s-sterg]produs..%s\n", produs);
  fflush(stdout);
  printf("[s-sterg]am primit comanda..%s\n", msg);
  fflush(stdout);

  if (produs_gasit(produs) == 1) // exista produsul cu numele acesta
  {
    printf("[s-sterg]produsul %s exista!", produs);
    if (sterge_produs(produs) == 1) // produsul a fost sters cu succes
    {
      printf("[s-sterg]produsul %s a fost sters cu succes!\n", produs);
      bzero(raspuns, dim);
      strcat(raspuns, "produsul ");
      strcat(raspuns, produs);
      strcat(raspuns, " a fost sters cu succes!");
    }
    else // eroare la stergere
    {
      printf("[s-sterg]eroare la stergerea produsului %s", produs);
      bzero(raspuns, dim);
      strcat(raspuns, "produsul ");
      strcat(raspuns, produs);
      strcat(raspuns, " nu s-a sters, mai incearca o data!");
    }
  }
  else // produsul nu exista in meniu
  {
    printf("[s-sterg]produsul NU %s exista!", produs);
    bzero(raspuns, dim);
    strcat(raspuns, "produsul ");
    strcat(raspuns, produs);
    strcat(raspuns, " NU exista! deci nu ai cum sa-l stergi");
  }

  printf("[s-sterg]trimitem mesajul inapoi..%s\n", raspuns);

  if (bytes && write(fd, raspuns, bytes) == -1)
  {
    perror("[s-sterg] Eroare la write() catre client.\n");
    return 0;
  }

  return bytes;
}

/*-------------------------------------------------------------------------------------------------


                                      DECONECTARE


---------------------------------------------------------------------------------------------------*/

int deconectare(char *msg, int fd, int bytes)
{
  char raspuns[dim] = "";

  printf("[s-deconn]am primit comanda..%s\n", msg);

  /*pregatim mesajul de raspuns */
  bzero(raspuns, dim);
  // strcat(raspuns, "[s-deconn]S-a deconectat clientul cu descriptorul \n");
  // strcat(raspuns, fd);
  // strcat(raspuns, '!');
  snprintf(raspuns, dim, "S-a deconectat clientul cu descriptorul %d!\n", fd);
  printf("[s-deconn]trimitem mesajul inapoi..%s\n", raspuns);
  fflush(stdout);

  if (bytes && write(fd, raspuns, bytes) == -1)
  {
    perror("[s-deconn]Eroare la write() catre client.\n");
    return 0;
  }
  conectat[fd] = 0; // clientul de la descriptorul fd s-a deconectat
  return bytes;
}

/*-------------------------------------------------------------------------------------------------


                                      verificarea instructiunilor
                                      --> fara_conexiune
                                      --> eroare_sintactica


---------------------------------------------------------------------------------------------------*/
int fara_conexiune(char *msg, int fd, int bytes)
{
  char raspuns[dim];
  printf("[s-err]am primit comanda..%s\n", msg);

  /*pregatim mesajul de raspuns */
  bzero(raspuns, dim);
  strcat(raspuns, "nu te-ai conectat!");
  printf("[s-err]trimitem mesajul inapoi..%s\n", raspuns);

  if (bytes && write(fd, raspuns, bytes) == -1)
  {
    perror("[s-err] Eroare la write() catre client.\n");
    return 0;
  }
  return bytes;
}

int eroare_sintactica(char *msg, int fd, int bytes)
{
  char raspuns[dim];
  int on = 0;
  printf("[s-instr]am primit comanda..%s\n", msg);

  if (msg[13] == '\n')
    printf("da");

  /*pregatim mesajul de raspuns */
  bzero(raspuns, dim);

  //----------------------------- CONECTARE-----------
  if (strncmp(msg, "conectare : ", 12) == 0 && isalpha(msg[12]) == 0) // issalpha !=0 caracter alfabetic
  {
    on = 1;
    printf("[s-instr-conn1]sintaxa comenzii este: \"conectare : <nume>\"\n");
    strcat(raspuns, "sintaxa comenzii este: \"conectare : <nume>\"");
  }
  if (strncmp(msg, "conectare : ", 12) == 0 && isalpha(msg[12]) != 0 && conectat[fd] == 1) // isspace !=0 caracter alfabetic
  {
    on = 1;
    printf("[s-instr-conn2]te-ai conectat deja! nu te mai poti conecta inca o data!\n");
    strcat(raspuns, "te-ai conectat deja! nu te mai poti conecta inca o data!");
  }

  //----------------------------- AFISARE-----------
  if (strncmp(msg, "afisare", 7) == 0 && isspace(msg[7]) != 0 && conectat[fd] == 1) // daca are ceva dupa afisare
  {
    on = 1;
    printf("[s-instr-afis]sintaxa comenzii este: \"afisare\"\n");
    strcat(raspuns, "sintaxa comenzii este: \"afisare\"");
  }

  //----------------------------CREAZA MENIU-----------
  if (strncmp(msg, "creeaza meniu", 6) == 0 && isspace(msg[6]) != 0 && conectat[fd] == 1)
  {
    on = 1;
    printf("[s-instr-exp]sintaxa comenzii este: \"creeaza meniu\"\n");
    strcat(raspuns, "sintaxa comenzii este: \"creeaza meniu\"");
  }

  //-----------------------------INSERARE-----------
  int ins = verif_ins(msg, fd, bytes);
  if (strncmp(msg, "inserare", 8) == 0 && ins == 1 && conectat[fd] == 1)
  {
    on = 1;
    printf("[s-instr-ins]sintaxa comenzii este: \"inserare : <denumire produs>=<ingredient 1,ingredient 2>=<pret>\"\n");
    strcat(raspuns, "sintaxa comenzii este: \"inserare : <denumire produs>=<ingredient 1,ingredient 2>=<pret>\"");
  }

  //----------------------------- MODIFICARE-----------
  int modif = verif_modificare(msg, fd, bytes);
  if (strncmp(msg, "modificare", 10) == 0 && modif == 1 && conectat[fd] == 1)
  {
    on = 1;
    printf("[s-instr-modif]sintaxa comenzii este: \"modificare : <denumire produs> <atribut produs(ingrediente/pret)>\"\n");
    strcat(raspuns, "sintaxa comenzii este: \"modificare : <denumire produs> <atribut produs(ingrediente/pret)>\"");
  }

  //-----------------------------STERGERE-----------
  int sterg = verif_sterg(msg, fd, bytes);
  if (strncmp(msg, "stergere", 8) == 0 && sterg == 1 && conectat[fd] == 1)
  {
    on = 1;
    printf("[s-instr-sterg]sintaxa comenzii este: \"stergere : <denumire produs>\"\n");
    strcat(raspuns, "sintaxa comenzii este: \"stergere : <denumire produs>\"");
  }

  //----------------------------- DECONECTARE-----------
  if (strncmp(msg, "deconectare", 11) == 0 && isspace(msg[11]) != 0 && conectat[fd] == 1)
  {
    on = 1;
    printf("[s-instr-deconn]sintaxa comenzii este: \"deconectare\"\n");
    strcat(raspuns, "sintaxa comenzii este: \"deconectare\"");
  }

  if (on == 1)
  {
    wait(1);

    printf("[s-instr]trimitem mesajul inapoi..%s\n", raspuns);

    if (bytes && write(fd, raspuns, bytes) == -1)
    {
      perror("[s-instr] Eroare la write() catre client.\n");
      return 0;
    }
    return bytes;
  }
  else
    return 0;
}

/*------------------------------------------------------------------------------------------------


                                      programul


--------------------------------------------------------------------------------------------------*/
int main()
{
  struct sockaddr_in server; /* structurile pentru server si clienti */
  struct sockaddr_in from;
  fd_set readfds;    /* multimea descriptorilor de citire */
  fd_set actfds;     /* multimea descriptorilor activi */
  struct timeval tv; /* structura de timp pentru select() */
  int sd, client;    /* descriptori de socket */
  int optval = 1;    /* optiune folosita pentru setsockopt()*/
  int fd;            /* descriptor folosit pentru
                  parcurgerea listelor de descriptori */
  int nfds;          /* numarul maxim de descriptori */
  int len;           /* lungimea structurii sockaddr_in */

  /* creare socket */
  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("[server] Eroare la socket().\n");
    return errno;
  }

  /*setam pentru socket optiunea SO_REUSEADDR */
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

  /* pregatim structurile de date */
  bzero(&server, sizeof(server));

  /* umplem structura folosita de server */
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  server.sin_port = htons(PORT);

  /* atasam socketul */
  if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
  {
    perror("[server] Eroare la bind().\n");
    return errno;
  }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen(sd, 10) == -1)
  {
    perror("[server] Eroare la listen().\n");
    return errno;
  }

  /* completam multimea de descriptori de citire */
  FD_ZERO(&actfds);    /* initial, multimea este vida */
  FD_SET(sd, &actfds); /* includem in multime socketul creat */

  tv.tv_sec = 1; /* se va astepta un timp de 1 sec. */
  tv.tv_usec = 0;

  /* valoarea maxima a descriptorilor folositi */
  nfds = sd;

  printf("[server] Asteptam la portul %d...\n", PORT);
  fflush(stdout);

  /* servim in mod concurent (!?) clientii... */
  while (1)
  {
    /* ajustam multimea descriptorilor activi (efectiv utilizati) */
    bcopy((char *)&actfds, (char *)&readfds, sizeof(readfds));

    /* apelul select() */
    if (select(nfds + 1, &readfds, NULL, NULL, &tv) < 0)
    {
      perror("[server] Eroare la select().\n");
      return errno;
    }
    /* vedem daca e pregatit socketul pentru a-i accepta pe clienti */
    if (FD_ISSET(sd, &readfds))
    {
      /* pregatirea structurii client */
      len = sizeof(from);
      bzero(&from, sizeof(from));

      /* a venit un client, acceptam conexiunea */
      client = accept(sd, (struct sockaddr *)&from, &len);

      /* eroare la acceptarea conexiunii de la un client */
      if (client < 0)
      {
        perror("[server] Eroare la accept().\n");
        continue;
      }

      if (nfds < client) /* ajusteaza valoarea maximului */
        nfds = client;

      /* includem in lista de descriptori activi si acest socket */
      FD_SET(client, &actfds);

      printf("[server] S-a conectat clientul cu descriptorul %d, de la adresa %s.\n", client, conv_addr(from));
      fflush(stdout);

      //    acum clientul este conectat la sever(s-a facut conexiunea prin sockets),
      // mai departe verificam daca se logheaza
    }
    /* vedem daca e pregatit vreun socket client pentru a trimite raspunsul */
    for (fd = 0; fd <= nfds; fd++) /* parcurgem multimea de descriptori */
    {
      /* este un socket de citire pregatit? */ // si clientul de la descriptor connectat?
      if (fd != sd && FD_ISSET(fd, &readfds))
      {
        // verificam instructiunea pe care o primeste serverul daca este valida
        char buffer[dim];
        int bytes;
        bytes = read(fd, msg, sizeof(buffer));
        if (bytes < 0)
        {
          perror("[server]Eroare la read() de la client.\n");
          return 0;
        }
        printf("[server]Mesajul a fost receptionat...%s\n", msg);
        // verificam instructiunile
        if (eroare_sintactica(msg, fd, bytes) == 0)
        {
          //          CONECTARE
          if (strncmp(msg, "conectare : ", 12) == 0 && isalpha(msg[12]) != 0 && conectat[fd] == 0)
          {
            if (conectare(msg, fd, bytes))
              printf("[server]am executat comanda conn\n");
            else
              printf("[server]eroare la comanda conn\n");
          } //        AFISARE
          else if (conectat[fd] == 1)
          {
            if (strcmp(msg, "afisare") == 0 && msg[7] != '\n') // verif daca am scris doar afisare, daca am scris "afisare " o sa dea eroare
            {
              if (afisare(msg, fd, bytes))
                printf("[server]am executat comanda afisare\n");
              else
                printf("[server]eroare la afisare\n");
            } //        creeaza_meniu
            else if (strcmp(msg, "creeaza meniu") == 0 && msg[13] != '\n') // verif daca am scris doar creeaza meniu
            {
              if (creeaza_meniu(msg, fd, bytes))
                printf("[server]am executat comanda creeaza meniu\n");
              else
                printf("[server]eroare la creeaza meniu\n");
            } //        INSERARE
            else if (strncmp(msg, "inserare : ", 11) == 0) //"inserare : "--> 11
            {
              if (verif_ins(msg, fd, bytes) == 0) // daca da 1 inseamna ca clientul nu a introdus destule argumente pt comanda inserare.
              {
                if (inserare(msg, fd, bytes))
                  printf("[server]am executat comanda inserare\n");
                else
                  printf("[server]eroare la inserare\n");
              }
            } //        MODIFICARE
            else if (strncmp(msg, "modificare : ", 13) == 0) //"modificare : "--> 13
            {
              if (verif_modificare(msg, fd, bytes) == 0) // daca da 1 inseamna ca clientul nu a introdus destule argumente pt comanda modificare.
              {
                if (modificare(msg, fd, bytes))
                  printf("[server]am executat comanda modificare\n");
                else
                  printf("[server]eroare la modificare\n");
              }
            } //        stergere
            else if (strncmp(msg, "stergere : ", 11) == 0) //"stergere : "--> 11
            {
              if (verif_sterg(msg, fd, bytes) == 0) // daca da 1 inseamna ca clientul nu a introdus destule argumente pt comanda stergere.
              {
                if (stergere(msg, fd, bytes))
                  printf("[server]am executat comanda stergere\n");
                else
                  printf("[server]eroare la stergere\n");
              }
            } //        DECONECTARE
            else if (strcmp(msg, "deconectare") == 0 && msg[11] != '\n') // verif daca am scris doar deconectare
            {
              if (deconectare(msg, fd, bytes))
              {
                printf("[server]am executat comanda deconn\n");
                close(fd);           /* inchidem conexiunea cu clientul */
                FD_CLR(fd, &actfds); /* scoatem si din multime */
              }
              else
                printf("[server]eroare la deconectare\n");
            }
          }
          else
            fara_conexiune(msg, fd, bytes);
        } // if verif cu instructiuni
      } // if cu socket pregatit
    } /* for */
  } /* while */
} /* main */