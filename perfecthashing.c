#include "perfecthashing.h"
#include <stdlib.h>
#include <string.h>


/**
 * Función comparadora de punteros a bucket.
 * Devuelve true si a tiene menor tamaño que b. En caso contrario devuelve false.
 */
int bucket_smaller_than(Bucket *a, Bucket *b){
	return a->sz < b->sz;
}

/**
 * Crea un nuevo Bucket vacío.
 */
Bucket* bucket_crear(){
	//Reservamos memoria.
	Bucket *b = (Bucket*) malloc(sizeof(Bucket));
	//Inicializamos los campos de la estructura.
	b->sz = 0;
	b->list = NULL;
	return b;
}

/**
 * Inserta el string key al comienzo de la lista enlazada b.
 */
Bucket* bucket_insertar(Bucket *b, char *key){
	//Reservamos memoria para el string y lo guardamos.
	char *w = (char*) malloc(sizeof(char) * WORDLENGTH);
	strcpy(w, key);
	//Reservamos memoria para el nuevo nodo y lo inicializamos.
	BucketNode* new = (BucketNode*) malloc(sizeof(BucketNode));
	new->w = w;
	new->next = b->list;
	//El nuevo nodo ahora es el primero de la lista.
	b->list = new;
	//Aumentamos el tamaño del bucket.
	b->sz++;
	
	return b;
}

/**
 * Destruye el Bucket.
 */
void bucket_destruir(Bucket *b){
	BucketNode *act, *sig;
	
	//Liberamos los elementos de la lista.
	for(act = b->list; act != NULL; act = sig){
		sig = act->next;
		free(act);
	}
	
	//Liberamos el puntero a Bucket.
	free(b);
}

/**
 * Crea una BucketList de tamaño fijo sz.
 * No inicializa el arreglo.
 */
BucketList* bucketlist_crear(uns sz){
	//Reservamos espacio para la BucketList 
	BucketList *BL = (BucketList*) malloc(sizeof(BucketList));
	//Reservamos espacio para el array de tamaño sz y guardamos el valor sz.
	BL->arr = (Bucket**) malloc(sz * sizeof(Bucket*));
	BL->sz = sz;
	return BL;
}

/**
 * Destruye la BucketList.
 * Si eliminaElementos es false, sólo libera BL y el puntero arr.
 * Si eliminaElementos es true, también libera los elementos del array.
 */
void bucketlist_destruir(BucketList* BL, int eliminaElementos){
	if(eliminaElementos){
		int i;
		for(i = 0; i < BL->sz; ++i)
			bucket_destruir(BL->arr[i]);
	}
	free(BL->arr);
	free(BL);
}

/**
 * Dada una BucketList y una función comparadora, la ordena de mayor a menor.
 */
void bucketlist_sort(BucketList *BL, BucketPointerCompare comp){
	//Usaremos merge sort pues tiene peor caso O(n logn)
	
	//Caso base: Si la lista tiene un elemento o ninguno, está ordenada.
	if(BL->sz < 2)
		return;
		
	//Crearemos dos nuevos BucketList.
	//A1 será el rango [0, m) de BL.
	//A2 será el rango [m, BL->sz) de BL.
	int m = BL->sz/2;
	BucketList *A1, *A2;
	A1 = bucketlist_crear(m);
	A2 = bucketlist_crear(BL->sz - m);
	
	int i,j,k;
	//Copiamos la primera mitad en A1.
	for(i = 0; i < m; ++i)
		A1->arr[i] = BL->arr[i];
		
	//Copiamos la segunda mitad en A2.
	for(i = m; i < BL->sz; ++i)
		A2->arr[i - m] = BL->arr[i];
	
	//Ordenamos recursivamente A1 y A2.
	bucketlist_sort(A1, comp);
	bucketlist_sort(A2, comp);
	
	i = j = k = 0;
	//i en A1, j en A2, k en BL
	for(k=0; k < BL->sz ; ++k){
		//A1 esta vacio
		if(i >= A1->sz){
			//Entonces tomamos el próximo elemento de A2.
			BL->arr[k] = A2->arr[j++];
			continue;
		}
		
		//A2 esta vacio
		if(j >= A2->sz){
			//Entonces tomamos el próximo elemento de A1.
			BL->arr[k] = A1->arr[i++];
			continue;
		}
		
		//Si el primer bucket no asignado en A2 es menor al de A1
		if(comp(A2->arr[j], A1->arr[i]))
			//Copiamos el próximo elemento de A1
			BL->arr[k] = A1->arr[i++];
		
		else
			//En caso contrario copiamos el de A2
			BL->arr[k] = A2->arr[j++];
			
	}
	
	//Liberamos las BucketList A1 y A2.
	//eliminaElementos es 0, pues los punteros en A1->arr
	//y A2->arr están también en BL.
	bucketlist_destruir(A1, 0);
	bucketlist_destruir(A2, 0);
}

/**
 * Función hash que hashea un par (número, string).
 * Para este trabajo se usó FNV-1a de 32 bits.
 * Fuente: http://isthe.com/chongo/tech/comp/fnv/
 */
uns hash(int d, char *key){
	//Constantes obtenidas de http://isthe.com/chongo/tech/comp/fnv/
	uns prime = 16777619;
	uns basis = 2166136261;
	uns l = strlen(key);
	uns i;
	
	//La función depende tanto de key como de d.
	uns hash = d;
	//El caso d==0 es especial pues se usa para determinar los Buckets.
	if(d==0)
		hash = basis;
	
	//Hasheamos cada byte de key. Ignoramos el overflow pues usamos uns.
	for(i = 0 ; i < l ; ++i){
		hash = hash*prime;
		hash = hash^key[i];
	}
	
	return hash;
}

/**
 * Crea un PerfectHash a partir de un diccionario D de tamaño dicSize.
 */
PerfectHash* ph_crear(char **D, uns dicSize){
	//Reservamos lugar para el PerfectHash, guardamos dicSize.
	PerfectHash *ph = (PerfectHash*) malloc(sizeof(PerfectHash));
	ph->sz = dicSize;
	//Aquí guardaremos los enteros necesarios para el hashing.
	int *G = (int*) malloc(dicSize * sizeof(int));
	ph->G = G;
	
	int i,j;
	
	//Creamos la BucketList de tamaño dicSize. Inicializamos sus elementos con Buckets vacíos.
	BucketList *BL = bucketlist_crear(dicSize);
	for(i = 0; i < BL->sz ; ++i)
		BL->arr[i] = bucket_crear();
	
	//Creamos la tabla Hash. La inicializamos con NULL.
	char **TH = (char**) malloc(dicSize * sizeof(char*));
	for(i = 0; i < dicSize; ++i)
		TH[i] = NULL;
	ph->TH = TH;
	
	//Metemos el diccionario en la Bucketlist, dependiendo de su hash con 0.
	//A cada Bucket le corresponde un hash, en un mismo bucket se encuentran elementos
	//que colisionan.
	for(i = 0; i < dicSize; ++i){
		int idx = hash(0, D[i]) % dicSize;
		BL->arr[idx] =  bucket_insertar(BL->arr[idx], D[i]);
	}
	
	//Ordenamos la BucketList de mayor a menor.
	bucketlist_sort(BL, bucket_smaller_than);
	
	//Recorremos la BucketList.
	for(i = 0; i < BL->sz ; ++i){
		//actual es nuestro Bucket actual.
		Bucket *actual = BL->arr[i];
		//Los buckets de size 1 los procesamos después con una triquiñuela.
		if(actual->sz <= 1)
			break;
		
		//En slots guardamos los lugares tentativos de las keys de actual.
		int *slots = (int*) malloc(sizeof(int) * actual->sz);
		//Inicializamos en un valor que no puede ser índice de TH.
		for(j = 0; j < actual->sz ; j++)
			slots[j] = -1;
		
		//WordList es un array que contiene a los nodos de actual.
		BucketNode **wordList = (BucketNode**) malloc(actual->sz * sizeof(BucketNode*));
		BucketNode *nodo;
		int wordIdx = 0;
		for(nodo = actual->list; nodo !=  NULL; nodo = nodo->next)
			wordList[wordIdx++] = nodo;
		
		
		//Queremos encontrar un d para el cual no haya colisiones.
		int d = 0;
		//Recorremos la lista de nodos de actual.
		for(wordIdx = 0; wordIdx < actual->sz; wordIdx++){
			//slot es el lugar tentativo de la palabra que estamos procesando.
			int slot = hash(d, wordList[wordIdx]->w) % dicSize;
			int pertenece = 0;
			
			//Comprobamos que slot no esté en slots.
			for(j = 0; j < actual->sz; j++)
				if(slots[j] == slot)
					pertenece = 1;
			
			//Si está en slots o el lugar slot en TH está ocupado, hay colisión.
			if(TH[slot]!=NULL || pertenece){
				//Entonces hay que probar con el siguiente d.
				d++;
				//Seteamos wordIdx a 0 (se cancela con el ++ del for).
				//Esto descarta los valores en slots hasta ahora.
				wordIdx = -1;
			}
			else
				//Si no hay colisión, slot es un lugar posible.
				slots[wordIdx] = slot;
		}
		
		//Salir del for anterior implica que para d, el hash no tiene colisiones.
		//Guardamos d en G, en la posición que resulta del hash con 0.
		//Es indiferente qué key usar, pues todas en el Bucket hashean a lo mismo.
		G[hash(0, wordList[0]->w) % dicSize] = d;
		
		//Guardamos cada palabra en el lugar que acabamos de calcular.
		for(j = 0; j < actual->sz; ++j){
			//Reservamos lugar en la TH y copiamos el string.
			TH[ slots[j] ] = (char*) malloc(sizeof(char) * WORDLENGTH);
			strcpy(TH[ slots[j] ], wordList[j]->w);
		}
		
		//Liberamos los arrays auxiliares.
		free(wordList);
		free(slots);
	}
	
	//En i tengo guardada la primera posicion de un bucket con tamaño menor a 2.
	//libres es un array de int que contiene libresSz elementos.
	//Cada elemento es un índice libre de TH.
	int *libres = (int*) malloc(sizeof(int) * dicSize);
	int libresIdx = 0;
	int libresSz = 0;
	for(j = 0; j < dicSize; ++j)
		if(TH[j] == 0)
			libres[libresSz++] = j;

	//Recorremos los buckets restantes (tamaño a lo sumo 1).
	for(; i < BL->sz; ++i){
		//actual es el Bucket que estamos recorriendo.
		Bucket *actual = BL->arr[i];
		//Si el Bucket esta vacio, terminamos.
		if(actual->sz == 0)
			break;
		
		///Triquiñuela time!!!
		//word es la palabra actual. A word le asignamos un lugar libre (próximo elemento de libres).
		char* word = actual->list->w;
		//lugar es la posición que tendrá word en TH.
		int lugar = libres[libresIdx++];
		//Para saber que usamos la triquiñuela, en G guardamos un número negativo (restamos 1 por si lugar es 0).
		//En G guardaremos el número anterior al opuesto del lugar que le corresponde a word en TH.
		//Guardamos en la posición de G que resulta de hacer el hash con 0.
		G[hash(0, word) % dicSize] = -lugar-1;
		//Guardamos word en TH en la posición lugar.
		TH[lugar] = (char*) malloc(sizeof(char) * WORDLENGTH);
		strcpy(TH[lugar], word);
	}
	
	//Liberamos el array auxiliar.
	free(libres);
	//Destruimos BL. eliminarElementos vale 1 pues terminamos de usarlos.
	bucketlist_destruir(BL, 1);
	
	//Ya está! Devolvemos la estructura resultante.
	return ph;
}

/**
 * Destruye el PerfectHash.
 */
void ph_destruir(PerfectHash *ph){
	free(ph->G);
	int i;
	for(i = 0; i < ph->sz ; ++i)
		free(ph->TH[i]);
	free(ph->TH);
	free(ph);
}

/**
 * Función de búsqueda en PerfectHash.
 * Devuelve true si key está en ph. En caso contrario devuelve false.
 */
int ph_buscar(PerfectHash *ph, char *key){
	//res el la cadena que se encuentra en el lugar que le corresponde a key.
	char *res;
	
	//Recuperamos el d necesario para el hashing sin colisiones.
	int d = ph->G[hash(0,key) % ph->sz];
	
	//Si d<0, quiere decir que usamos la triquiñuela y d = -lugar-1
	if(d < 0)
		//-d-1 = lugar
		res = ph->TH[-d-1];
	else
		//Si no usamos la triquiñuela, hacemos el hash con el d obtenido.
		res = ph->TH[hash(d, key) % ph->sz];
		
	//Comparamos key con res.	
	if(strcmp(key, res) == 0)
		return 1;
	else
		return 0;
}
