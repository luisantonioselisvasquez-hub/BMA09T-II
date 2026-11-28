#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_CLAVE 100
#define MAX_HIJOS 26

typedef struct NodoRadix {
    char *etiqueta;
    int es_fin_clave;
    int cantidad_hijos;
    struct NodoRadix **hijos;
} NodoRadix;

/* Duplica una cadena de caracteres reservando memoria dinamica */
char *duplicar_cadena(const char *origen) {
    size_t longitud = strlen(origen);
    char *copia = (char *)malloc(longitud + 1);
    if (copia == NULL) {
        fprintf(stderr, "Error al asignar memoria para una etiqueta.\n");
        exit(EXIT_FAILURE);
    }
    strcpy(copia, origen);
    return copia;
}

/* Crea e inicializa un nuevo nodo del arbol radix */
NodoRadix *crear_nodo_radix(const char *etiqueta, int es_fin_clave) {
    NodoRadix *nodo = (NodoRadix *)malloc(sizeof(NodoRadix));
    if (nodo == NULL) {
        fprintf(stderr, "Error al asignar memoria para un nodo radix.\n");
        exit(EXIT_FAILURE);
    }
    nodo->etiqueta = duplicar_cadena(etiqueta);
    nodo->es_fin_clave = es_fin_clave;
    nodo->cantidad_hijos = 0;
    nodo->hijos = NULL;
    return nodo;
}

/* Normaliza una cadena a minusculas solo con letras a-z */
void normalizar_cadena(const char *entrada, char *salida) {
    int i = 0;
    int j = 0;
    while (entrada[i] != '\0') {
        unsigned char c = (unsigned char)tolower((unsigned char)entrada[i]);
        if (c >= 'a' && c <= 'z') {
            salida[j++] = (char)c;
        }
        i++;
    }
    salida[j] = '\0';
}

/* Calcula cuantos caracteres iniciales coinciden entre dos cadenas */
int longitud_prefijo_comun(const char *a, const char *b) {
    int k = 0;
    while (a[k] != '\0' && b[k] != '\0' && a[k] == b[k]) {
        k++;
    }
    return k;
}

/* Agrega un hijo existente a un nodo padre */
void agregar_hijo(NodoRadix *padre, NodoRadix *hijo) {
    int nuevo_total = padre->cantidad_hijos + 1;
    NodoRadix **nueva_lista = (NodoRadix **)realloc(
        padre->hijos, (size_t)nuevo_total * sizeof(NodoRadix *)
    );
    if (nueva_lista == NULL) {
        fprintf(stderr, "Error al redimensionar la lista de hijos.\n");
        exit(EXIT_FAILURE);
    }
    padre->hijos = nueva_lista;
    padre->hijos[padre->cantidad_hijos] = hijo;
    padre->cantidad_hijos = nuevo_total;
}

/* Inserta una clave en el arbol radix comprimiendo y dividiendo aristas */
void insertar_radix(NodoRadix *actual, const char *clave) {
    int i;
    int pref;
    int len_etiqueta;
    int len_clave;
    NodoRadix *hijo;
    NodoRadix *intermedio;
    char resto_etiqueta[MAX_CLAVE];
    char resto_clave[MAX_CLAVE];

    if (clave[0] == '\0') {
        actual->es_fin_clave = 1;
        return;
    }

    for (i = 0; i < actual->cantidad_hijos; i++) {
        hijo = actual->hijos[i];
        pref = longitud_prefijo_comun(hijo->etiqueta, clave);

        if (pref > 0) {
            len_etiqueta = (int)strlen(hijo->etiqueta);
            len_clave = (int)strlen(clave);

            /* Caso 1: La etiqueta del hijo coincide por completo */
            if (pref == len_etiqueta) {
                if (pref == len_clave) {
                    hijo->es_fin_clave = 1;
                } else {
                    insertar_radix(hijo, clave + pref);
                }
                return;
            }

            /* Caso 2: Coincidencia parcial -> dividir la arista */
            strcpy(resto_etiqueta, hijo->etiqueta + pref);
            free(hijo->etiqueta);
            hijo->etiqueta = (char *)malloc((size_t)pref + 1);
            if (hijo->etiqueta == NULL) {
                fprintf(stderr, "Error al reasignar etiqueta dividida.\n");
                exit(EXIT_FAILURE);
            }
            strncpy(hijo->etiqueta, clave, (size_t)pref);
            hijo->etiqueta[pref] = '\0';

            intermedio = crear_nodo_radix(resto_etiqueta, hijo->es_fin_clave);
            intermedio->cantidad_hijos = hijo->cantidad_hijos;
            intermedio->hijos = hijo->hijos;

            hijo->hijos = NULL;
            hijo->cantidad_hijos = 0;

            if (pref == len_clave) {
                hijo->es_fin_clave = 1;
                agregar_hijo(hijo, intermedio);
            } else {
                hijo->es_fin_clave = 0;
                agregar_hijo(hijo, intermedio);
                strcpy(resto_clave, clave + pref);
                agregar_hijo(hijo, crear_nodo_radix(resto_clave, 1));
            }
            return;
        }
    }

    /* Caso 3: Ningun hijo comparte prefijo; crear nueva rama comprimida */
    agregar_hijo(actual, crear_nodo_radix(clave, 1));
}

/* Busca una clave exacta en el arbol radix */
int buscar_radix(NodoRadix *actual, const char *clave) {
    int i;
    int pref;
    int len_etiqueta;
    int len_clave;
    NodoRadix *hijo;

    if (clave[0] == '\0') {
        return actual->es_fin_clave;
    }

    for (i = 0; i < actual->cantidad_hijos; i++) {
        hijo = actual->hijos[i];
        pref = longitud_prefijo_comun(hijo->etiqueta, clave);
        len_etiqueta = (int)strlen(hijo->etiqueta);
        len_clave = (int)strlen(clave);

        if (pref == len_etiqueta) {
            if (pref == len_clave) {
                return hijo->es_fin_clave;
            }
            return buscar_radix(hijo, clave + pref);
        }
    }

    return 0;
}

/* Recorre el subarbol acumulando etiquetas para listar claves */
void recolectar_claves(NodoRadix *nodo, char *acumulado, int *contador) {
    int i;
    size_t longitud_previa = strlen(acumulado);
    strcat(acumulado, nodo->etiqueta);

    if (nodo->es_fin_clave) {
        printf("  - %s\n", acumulado);
        (*contador)++;
    }

    for (i = 0; i < nodo->cantidad_hijos; i++) {
        recolectar_claves(nodo->hijos[i], acumulado, contador);
    }

    acumulado[longitud_previa] = '\0';
}

/* Busca y muestra todas las claves que comienzan con un prefijo */
void buscar_prefijo_radix(NodoRadix *raiz, const char *prefijo) {
    int i;
    int pref;
    int len_etiqueta;
    int len_prefijo;
    int contador;
    NodoRadix *actual = raiz;
    NodoRadix *hijo;
    char base[MAX_CLAVE * 2];
    char resto[MAX_CLAVE];

    base[0] = '\0';
    strcpy(resto, prefijo);

    while (resto[0] != '\0') {
        int avanzado = 0;
        len_prefijo = (int)strlen(resto);

        for (i = 0; i < actual->cantidad_hijos; i++) {
            hijo = actual->hijos[i];
            len_etiqueta = (int)strlen(hijo->etiqueta);
            pref = longitud_prefijo_comun(hijo->etiqueta, resto);

            if (pref == len_etiqueta) {
                strcat(base, hijo->etiqueta);
                strcpy(resto, resto + pref);
                actual = hijo;
                avanzado = 1;
                break;
            } else if (pref == len_prefijo) {
                /* El prefijo termina dentro de una etiqueta comprimida */
                strcat(base, hijo->etiqueta);
                printf("Claves que inician con \"%s\":\n", prefijo);
                contador = 0;
                recolectar_claves_desde_rama(hijo, base, &contador);
                if (contador == 0) {
                    printf("  (Sin coincidencias)\n");
                }
                return;
            }
        }

        if (!avanzado) {
            printf("No hay claves que inicien con \"%s\".\n", prefijo);
            return;
        }
    }

    printf("Claves que inician con \"%s\":\n", prefijo);
    contador = 0;
    if (actual->es_fin_clave) {
        printf("  - %s\n", base);
        contador++;
    }
    for (i = 0; i < actual->cantidad_hijos; i++) {
        recolectar_claves(actual->hijos[i], base, &contador);
    }
    if (contador == 0) {
        printf("  (Sin coincidencias)\n");
    }
}

/* Auxiliar cuando el prefijo cae dentro de una etiqueta de rama */
void recolectar_claves_desde_rama(NodoRadix *nodo, const char *ruta_completa, int *contador) {
    int i;
    char buffer[MAX_CLAVE * 2];
    strcpy(buffer, ruta_completa);

    if (nodo->es_fin_clave) {
        printf("  - %s\n", buffer);
        (*contador)++;
    }

    for (i = 0; i < nodo->cantidad_hijos; i++) {
        char siguiente[MAX_CLAVE * 2];
        strcpy(siguiente, buffer);
        strcat(siguiente, nodo->hijos[i]->etiqueta);
        recolectar_claves_desde_rama(nodo->hijos[i], siguiente, contador);
    }
}

/* Muestra la estructura comprimida y cuenta el total de nodos */
void mostrar_estructura(NodoRadix *nodo, int nivel, int *total_nodos) {
    int i;
    int s;
    (*total_nodos)++;

    for (s = 0; s < nivel; s++) {
        printf("  ");
    }
    printf("[%s]%s\n",
           (nodo->etiqueta[0] == '\0') ? "RAIZ" : nodo->etiqueta,
           nodo->es_fin_clave ? " (fin)" : "");

    for (i = 0; i < nodo->cantidad_hijos; i++) {
        mostrar_estructura(nodo->hijos[i], nivel + 1, total_nodos);
    }
}

/* Libera toda la memoria asignada al arbol radix */
void liberar_radix(NodoRadix *nodo) {
    int i;
    if (nodo == NULL) {
        return;
    }
    for (i = 0; i < nodo->cantidad_hijos; i++) {
        liberar_radix(nodo->hijos[i]);
    }
    free(nodo->etiqueta);
    free(nodo->hijos);
    free(nodo);
}

int main(void) {
    int i;
    int opcion;
    int total_nodos;
    char entrada_cruda[MAX_CLAVE];
    char entrada_limpia[MAX_CLAVE];
    NodoRadix *raiz = crear_nodo_radix("", 0);

    const char *palabras_iniciales[] = {
        "romano", "romana", "romero", "rubi", "rubio", "ruta"
    };
    int total_inicial = (int)(sizeof(palabras_iniciales) / sizeof(palabras_iniciales[0]));

    for (i = 0; i < total_inicial; i++) {
        insertar_radix(raiz, palabras_iniciales[i]);
    }

    do {
        printf("\n=== MENU ARBOL RADIX (TRIE COMPRIMIDO) ===\n");
        printf("1. Insertar clave\n");
        printf("2. Buscar clave exacta\n");
        printf("3. Buscar por prefijo\n");
        printf("4. Mostrar estructura y total de nodos\n");
        printf("0. Salir\n");
        printf("Seleccione una opcion: ");
        if (scanf("%d", &opcion) != 1) {
            break;
        }

        if (opcion == 1) {
            printf("Ingrese la clave a insertar: ");
            if (scanf("%99s", entrada_cruda) == 1) {
                normalizar_cadena(entrada_cruda, entrada_limpia);
                if (entrada_limpia[0] != '\0') {
                    insertar_radix(raiz, entrada_limpia);
                    printf("Clave '%s' insertada y comprimida.\n", entrada_limpia);
                }
            }
        } else if (opcion == 2) {
            printf("Ingrese la clave exacta a buscar: ");
            if (scanf("%99s", entrada_cruda) == 1) {
                normalizar_cadena(entrada_cruda, entrada_limpia);
                printf("Resultado: %s\n",
                       buscar_radix(raiz, entrada_limpia) ? "Encontrada" : "No encontrada");
            }
        } else if (opcion == 3) {
            printf("Ingrese el prefijo a consultar: ");
            if (scanf("%99s", entrada_cruda) == 1) {
                normalizar_cadena(entrada_cruda, entrada_limpia);
                buscar_prefijo_radix(raiz, entrada_limpia);
            }
        } else if (opcion == 4) {
            total_nodos = 0;
            printf("\nEstructura del arbol radix:\n");
            mostrar_estructura(raiz, 0, &total_nodos);
            printf("Total de nodos en el arbol radix: %d\n", total_nodos);
        }
    } while (opcion != 0);

    liberar_radix(raiz);
    return 0;
}
