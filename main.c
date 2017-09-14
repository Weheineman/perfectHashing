#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WORDSIZE 30

typedef unsigned int uns;

typedef struct _PerfectHash{
	uns sz;
	char **HT;
	int *G;
} PerfectHash;

typedef struct _BucketNode{
	char *w;
	struct _BucketNode *next;
} BucketNode;

typedef struct _Bucket{
	BucketNode *list;
	uns sz;
} Bucket;

typedef struct _BucketList{
	Bucket **arr;
	uns sz;
} BucketList;

typedef int (*BucketPointerCompare) (Bucket* , Bucket*);

int bucket_smaller_than(Bucket *a, Bucket *b){
	return a->sz < b->sz;
}

Bucket* bucket_crear(){
	Bucket *b = (Bucket*) malloc(sizeof(Bucket));
	b->sz = 0;
	b->list = NULL;
	return b;
}

Bucket* bucket_insertar(Bucket *b, char *key){
	char *w = (char*) malloc(sizeof(char) * WORDSIZE);
	strcpy(w, key);
	BucketNode* new = (BucketNode*) malloc(sizeof(BucketNode));
	new->w = w;
	new->next = b->list;
	b->list = new;
	b->sz++;
	
	return b;
}

BucketList* bucketlist_crear(uns sz){
	BucketList *BL = (BucketList*) malloc(sizeof(BucketList));
	BL->arr = (Bucket**) malloc(sz * sizeof(Bucket*));
	BL->sz = sz;
	return BL;
}

void bucketlist_destruir(BucketList* BL, int eliminaElementos){
	free(BL->arr);
	free(BL);
	if(eliminaElementos);
		//Completar
}

//Ordena de mayor a menor
BucketList* bucketlist_sort(BucketList *BL, BucketPointerCompare comp){
	if(BL->sz < 2)
		return BL;
		
	int m = BL->sz/2;
	BucketList *A1, *A2;
	A1 = bucketlist_crear(m);
	A2 = bucketlist_crear(BL->sz - m);
	
	int i,j,k;
	for(i = 0; i < m; ++i)
		A1->arr[i] = BL->arr[i];
		
	for(i = m; i < BL->sz; ++i)
		A2->arr[i - m] = BL->arr[i];
	
	//printf("pude crear los chiquitos con size %i y %i\n", A1->sz, A2->sz);
		
	A1 = bucketlist_sort(A1, comp);
	A2 = bucketlist_sort(A2, comp);
	
	//printf("pude ordenar los chiquitos\n");
	
	i = j = k = 0;
	//i en A1, j en A2, k en BL
	for(k=0; k < BL->sz ; ++k){
		//printf("voy a meter el elemento %i\n", k);
		
		//A1 esta vacio
		if(i >= A1->sz){
			//printf("A1 vacio\n");
			BL->arr[k] = A2->arr[j++];
			continue;
		}
		
		//A2 esta vacio
		if(j >= A2->sz){
			//printf("A2 vacio\n");
			BL->arr[k] = A1->arr[i++];
			continue;
		}
		
		//printf("voy a comparar A1 en el indice %i A2 en el indice %i\n", i, j);
		//Si el primer bucket no asignado en A2 es menor al de A1
		if(comp(A2->arr[j], A1->arr[i])){
			//printf("dio true\n");
			//Copiamos el de A1
			BL->arr[k] = A1->arr[i++];
		}
		else{
			//printf("dio false\n");
			//En caso contrario copiamos el de A2
			BL->arr[k] = A2->arr[j++];
		}
		//printf("meti el elemento %i\n", k);
	}
	
	//printf("pude mergear\n");
	
	bucketlist_destruir(A1, 0);
	bucketlist_destruir(A2, 0);
	
	return BL;
}

void bucket_destruir(Bucket *b){
	BucketNode *act, *sig;
	for(act = b->list; act != NULL; act = sig){
		sig = act->next;
		free(act);
	}
}

//FNV-1a de 32 bits
uns hash(int d, char *key){
	//Constantes obtenidas de http://isthe.com/chongo/tech/comp/fnv/
	uns prime = 16777619;
	uns basis = 2166136261;
	uns l = strlen(key);
	uns i;
	
	//Cada d genera un resultado diferente
	uns hash = d;
	if(d==0)
		hash = basis;
	
	for(i = 0 ; i < l ; ++i){
		hash = hash*prime;
		hash = hash^key[i];
	}
	
	return hash;
}

PerfectHash* ph_crear(char **D, uns dicSize){
	PerfectHash *ph = (PerfectHash*) malloc(sizeof(PerfectHash));
	ph->sz = dicSize;
	int *G = (int*) malloc(dicSize * sizeof(int));
	ph->G = G;
	
	int i,j;
	
	BucketList *BL = bucketlist_crear(dicSize);
	
	for(i = 0; i < BL->sz ; ++i)
		BL->arr[i] = bucket_crear();
	
	char **TH = (char**) malloc(dicSize * sizeof(char*));
	for(i = 0; i < dicSize; ++i)
		TH[i] = NULL;
	
	
	for(i = 0; i < dicSize; ++i){
		int idx = hash(0, D[i]) % dicSize;
		BL->arr[idx] =  bucket_insertar(BL->arr[idx], D[i]);
	}
	
	
	BL = bucketlist_sort(BL, bucket_smaller_than);
	
	for(i = 0; i < BL->sz ; ++i){
		Bucket *actual = BL->arr[i];
		//Los buckets de size 1 se procesan despues
		if(actual->sz <= 1)
			break;
			
		int *slots = (int*) malloc(sizeof(int) * actual->sz);
		for(j = 0; j < actual->sz ; j++)
			slots[j] = -1;
		
		BucketNode **wordList = (BucketNode**) malloc(actual->sz * sizeof(BucketNode*));
		BucketNode *nodo;
		int wordIdx = 0;
		for(nodo = actual->list; nodo !=  NULL; nodo = nodo->next)
			wordList[wordIdx++] = nodo;
		
		
		int d = 0;
		for(wordIdx = 0; wordIdx < actual->sz; wordIdx++){
			int slot = hash(d, wordList[wordIdx]->w) % dicSize;
			int pertenece = 0;
			
			for(j = 0; j < actual->sz; j++)
				if(slots[j] == slot)
					pertenece = 1;
			
			if(TH[slot]!=NULL || pertenece){
				d++;
				//Para que no se rompa con el ++
				wordIdx = -1;
			}
			else
				slots[wordIdx] = slot;
		}
		
		G[hash(0, wordList[0]->w) % dicSize] = d;
		for(j = 0; j < actual->sz; ++j){
			TH[ slots[j] ] = (char*) malloc(sizeof(char) * WORDSIZE);
			strcpy(TH[ slots[j] ], wordList[j]->w);
		}
		
		free(wordList);
		free(slots);
	}
	
	//En i tengo guardada la primera posicion de un bucket con tamaño 1
	int *libres = (int*) malloc(sizeof(int) * dicSize);
	int libresIdx = 0;
	int libresSz = 0;
	for(j = 0; j < dicSize; ++j)
		if(TH[j] == 0)
			libres[libresSz++] = j;

	//Recorremos los buckets restantes
	for(; i < BL->sz; ++i){
		Bucket *actual = BL->arr[i];
		//Si el bucket esta vacio, terminamos
		if(actual->sz == 0)
			break;
			
		char* word = actual->list->w;
		int lugar = libres[libresIdx++];
		//Restamos 1 para asegurarnos que queda negativo
		G[hash(0, word) % dicSize] = -lugar-1;
		TH[lugar] = (char*) malloc(sizeof(char) * WORDSIZE);
		strcpy(TH[lugar], word);
	}
	
	ph->HT = TH;
	
	return ph;
}

void ph_destruir(PerfectHash *ph){
	free(ph->G);
	int i;
	for(i = 0; i < ph->sz ; ++i)
		free(ph->HT[i]);
	free(ph->HT);
	free(ph);
}

int ph_buscar(PerfectHash *ph, char *key){
	char *res;
	int d = ph->G[hash(0,key) % ph->sz];
	if(d < 0)
		//Invertimos la triquiñuela
		res = ph->HT[-d-1];
	else
		res = ph->HT[hash(d, key) % ph->sz];
		
	if(strcmp(key, res) == 0)
		return 1;
	else
		return 0;
}

int main(){
	FILE *lemario;
	lemario = fopen("lemario.txt", "r");
	uns dicSize=0; //( ͡° ͜ʖ ͡°)
	int i;
	char w[WORDSIZE];
	
	while(fgets(w, WORDSIZE, lemario) != NULL)
		++dicSize;
	
	rewind(lemario);
	char **D = (char**) malloc(sizeof(char*) * dicSize);
	for(i = 0; fgets(w, WORDSIZE, lemario) != NULL; ++i) {
		//Eliminamos el \n
		w[strlen(w) - 1] = '\0';
		//Reservamos lugar y guardamos la palabra leida
		D[i] = (char*) malloc(sizeof(char) * WORDSIZE);
		strcpy(D[i], w);
	}
		
	PerfectHash *ph = ph_crear(D, dicSize);
	
	rewind(lemario);
	while(fgets(w, WORDSIZE, lemario) != NULL){
		w[strlen(w) - 1] = '\0';
		if(ph_buscar(ph, w) == 0)
			printf("%s\n", w);
	}
	
	while(scanf("%s", w)){
		if(ph_buscar(ph, w) == 0)
			printf("%s\n", w);
	}
	
	fclose(lemario);
	//No hace destruir el diccionario pues es parte de ph
	ph_destruir(ph);
	
	return 0;
}
