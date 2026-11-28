#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PREFIJO 33
#define MAX_DESTINO 80

typedef struct NodoRuta {
    struct NodoRuta *hijos[2]; /* 0 para bit '0', 1 para bit '1' */
    int tiene_ruta;
    char destino[MAX_DESTINO];
} NodoRuta;

/* Crea e inicializa un nuevo nodo del trie binario de rutas */
NodoRuta *crear_nodo_ruta(void) {
    NodoRuta *nodo = (NodoRuta *)malloc(sizeof(NodoRuta));
    if (nodo == NULL) {
        fprintf(stderr, "Error al asignar memoria para un nodo de ruta.\n");
        exit(EXIT_FAILURE);
    }
    nodo->hijos[0] = NULL;
    nodo->hijos[1] = NULL;
    nodo->tiene_ruta = 0;
    nodo->destino[0] = '\0';
    return nodo;
}

/* Inserta un prefijo binario y su interfaz o destino asociado */
void registrar_ruta(NodoRuta *raiz, const char *prefijo_binario, const char *destino) {
    int i;
    int bit;
    NodoRuta *actual = raiz;

    for (i = 0; prefijo_binario[i] != '\0'; i++) {
        if (prefijo_binario[i] == '0') {
            bit = 0;
        } else if (prefijo_binario[i] == '1') {
            bit = 1;
        } else {
            continue;
        }

        if (actual->hijos[bit] == NULL) {
            actual->hijos[bit] = crear_nodo_ruta();
        }
        actual = actual->hijos[bit];
    }

    actual->tiene_ruta = 1;
    strncpy(actual->destino, destino, MAX_DESTINO - 1);
    actual->destino[MAX_DESTINO - 1] = '\0';
}

/* Busca la coincidencia de prefijo mas largo (Longest Prefix Match) */
void buscar_ruta_mas_especifica(NodoRuta *raiz, const char *direccion_binaria) {
    int i;
    int bit;
    int longitud_mejor = -1;
    NodoRuta *actual = raiz;
    char mejor_destino[MAX_DESTINO];
    char mejor_prefijo[MAX_PREFIJO];

    mejor_destino[0] = '\0';
    mejor_prefijo[0] = '\0';

    /* Evalua ruta por defecto (prefijo vacio) si existe */
    if (actual->tiene_ruta) {
        longitud_mejor = 0;
        strcpy(mejor_destino, actual->destino);
        strcpy(mejor_prefijo, "(Ruta por defecto)");
    }

    for (i = 0; direccion_binaria[i] != '\0'; i++) {
        if (direccion_binaria[i] == '0') {
            bit = 0;
        } else if (direccion_binaria[i] == '1') {
            bit = 1;
        } else {
            printf("Entrada invalida: utilice unicamente digitos binarios (0 y 1).\n");
            return;
        }

        if (actual->hijos[bit] == NULL) {
            break;
        }

        actual = actual->hijos[bit];

        if (actual->tiene_ruta) {
            longitud_mejor = i + 1;
            strcpy(mejor_destino, actual->destino);
            strncpy(mejor_prefijo, direccion_binaria, (size_t)(i + 1));
            mejor_prefijo[i + 1] = '\0';
        }
    }

    if (longitud_mejor >= 0) {
        printf("Direccion consultada : %s\n", direccion_binaria);
        printf("Prefijo coincidente  : %s (%d bits)\n", mejor_prefijo, longitud_mejor);
        printf("Interfaz / Destino   : %s\n", mejor_destino);
    } else {
        printf("No se encontro ninguna ruta aplicable para %s.\n", direccion_binaria);
    }
}

/* Muestra todas las rutas registradas en el trie */
void listar_rutas(NodoRuta *nodo, char *buffer, int nivel) {
    if (nodo == NULL) {
        return;
    }

    if (nodo->tiene_ruta) {
        buffer[nivel] = '\0';
        printf("  Prefijo %-12s -> %s\n",
               (nivel == 0) ? "default" : buffer,
               nodo->destino);
    }

    if (nodo->hijos[0] != NULL) {
        buffer[nivel] = '0';
        listar_rutas(nodo->hijos[0], buffer, nivel + 1);
    }
    if (nodo->hijos[1] != NULL) {
        buffer[nivel] = '1';
        listar_rutas(nodo->hijos[1], buffer, nivel + 1);
    }
}

/* Libera toda la memoria ocupada por el trie de rutas */
void liberar_rutas(NodoRuta *nodo) {
    if (nodo == NULL) {
        return;
    }
    liberar_rutas(nodo->hijos[0]);
    liberar_rutas(nodo->hijos[1]);
    free(nodo);
}

int main(void) {
    int opcion;
    char prefijo[MAX_PREFIJO];
    char destino[MAX_DESTINO];
    char buffer[MAX_PREFIJO];
    NodoRuta *raiz = crear_nodo_ruta();

    /* Tabla inicial de prefijos de red */
    registrar_ruta(raiz, "", "Puerta de enlace general (eth0)");
    registrar_ruta(raiz, "110", "Red Corporativa Central (eth1)");
    registrar_ruta(raiz, "11001", "Subred Servidores Internos (eth2)");
    registrar_ruta(raiz, "1100101", "Servidor de Base de Datos (eth3)");
    registrar_ruta(raiz, "1010", "Red de Invitados (wlan0)");

    do {
        printf("\n=== MENU TRIE DE ENRUTAMIENTO POR PREFIJOS ===\n");
        printf("1. Consultar direccion binaria (Longest Prefix Match)\n");
        printf("2. Registrar nuevo prefijo de ruta\n");
        printf("3. Mostrar tabla completa de prefijos\n");
        printf("0. Salir\n");
        printf("Seleccione una opcion: ");
        if (scanf("%d", &opcion) != 1) {
            break;
        }

        if (opcion == 1) {
            printf("Ingrese la direccion binaria a enrutar (ejemplo: 1100101110): ");
            if (scanf("%32s", prefijo) == 1) {
                buscar_ruta_mas_especifica(raiz, prefijo);
            }
        } else if (opcion == 2) {
            printf("Ingrese el prefijo binario (ejemplo: 101011): ");
            if (scanf("%32s", prefijo) == 1) {
                getchar(); /* Limpia salto de linea pendiente */
                printf("Ingrese el nombre de la interfaz o destino: ");
                if (fgets(destino, MAX_DESTINO, stdin) != NULL) {
                    size_t len = strlen(destino);
                    if (len > 0 && destino[len - 1] == '\n') {
                        destino[len - 1] = '\0';
                    }
                    registrar_ruta(raiz, prefijo, destino);
                    printf("Ruta registrada correctamente.\n");
                }
            }
        } else if (opcion == 3) {
            printf("\nTabla de prefijos activos:\n");
            listar_rutas(raiz, buffer, 0);
        }
    } while (opcion != 0);

    liberar_rutas(raiz);
    return 0;
}
