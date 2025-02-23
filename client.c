/* 
client.c 
- TCP concurent care trimite si asteapta mesaj de la server
-instructiuni acceptate : 
// /*1.*/// conectare : <username>
// /*2.*/ afisare
// /*3.*/ creeaza meniu\n
// /*4.*/ inserare : <denumire produs>=<ingredient 1,ingredient 2>=<pret>
// /*5.*/ modificare : <denumire produs>=<atribut produs(ingrediente/pret)
// /*6.*/ stergere : <denumire produs>
// /*7.*/ deconectare

// @daniela dutescu
//*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#define dimensiune 1048

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;

int main(int argc, char *argv[])
{
  int sd;                    // descriptorul de socket
  struct sockaddr_in server; // structura folosita pentru conectare
  char msg[dimensiune];      // mesajul de la client
  char rsp[dimensiune];      // mesajul de la server

  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
  {
    printf("[client] Sintaxa: %s <adresa_server> <port>\n", argv[0]);
    return -1;
  }

  /* stabilim portul */
  port = atoi(argv[2]);

  /* cream socketul */
  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("[client] Eroare la socket().\n");
    return errno;
  }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons(port);

  /* ne conectam la server */
  if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
  {
    perror("[client]Eroare la connect().\n");
    return errno;
  }

  printf("Salutare, client! Poti alege una dintre urmatoarele comenzi:\n"
         /*1.*/ "conectare : <username>\n"
         /*2.*/ "afisare\n"
         /*3.*/ "creeaza meniu\n"
         /*4.*/ "inserare : <denumire produs>=<ingredient 1,ingredient 2>=<pret>\n"
         /*5.*/ "modificare : <denumire produs>=<atribut produs(ingrediente/pret)>\n"
         /*6.*/ "stergere : <denumire produs>\n"
         /*7.*/ "deconectare\n");

  while (1)
  {
    /* citirea mesajului */
    bzero(msg, dimensiune);
    printf("[client]Introduceti o comanda..");
    fflush(stdout);
    read(0, msg, dimensiune);

    // elimin newline
    for (int i = 0; i < dimensiune; i++)
      if (msg[i] == '\n')
        msg[i] = '\0';

    /* trimiterea mesajului la server */
    if (write(sd, msg, dimensiune) <= 0)
    {
      perror("[client]Eroare la write() spre server.\n");
      return errno;
    }

    /* citirea raspunsului dat de server
       (apel blocant pina cind serverul raspunde) */
    if (read(sd, rsp, dimensiune) < 0)
    {
      perror("[client]Eroare la read() de la server.\n");
      return errno;
    }
    /* afisam mesajul primit */
    printf("[client]Mesajul primit este..%s\n", rsp);

    /* inchidem conexiunea, am terminat */
    if (strncmp(rsp, "S-a deconectat clientul cu descriptorul",39) == 0 /*&& log == 1*/)
    {
      close(sd);
      return 0;
      // break;
    }
  } // while
} //main