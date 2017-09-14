#include "perfecthashing.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * Toma un string s y un índice idx.
 * Intercambia el caracter en la posición idx con el siguiente, es decir, idx+1.
 */
void swap_ady(char *s, int idx){
	char aux = s[idx];
	s[idx] = s[idx+1];
	s[idx+1] = aux;
}

/**
 * Toma un string s y un índice idx.
 * Elimina el caracter en la posición idx.
 */
 void del_char(char *s, int idx){
	int i, j=0;
	int l = strlen(s);
	for(i = 0; i < l; ++i)
		//Si el caracter no está en la posición idx, lo copiamos.
		if(i != idx)
			s[j++] = s[i];
	
	//Colocamos el terminador en el próximo lugar de s.
	s[j] = '\0';
}

/**
 * Toma un string s, un índice idx y un caracter c.
 * Agrega el caracter c en la posición idx de la cadena s, desplazando los caracteres existentes a la derecha.
 */ 
void add_char(char *s, int idx, char c){
	int i;
	int l = strlen(s);
	//ant es el caracter del lugar anterior.
	//sig es el caracter que debemos guardar en el lugar siguiente.
	char ant = c;
	char sig = c;
	
	//Desplazamos todo hacia la derecha a partir de idx.
	for(i=idx; i <= l; ++i){
		ant = sig;
		sig = s[i];
		s[i] = ant;
	}
	
	//Colocamos el terminador en el próximo lugar de s.
	s[i] = '\0';
}

/**
 * Toma un string s, un índice idx y dos strings s1, s2.
 * s1 será el rango [0,idx) de s
 * s2 será el rango [0,l) de s, siendo l la longitud de s.
 */
void str_split(char *s, int idx, char *s1, char *s2){
	int i;
	int l = strlen(s);
	//Copiamos los primeros idx caracteres en s1.
	s1 = strncpy(s1, s, idx);
	
	//Copiamos los caracteres restantes en s2.
	for(i = idx; i < l; ++i)
		s2[i - idx] = s[i];
		
	//Ponemos los terminadores.
	s1[idx] = '\0';
	s2[l-idx] = '\0';
}

/**
 * Toma una palabra w normalizada (todas letras minus), un PerfectHash ph y un puntero a archivo output.
 * Si w está en la tabla de ph, no hace nada.
 * Si w no está (es incorrecta), lo escribe en out y sugiere
 * palabras correctas similares (si existen).
 */
void correct(char *w, PerfectHash *ph, FILE *output){
	int i,j;
	
	//Si w es una palabra de longitud 0 o está en la tabla de ph, es correcta.
	if(strlen(w)==0 || ph_buscar(ph, w))
		return;

	//En caso contrario, es incorrecta.
	fprintf(output, "La siguiente palabra es incorrecta: %s\n", w);
	fprintf(output, "Sugerencias:\n");
	
	//dummy es una copia modificable de w.
	char *dummy = malloc(sizeof(char) * WORDLENGTH);
	strcpy(dummy, w);
	int l = strlen(w);
	//minus es la lista de letras minúsculas. Tiene tamaño m.
	char *minus = "abcdefghijklmnopqrstuvwxyz";
	int m = strlen(minus);
	
	//Reemplazar una letra por otra
	for(i = 0; i < l; ++i){
		//Recupero w.
		strcpy(dummy, w);
		//Pruebo todas las letras en la posición i.
		for(j = 0; j < m; ++j){
			dummy[i] = minus[j];
			if(ph_buscar(ph, dummy))
				fprintf(output, "%s\n", dummy);
		}
	}
	
	//Intercambiar un par de caracteres adyacentes
	//Recorro hasta l-1, pues intercambio con el caracter siguiente.
	for(i = 0; i < l-1; ++i){
		//Recupero w, le aplico el intercambio y compruebo si está en ph.
		strcpy(dummy, w);
		swap_ady(dummy, i);
		if(ph_buscar(ph, dummy))
			fprintf(output, "%s\n", dummy);
	}
	
	//Eliminar un caracter de la palabra
	for(i = 0; i < l; ++i){
		//Recupero w, le quito el caracter de la posición i y compruebo si está en ph.
		strcpy(dummy, w);
		del_char(dummy, i);
		if(ph_buscar(ph, dummy))
			fprintf(output, "%s\n", dummy);
	}
	
	//Insertar un caracter entre un par de caracteres
	for(i = 0; i < l; ++i){
		//Pruebo todos los caracteres
		for(j = 0; j < m; ++j){
			//Recupero w, le agrego el caracter en la posición i y compruebo si está en ph.
			strcpy(dummy, w);
			add_char(dummy, i, minus[j]);
			if(ph_buscar(ph, dummy))
				fprintf(output, "%s\n", dummy);
		}
	}
	
	
	//Separar la palabra en dos
	//Reservo memoria para s1 y s2.
	char *s1 = (char*) malloc(sizeof(char) * WORDLENGTH);
	char *s2 = (char*) malloc(sizeof(char) * WORDLENGTH);
	for(i = 0; i < l; ++i){
		//Recupero w, la divido en la posición i y compruebo si s1 y s2 están en ph.
		strcpy(dummy, w);
		str_split(dummy, i, s1, s2);
		if(ph_buscar(ph, s1) && ph_buscar(ph, s2))
			fprintf(output, "%s %s\n", s1, s2);
	}
	
	//Terminamos de corregir, dejo una línea de separación.
	fprintf(output, "\n");

	
	//Libero variables auxiliares.
	free(s1);
	free(s2);	
	free(dummy);	
}

/**
 * Toma un puntero a archivo (con formato de lemario) y un puntero a uns.
 * Retorna un diccionario D con todas las palabras del archivo.
 * Guarda en dicSize el tamaño de D.
 */
char** dic_crear(FILE *lemario, uns *dicSize){
	int i;
	*dicSize = 0;
	char w[WORDLENGTH];
	
	//Contamos la cantidad de palabras.
	while(fgets(w, WORDLENGTH, lemario) != NULL)
		++(*dicSize);
	
	//Reservamos espacio.
	char **D = (char**) malloc(sizeof(char*) * (*dicSize));
	
	rewind(lemario);
	for(i = 0; fgets(w, WORDLENGTH, lemario) != NULL; ++i) {
		//Eliminamos el \n
		w[strlen(w) - 1] = '\0';
		//Reservamos lugar y guardamos la palabra leida.
		D[i] = (char*) malloc(sizeof(char) * WORDLENGTH);
		strcpy(D[i], w);
	}
	
	return D;
}

int main(int argc, char **argv){
	//Declaro los punteros a archivos y abro los archivos correspondientes.
	//El programa se llama como main archivoEntrada archivoSalida
	FILE *lemario, *texto, *output;
	lemario = fopen("lemario.txt", "r");
	texto = fopen(argv[1], "r");
	output = fopen(argv[2], "w");
	
	//Declaramos variables
	uns dicSize=0; //( ͡° ͜ʖ ͡°)
	int i,j;
	char c;
	char w[WORDLENGTH];
	
	//Generamos el diccionario y la estructura PerfectHash.
	char **D = dic_crear(lemario, &dicSize);
	PerfectHash *ph = ph_crear(D, dicSize);
	
	//Flag
	int notEOF = 1;
	while(notEOF){
		//Leemos una palabra
		for(i = 0; ;){
			//Si llegamos al EOF, apagamos el flag y terminamos de leer la palabra.
			if(fscanf(texto, "%c", &c) == EOF){
				notEOF = 0;
				break;
			}
			//Si el caracter es de puntuación o de espaciado, terminamos de leer la palabra.
			//Ignoramos dicho caracter.
			if(ispunct(c) || c == ' ' || c == '\n' || c == '\t')
				break;
				
			//En caso contrario guardamos el caracter pasado a minúscula.
			w[i++] = tolower(c);
		}
		
		//Colocamos el terminador.
		w[i] = '\0';
		
		//Evaluamos la palabra leída.
		correct(w, ph, output);
	}
	
	//Liberamos las entradas del diccionario y el puntero D.
	for(i = 0; i < dicSize; ++i)
		free(D[i]);
	free(D);
	
	//Cerramos los archivos abiertos
	fclose(lemario);
	fclose(texto);
	fclose(output);
	
	//Destruimos el PerfectHash.
	ph_destruir(ph);
	return 0;
}
