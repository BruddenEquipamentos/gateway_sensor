#include <stdlib.h>
#include <stdio.h>
#include "listenc.h"
#include "Arduino.h"

TLista criaLista() {
    TLista c = (tcabec*) malloc(sizeof(tcabec));

    c->tamanho = 0;
    c->primeiro = NULL;
    c->ultimo = NULL;

    return c;
}

TLista appendLista(TLista lst, int id) {
    pnoh novono = (tipoNo*) malloc(sizeof(tipoNo));

    novono->id = id;

    novono->anterior = lst->ultimo;
    novono->proximo = NULL;

    if (lst->tamanho == 0) {
        lst->primeiro = novono;
    } else {
        lst->ultimo->proximo = novono;
    }

    lst->ultimo = novono;
    lst->tamanho++;

    return lst;
}

int lenLista(TLista lst) {
    return lst->tamanho;
}

int primLista(TLista lst) {
    return lst->primeiro->id;
}

int ultLista(TLista lst) {
    return lst->ultimo->id;
}

//Não usar
TLista insertLista(TLista lst, int i, int id)
{
    int tam = lenLista(lst);

    if ((i < 0) || (i > tam)) {
        return NULL;
    }

    if ((lenLista(lst) == 0) || (i == tam)) {
        appendLista(lst, id);
    } else {
        pnoh novono = (tipoNo*) malloc(sizeof(tipoNo));
        novono->id = id;

        if (i == 0) {
            novono->proximo = lst->primeiro;
            lst->primeiro = novono;
        } else {
            pnoh aux = lst->primeiro;
            int pos = 0;

            while (pos != (i - 1))
            {
                aux = aux->proximo;
                pos++;
            }
            novono->proximo = aux->proximo;
            aux->proximo = novono;
        }

        lst->tamanho++;
    }

    return lst;
}

pnoh infoLista(TLista lst, int i) {
    int tam = lenLista(lst);

    if ((tam == 0) || (i < 0) || (i > tam-1)) {
    	//printf("Caiu");
        return NULL;
    }

    if (i == 0) {
        return lst->primeiro;
    }

    if (i == tam - 1) {
        return lst->ultimo;
    }

    pnoh aux = lst->primeiro;
    int pos = 0;

    while (pos != i) {
        aux = aux->proximo;
        pos++;
    }

    return aux;
}

void removeLista(TLista lst, int i) {
    int tam = lenLista(lst);

    if ((i < 0) || (i > tam-1) || (tam == 0)) {
        printf("Erro: indice inexistente dentro da Lista.");
        return;
    }

    if (tam == 1) {
        pnoh aux = lst->primeiro;
        lst->primeiro = NULL;
        lst->ultimo = NULL;
        lst->tamanho--;

        free(aux);
    } else {
        if (i == 0) {
            pnoh aux = lst->primeiro;
            lst->primeiro = aux->proximo;
            lst->tamanho--;
            free(aux);
        } else {
            if (i == tam - 1) {
                pnoh aux = lst->ultimo;

                pnoh penultimo = lst->primeiro;
                int pos = 0;

                while (pos != i - 1) {
                    penultimo = penultimo->proximo;
                    pos++;
                }

                penultimo->proximo = NULL;
                lst->ultimo = penultimo;

                lst->tamanho--;
                free(aux);
            } else {
                pnoh anterior = lst->primeiro;
                int pos = 0;

                while (pos != i - 1) {
                    anterior = anterior->proximo;
                    pos++;
                }

                pnoh aux = anterior->proximo;
                anterior->proximo = aux->proximo;
                lst->tamanho--;
                free(aux);
            }
        }
    }
}

int indexLista(TLista lst, int id) {
    int tam = lenLista(lst);
    if(tam==0)
    	return -1;
	int i = 0;
	pnoh aux = lst->primeiro;
	for(i;i<tam;i++){
		if(aux->id==id)
			return i;
		else 
			aux = aux->proximo;		
	}
    return -1;
}

TLista clearLista(TLista lst) {
    int tam = lenLista(lst);

    if (tam == 0) {
        return lst;
    }

    while (lenLista(lst) > 0) {
        removeLista(lst, 0);
    }

    return lst;
}

//Não usar
TLista clonaLista(TLista lst) {
    TLista clone = criaLista();
    int tam = lenLista(lst);

    if (tam == 0) {
        return clone;
    }

    for (int i = 0; i < tam; i++) {
        pnoh no = infoLista(lst, i);
        if (no != NULL) {
            appendLista(clone, no->id);
        }
    }

    return clone;
}


void imprimeLista(TLista lst){
	pnoh aux = lst->primeiro;
	while(aux != NULL){
		Serial.println(aux->id);
		aux = aux->proximo;
	}
}