#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Nodo {
    char clave[32];
    int valor;
    struct Nodo *siguiente;
} Nodo;

typedef struct {
    Nodo **cubetas;
    int m;
    int n;
} TablaHash;

unsigned long hash_cadena(const char *s) {
    unsigned long h = 0;
    int i;
    for (i = 0; s[i] != '\0'; i++)
        h = h * 31 + (unsigned char)s[i];
    return h;
}

TablaHash *crear_tabla(int m) {
    TablaHash *t = (TablaHash*) malloc(sizeof(TablaHash));
    if (t == NULL) return NULL;
    t->cubetas = (Nodo**) calloc(m, sizeof(Nodo*));
    if (t->cubetas == NULL) {
        free(t);
        return NULL;
    }
    t->m = m;
    t->n = 0;
    return t;
}

Nodo *buscar(const TablaHash *t, const char *clave) {
    int i = hash_cadena(clave) % t->m;
    Nodo *actual = t->cubetas[i];
    while (actual != NULL) {
        if (strcmp(actual->clave, clave) == 0) return actual;
        actual = actual->siguiente;
    }
    return NULL;
}

int redimensionar(TablaHash *t) {
    int i, j;
    int nuevo_m = t->m * 2 + 1;
    Nodo **nuevas;
    Nodo *actual, *sig;
    nuevas = (Nodo**) calloc(nuevo_m, sizeof(Nodo*));
    if (nuevas == NULL) return 0;
    for (i = 0; i < t->m; i++) {
        actual = t->cubetas[i];
        while (actual != NULL) {
            sig = actual->siguiente;
            j = hash_cadena(actual->clave) % nuevo_m;
            actual->siguiente = nuevas[j];
            nuevas[j] = actual;
            actual = sig;
        }
    }
    free(t->cubetas);
    t->cubetas = nuevas;
    t->m = nuevo_m;
    return 1;
}

int insertar(TablaHash *t, const char *clave, int valor) {
    Nodo *existente, *nuevo;
    int i;
    existente = buscar(t, clave);
    if (existente != NULL) {
        existente->valor = valor;
        return 1;
    }
    if ((double)(t->n + 1) / t->m > 0.75) {
        if (redimensionar(t))
            printf("(tabla redimensionada a %d cubetas)\n", t->m);
    }
    nuevo = (Nodo*) malloc(sizeof(Nodo));
    if (nuevo == NULL) return 0;
    strncpy(nuevo->clave, clave, 31);
    nuevo->clave[31] = '\0';
    nuevo->valor = valor;
    i = hash_cadena(clave) % t->m;
    nuevo->siguiente = t->cubetas[i];
    t->cubetas[i] = nuevo;
    t->n++;
    return 1;
}

int eliminar(TablaHash *t, const char *clave) {
    int i = hash_cadena(clave) % t->m;
    Nodo *actual = t->cubetas[i];
    Nodo *anterior = NULL;
    while (actual != NULL && strcmp(actual->clave, clave) != 0) {
        anterior = actual;
        actual = actual->siguiente;
    }
    if (actual == NULL) return 0;
    if (anterior == NULL) t->cubetas[i] = actual->siguiente;
    else anterior->siguiente = actual->siguiente;
    free(actual);
    t->n--;
    return 1;
}

void mostrar(const TablaHash *t) {
    int i;
    Nodo *actual;
    printf("m = %d cubetas, n = %d elementos, factor de carga = %.2f\n",
           t->m, t->n, (double)t->n / t->m);
    for (i = 0; i < t->m; i++) {
        printf("[%d] ", i);
        actual = t->cubetas[i];
        while (actual != NULL) {
            printf("(%s,%d) -> ", actual->clave, actual->valor);
            actual = actual->siguiente;
        }
        printf("NULL\n");
    }
}

void liberar_tabla(TablaHash *t) {
    int i;
    Nodo *actual, *temporal;
    for (i = 0; i < t->m; i++) {
        actual = t->cubetas[i];
        while (actual != NULL) {
            temporal = actual;
            actual = actual->siguiente;
            free(temporal);
        }
    }
    free(t->cubetas);
    free(t);
}

int main(void) {
    TablaHash *tabla;
    int opcion = 0, valor;
    char clave[32];
    Nodo *p;

    printf("=== PROGRAMA TABLA HASH - VERSION NUEVA ===\n");
    tabla = crear_tabla(5);
    if (tabla == NULL) {
        printf("No se pudo reservar memoria.\n");
        return 1;
    }
    do {
        printf("\n--- TABLA HASH (ENCADENAMIENTO) ---\n");
        printf("1. Insertar\n2. Buscar\n3. Eliminar\n4. Mostrar tabla\n5. Salir\n");
        printf("Opcion: ");
        if (scanf("%d", &opcion) != 1) {
            printf("Entrada no valida.\n");
            break;
        }
        if (opcion == 1) {
            printf("Clave (sin espacios): ");
            scanf("%31s", clave);
            printf("Valor entero: ");
            scanf("%d", &valor);
            if (insertar(tabla, clave, valor)) printf("Par insertado.\n");
            else printf("No se pudo reservar memoria.\n");
        } else if (opcion == 2) {
            printf("Clave a buscar: ");
            scanf("%31s", clave);
            p = buscar(tabla, clave);
            if (p != NULL) printf("Encontrado: %s -> %d\n", p->clave, p->valor);
            else printf("La clave no existe.\n");
        } else if (opcion == 3) {
            printf("Clave a eliminar: ");
            scanf("%31s", clave);
            if (eliminar(tabla, clave)) printf("Par eliminado.\n");
            else printf("La clave no existe.\n");
        } else if (opcion == 4) {
            mostrar(tabla);
        } else if (opcion == 5) {
            printf("Finalizando.\n");
        } else {
            printf("Opcion no valida.\n");
        }
    } while (opcion != 5);
    liberar_tabla(tabla);
    return 0;
}
