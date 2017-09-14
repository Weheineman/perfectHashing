#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define WORDLENGTH 30

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
	char *w = (char*) malloc(sizeof(char) * WORDLENGTH);
	strcpy(w, key);
	BucketNode* new = (BucketNode*) malloc(sizeof(BucketNode));
	new->w = w;
	new->next = b->list;
	b->list = new;
	b->sz++;
	
	return b;
}

void bucket_destruir(Bucket *b){
	BucketNode *act, *sig;
	for(act = b->list; act != NULL; act = sig){
		sig = act->next;
		free(act);
	}
}

BucketList* bucketlist_crear(uns sz){
	BucketList *BL = (BucketList*) malloc(sizeof(BucketList));
	BL->arr = (Bucket**) malloc(sz * sizeof(Bucket*));
	BL->sz = sz;
	return BL;
}

void bucketlist_destruir(BucketList* BL, int eliminaElementos){
	if(eliminaElementos){
		int i;
		for(i = 0; i < BL->sz; ++i)
			bucket_destruir(BL->arr[i]);
	}
	free(BL->arr);
	free(BL);
}

//Ordena de mayor a menor con merge sort.
BucketList* bucketlist_sort(BucketList *BL, BucketPointerCompare comp){
	//Caso base.
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
	
		
	A1 = bucketlist_sort(A1, comp);
	A2 = bucketlist_sort(A2, comp);
	
	i = j = k = 0;
	//i en A1, j en A2, k en BL
	for(k=0; k < BL->sz ; ++k){
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
		
		//Si el primer bucket no asignado en A2 es menor al de A1
		if(comp(A2->arr[j], A1->arr[i]))
			//Copiamos el de A1
			BL->arr[k] = A1->arr[i++];
		
		else
			//En caso contrario copiamos el de A2
			BL->arr[k] = A2->arr[j++];
			
	}
	
	bucketlist_destruir(A1, 0);
	bucketlist_destruir(A2, 0);
	
	return BL;
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
			TH[ slots[j] ] = (char*) malloc(sizeof(char) * WORDLENGTH);
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
		TH[lugar] = (char*) malloc(sizeof(char) * WORDLENGTH);
		strcpy(TH[lugar], word);
	}
	
	bucketlist_destruir(BL, 1);
	
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

//Intercambia idx con el siguiente, es decir, idx+1
void swap_ady(char *s, int idx){
	char aux = s[idx];
	s[idx] = s[idx+1];
	s[idx+1] = aux;
}

//Borra el n-esimo caracter del string
void del_char(char *s, int n){
	int i, idx=0;
	int l = strlen(s);
	for(i = 0; i < l; ++i)
		if(i != n)
			s[idx++] = s[i];
			
	s[idx] = '\0';
}

//Agrega el caracter c en la posición idx de la cadena s, desplazando los caracteres existentes a la derecha. 
void add_char(char *s, int idx, char c){
	int i;
	int l = strlen(s);
	char ant = c;
	char sig = c;
	
	for(i=idx; i <= l; ++i){
		ant = sig;
		sig = s[i];
		s[i] = ant;
	}
	
	s[i] = '\0';
}

//s1 = [0, idx) s2 = [idx, l)
void str_split(char *s, int idx, char *s1, char *s2){
	int i;
	int l = strlen(s);
	s1 = strncpy(s1, s, idx);
	s1[idx] = '\0';
	
	for(i = idx; i < l; ++i)
		s2[i - idx] = s[i];
		
	s2[i-idx] = '\0';
}

int main(){
	FILE *lemario, *texto, *output;
	lemario = fopen("lemario.txt", "r");
	texto = fopen("texto.txt", "r");
	output = fopen("output.txt", "w");
	uns dicSize=0; //( ͡° ͜ʖ ͡°)
	int i,j;
	char w[WORDLENGTH];
	
	while(fgets(w, WORDLENGTH, lemario) != NULL)
		++dicSize;
	
	rewind(lemario);
	char **D = (char**) malloc(sizeof(char*) * dicSize);
	for(i = 0; fgets(w, WORDLENGTH, lemario) != NULL; ++i) {
		//Eliminamos el \n
		w[strlen(w) - 1] = '\0';
		//Reservamos lugar y guardamos la palabra leida
		D[i] = (char*) malloc(sizeof(char) * WORDLENGTH);
		strcpy(D[i], w);
	}
		
	PerfectHash *ph = ph_crear(D, dicSize);
	
	
	int notEOF = 1;
	while(notEOF){
		char c = 'a';
		for(i = 0; ;){
			if(fscanf(texto, "%c", &c) == EOF){
				notEOF = 0;
				break;
			}
			
			if(ispunct(c) || c == ' ' || c == '\n')
				break;
			w[i++] = tolower(c);
		}
		w[i] = '\0';
		if(strlen(w) && (!ph_buscar(ph, w))){
			fprintf(output, "La siguiente palabra es incorrecta: %s\n", w);
			fprintf(output, "Sugerencias:\n");
			
			char *dummy = malloc(sizeof(char) * WORDLENGTH);
			strcpy(dummy, w);
			int l = strlen(w);
			char *minus = "abcdefghijklmnopqrstuvwxyz";
			int m = strlen(minus);
			
			//Reemplazar una letra por otra
			for(i = 0; i < l; ++i){
				strcpy(dummy, w);
				for(j = 0; j < m; ++j){
					dummy[i] = minus[j];
					if(ph_buscar(ph, dummy))
						fprintf(output, "%s\n", dummy);
				}
			}
			
			//Intercambiar un par de caracteres adyacentes
			for(i = 0; i < l-1; ++i){
				strcpy(dummy, w);
				swap_ady(dummy, i);
				if(ph_buscar(ph, dummy))
					fprintf(output, "%s\n", dummy);
			}
			
			//Eliminar un caracter de la palabra
			for(i = 0; i < l; ++i){
				strcpy(dummy, w);
				del_char(dummy, i);
				if(ph_buscar(ph, dummy))
					fprintf(output, "%s\n", dummy);
			}
			
			//Insertar un caracter entre un par de caracteres
			for(i = 0; i < l; ++i){
				for(j = 0; j < m; ++j){
					strcpy(dummy, w);
					add_char(dummy, i, minus[j]);
					if(ph_buscar(ph, dummy))
						fprintf(output, "%s\n", dummy);
				}
			}
			
			
			//Separar la palabra en dos
			char *s1 = (char*) malloc(sizeof(char) * WORDLENGTH);
			char *s2 = (char*) malloc(sizeof(char) * WORDLENGTH);
			for(i = 0; i < l; ++i){
				strcpy(dummy, w);
				str_split(dummy, i, s1, s2);
				if(ph_buscar(ph, s1) && ph_buscar(ph, s2))
					fprintf(output, "%s %s\n", s1, s2);
			}
			
			fprintf(output, "\n");
		}	
	}
	
	
	for(i = 0; i < dicSize; ++i)
		free(D[i]);
	free(D);
	fclose(lemario);
	ph_destruir(ph);
	
	return 0;
}
