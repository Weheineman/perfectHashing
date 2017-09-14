#ifndef __PERFECTHASHING_H__
#define __PERFECTHASHING_H__

/**
 * Longitud máxima de una palabra.
 */
#define WORDLENGTH 30

typedef unsigned int uns;

/**
 * TH es una tabla hash donde cada casilla es un string.
 * La clave es igual al dato.
 * sz es el tamaño de TH.
 * G es el array donde se guardan los números con los cuales se deben hashear
 * los strings para evitar colisiones.
 */
typedef struct _PerfectHash{
	uns sz;
	char **TH;
	int *G;
} PerfectHash;

/**
 * Nodo de una lista enlazada simple. Contiene una palabra y un puntero al siguiente.
 */
typedef struct _BucketNode{
	char *w;
	struct _BucketNode *next;
} BucketNode;

/**
 * Lista enlazada simple de BucketNodes de tamaño variable sz.
 */
typedef struct _Bucket{
	BucketNode *list;
	uns sz;
} Bucket;

/**
 * BucketList es un arreglo de punteros a Bucket de tamaño fijo sz.
 */
typedef struct _BucketList{
	Bucket **arr;
	uns sz;	
} BucketList;

/**
 * Tipo de la función comparadora a ser usada en el sort.
 */
typedef int (*BucketPointerCompare) (Bucket* , Bucket*);

/**
 * Función comparadora de punteros a bucket.
 * Devuelve true si a tiene menor tamaño que b. En caso contrario devuelve false.
 */
int bucket_smaller_than(Bucket *a, Bucket *b);

/**
 * Crea un nuevo Bucket vacío.
 */
Bucket* bucket_crear();

/**
 * Inserta el string key al comienzo de la lista enlazada b.
 */
Bucket* bucket_insertar(Bucket *b, char *key);

/**
 * Destruye el Bucket.
 */
void bucket_destruir(Bucket *b);

/**
 * Crea una BucketList de tamaño fijo sz.
 * No inicializa el arreglo.
 */
BucketList* bucketlist_crear(uns sz);

/**
 * Destruye la BucketList.
 * Si eliminaElementos es false, sólo libera BL.
 * Si eliminaElementos es true, también libera los elementos del array.
 */
void bucketlist_destruir(BucketList* BL, int eliminaElementos);

/**
 * Dada una BucketList y una función comparadora, la ordena de mayor a menor.
 */
void bucketlist_sort(BucketList *BL, BucketPointerCompare comp);

/**
 * Función hash que hashea un par (número, string).
 * Para este trabajo se usó FNV-1a de 32 bits.
 * Fuente: http://isthe.com/chongo/tech/comp/fnv/
 */
uns hash(int d, char *key);

/**
 * Crea un PerfectHash a partir de un diccionario D de tamaño dicSize.
 */
PerfectHash* ph_crear(char **D, uns dicSize);

/**
 * Destruye el PerfectHash.
 */
void ph_destruir(PerfectHash *ph);

/**
 * Función de búsqueda en PerfectHash.
 * Devuelve true si key está en ph. En caso contrario devuelve false.
 */
int ph_buscar(PerfectHash *ph, char *key);

#endif
