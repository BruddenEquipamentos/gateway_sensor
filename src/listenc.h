#include <stdlib.h>
#include <stdio.h>
#include <esp32-hal.h>

typedef struct tno {
    int id;
    struct tno * anterior;
    struct tno * proximo;
} tipoNo;

typedef tipoNo * pnoh;

typedef struct {
    int tamanho;
    pnoh primeiro;
    pnoh ultimo;
} tcabec;

typedef tcabec * TLista;

TLista criaLista();
TLista appendLista(TLista lst, int id);
int lenLista(TLista lst);
int primLista(TLista lst);
int ultLista(TLista lst);
TLista insertLista(TLista lst, int i, int id);
pnoh infoLista(TLista lst, int i);
void removeLista(TLista lst, int i);
int indexLista(TLista lst, int id); //OK
TLista clearLista(TLista lst);
TLista clonaLista(TLista lst);
void imprimeLista(TLista lst);