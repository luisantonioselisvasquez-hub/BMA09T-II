#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define TAM_ALFABETO 26
#define MAX_PALABRA 100

typedef struct NodoTrie {
    struct NodoTrie *hijos[TAM_ALFABETO];
    int es_fin_palabra;
} NodoTrie;

/* Crea e inicializa un nuevo nodo del trie */
NodoTrie *crear_nodo(void) {
	int i;
    NodoTrie *nodo = (NodoTrie *)malloc(sizeof(NodoTrie));
    if (nodo == NULL) {
        fprintf(stderr, "Error al asignar memoria para un nodo del trie.\n");
        exit(EXIT_FAILURE);
    }
    nodo->es_fin_palabra = 0;
    for (i = 0; i < TAM_ALFABETO; i++) {
        nodo->hijos[i] = NULL;
    }
    return nodo;
}

/* Convierte un caracter en minuscula al indice 0..25 */
int indice_caracter(char c) {
    unsigned char uc = (unsigned char)tolower((unsigned char)c);
    if (uc >= 'a' && uc <= 'z') {
        return (int)(uc - 'a');
    }
    return -1;
}

/* Inserta una palabra en el trie */
void insertar(NodoTrie *raiz, const char *palabra) {
    NodoTrie *actual = raiz;
    int i;
    for (i = 0; palabra[i] != '\0'; i++) {
        int idx = indice_caracter(palabra[i]);
        if (idx == -1) {
            continue;
        }
        if (actual->hijos[idx] == NULL) {
            actual->hijos[idx] = crear_nodo();
        }
        actual = actual->hijos[idx];
    }
    actual->es_fin_palabra = 1;
}

/* Busca una coincidencia exacta de la palabra */
int buscar_exacta(NodoTrie *raiz, const char *palabra) {
    NodoTrie *actual = raiz;
    int i;
    for (i = 0; palabra[i] != '\0'; i++) {
        int idx = indice_caracter(palabra[i]);
        if (idx == -1 || actual->hijos[idx] == NULL) {
            return 0;
        }
        actual = actual->hijos[idx];
    }
    return (actual != NULL && actual->es_fin_palabra) ? 1 : 0;
}

/* Verifica si el nodo tiene al menos un hijo no nulo */
int tiene_hijos(NodoTrie *nodo) {
	int i;
    for (i = 0; i < TAM_ALFABETO; i++) {
        if (nodo->hijos[i] != NULL) {
            return 1;
        }
    }
    return 0;
}

/* Elimina una palabra recursivamente sin afectar prefijos compartidos */
int eliminar_recursivo(NodoTrie *actual, const char *palabra, int profundidad) {
    if (actual == NULL) {
        return 0;
    }

    if (palabra[profundidad] == '\0') {
        if (!actual->es_fin_palabra) {
            return 0;
        }
        actual->es_fin_palabra = 0;
        return !tiene_hijos(actual);
    }

    int idx = indice_caracter(palabra[profundidad]);
    if (idx == -1 || actual->hijos[idx] == NULL) {
        return 0;
    }

    int debe_eliminar_hijo = eliminar_recursivo(actual->hijos[idx], palabra, profundidad + 1);

    if (debe_eliminar_hijo) {
        free(actual->hijos[idx]);
        actual->hijos[idx] = NULL;
        return (!actual->es_fin_palabra && !tiene_hijos(actual));
    }

    return 0;
}

void eliminar_palabra(NodoTrie *raiz, const char *palabra) {
    eliminar_recursivo(raiz, palabra, 0);
}

/* Recorre un subarbol acumulando letras para imprimir sugerencias */
void recolectar_sugerencias(NodoTrie *nodo, char *buffer, int nivel, int *contador) {
    if (nodo->es_fin_palabra) {
        buffer[nivel] = '\0';
        printf("  - %s\n", buffer);
        (*contador)++;
    }
	int i;
    for (i = 0; i < TAM_ALFABETO; i++) {
        if (nodo->hijos[i] != NULL) {
            buffer[nivel] = (char)('a' + i);
            recolectar_sugerencias(nodo->hijos[i], buffer, nivel + 1, contador);
        }
    }
}

/* Muestra todas las palabras que inician con el prefijo dado */
void autocompletar(NodoTrie *raiz, const char *prefijo) {
    NodoTrie *actual = raiz;
    char buffer[MAX_PALABRA];
    int nivel = 0;
	int i;
    for (i = 0; prefijo[i] != '\0'; i++) {
        int idx = indice_caracter(prefijo[i]);
        if (idx == -1 || actual->hijos[idx] == NULL) {
            printf("No se encontraron sugerencias para el prefijo \"%s\".\n", prefijo);
            return;
        }
        buffer[nivel++] = (char)('a' + idx);
        actual = actual->hijos[idx];
    }

    printf("Sugerencias para \"%s\":\n", prefijo);
    int contador = 0;
    recolectar_sugerencias(actual, buffer, nivel, &contador);
    if (contador == 0) {
        printf("  (Sin palabras completas bajo este prefijo)\n");
    }
}

/* Libera toda la memoria ocupada por el trie */
void liberar_trie(NodoTrie *nodo) {
    if (nodo == NULL) {
        return;
    }
    int i;
	for (i = 0; i < TAM_ALFABETO; i++) {
        liberar_trie(nodo->hijos[i]);
    }
    free(nodo);
}

int main(void) {
    int i;
    int opcion;
    char entrada[MAX_PALABRA];
    NodoTrie *raiz = crear_nodo();

    const char *diccionario[] = {
        "algoritmo", "algebra", "alfabeto", "arbol", "arreglo",
        "autocompletar", "autor", "busqueda", "cadena", "caracter"
    };
    int total_inicial = (int)(sizeof(diccionario) / sizeof(diccionario[0]));

    for (i = 0; i < total_inicial; i++) {
        insertar(raiz, diccionario[i]);
    }

    do {
        printf("\n=== MENU TRIE Y AUTOCOMPLETADO ===\n");
        printf("1. Insertar nueva palabra\n");
        printf("2. Buscar palabra exacta\n");
        printf("3. Autocompletar por prefijo\n");
        printf("4. Eliminar palabra\n");
        printf("0. Salir\n");
        printf("Seleccione una opcion: ");
        if (scanf("%d", &opcion) != 1) {
            break;
        }

        if (opcion == 1) {
            printf("Ingrese la palabra a insertar: ");
            if (scanf("%99s", entrada) == 1) {
                insertar(raiz, entrada);
                printf("Palabra '%s' insertada correctamente.\n", entrada);
            }
        } else if (opcion == 2) {
            printf("Ingrese la palabra a buscar: ");
            if (scanf("%99s", entrada) == 1) {
                printf("Resultado: %s\n", buscar_exacta(raiz, entrada) ? "Encontrada" : "No encontrada");
            }
        } else if (opcion == 3) {
            printf("Ingrese el prefijo: ");
            if (scanf("%99s", entrada) == 1) {
                autocompletar(raiz, entrada);
            }
        } else if (opcion == 4) {
            printf("Ingrese la palabra a eliminar: ");
            if (scanf("%99s", entrada) == 1) {
                if (buscar_exacta(raiz, entrada)) {
                    eliminar_palabra(raiz, entrada);
                    printf("Palabra '%s' eliminada.\n", entrada);
                } else {
                    printf("La palabra '%s' no existe en el trie.\n", entrada);
                }
            }
        }
    } while (opcion != 0);

    liberar_trie(raiz);
    return 0;
}
