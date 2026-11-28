#include <stdio.h>
#include <stdlib.h>

typedef enum { ROJO, NEGRO } Color;

typedef struct NodoRB {
    int dato;
    Color color;
    struct NodoRB *izq;
    struct NodoRB *der;
    struct NodoRB *padre;
} NodoRB;

NodoRB *NIL;
NodoRB *raiz;

void inicializarRB(void) {
    NIL = (NodoRB *)malloc(sizeof(NodoRB));
    if (NIL == NULL) {
        printf("Error: no hay memoria suficiente.\n");
        exit(1);
    }

    NIL->dato = 0;
    NIL->color = NEGRO;
    NIL->izq = NIL;
    NIL->der = NIL;
    NIL->padre = NIL;
    raiz = NIL;
}

NodoRB *crearNodoRB(int dato) {
    NodoRB *nuevo;

    nuevo = (NodoRB *)malloc(sizeof(NodoRB));
    if (nuevo == NULL) {
        printf("Error: no hay memoria suficiente.\n");
        exit(1);
    }

    nuevo->dato = dato;
    nuevo->color = ROJO;
    nuevo->izq = NIL;
    nuevo->der = NIL;
    nuevo->padre = NIL;
    return nuevo;
}

NodoRB *buscarRB(int dato) {
    NodoRB *actual;

    actual = raiz;
    while (actual != NIL && actual->dato != dato) {
        if (dato < actual->dato) actual = actual->izq;
        else actual = actual->der;
    }
    return actual;
}

void rotarIzquierdaRB(NodoRB *x) {
    NodoRB *y;

    y = x->der;
    x->der = y->izq;
    if (y->izq != NIL) y->izq->padre = x;

    y->padre = x->padre;
    if (x->padre == NIL) raiz = y;
    else if (x == x->padre->izq) x->padre->izq = y;
    else x->padre->der = y;

    y->izq = x;
    x->padre = y;
}

void rotarDerechaRB(NodoRB *y) {
    NodoRB *x;

    x = y->izq;
    y->izq = x->der;
    if (x->der != NIL) x->der->padre = y;

    x->padre = y->padre;
    if (y->padre == NIL) raiz = x;
    else if (y == y->padre->der) y->padre->der = x;
    else y->padre->izq = x;

    x->der = y;
    y->padre = x;
}

void corregirInsercionRB(NodoRB *z) {
    NodoRB *tio;

    while (z->padre->color == ROJO) {
        if (z->padre == z->padre->padre->izq) {
            tio = z->padre->padre->der;

            if (tio->color == ROJO) {
                z->padre->color = NEGRO;
                tio->color = NEGRO;
                z->padre->padre->color = ROJO;
                z = z->padre->padre;
            } else {
                if (z == z->padre->der) {
                    z = z->padre;
                    rotarIzquierdaRB(z);
                }
                z->padre->color = NEGRO;
                z->padre->padre->color = ROJO;
                rotarDerechaRB(z->padre->padre);
            }
        } else {
            tio = z->padre->padre->izq;

            if (tio->color == ROJO) {
                z->padre->color = NEGRO;
                tio->color = NEGRO;
                z->padre->padre->color = ROJO;
                z = z->padre->padre;
            } else {
                if (z == z->padre->izq) {
                    z = z->padre;
                    rotarDerechaRB(z);
                }
                z->padre->color = NEGRO;
                z->padre->padre->color = ROJO;
                rotarIzquierdaRB(z->padre->padre);
            }
        }
    }
    raiz->color = NEGRO;
}

void insertarRB(int dato) {
    NodoRB *z;
    NodoRB *y;
    NodoRB *x;

    z = crearNodoRB(dato);
    y = NIL;
    x = raiz;

    while (x != NIL) {
        y = x;
        if (dato < x->dato) x = x->izq;
        else if (dato > x->dato) x = x->der;
        else {
            free(z);
            return;
        }
    }

    z->padre = y;
    if (y == NIL) raiz = z;
    else if (dato < y->dato) y->izq = z;
    else y->der = z;

    corregirInsercionRB(z);
}

NodoRB *minimoRB(NodoRB *n) {
    while (n->izq != NIL) n = n->izq;
    return n;
}

void trasplantarRB(NodoRB *u, NodoRB *v) {
    if (u->padre == NIL) raiz = v;
    else if (u == u->padre->izq) u->padre->izq = v;
    else u->padre->der = v;
    v->padre = u->padre;
}

void corregirEliminacionRB(NodoRB *x) {
    NodoRB *hermano;

    while (x != raiz && x->color == NEGRO) {
        if (x == x->padre->izq) {
            hermano = x->padre->der;

            if (hermano->color == ROJO) {
                hermano->color = NEGRO;
                x->padre->color = ROJO;
                rotarIzquierdaRB(x->padre);
                hermano = x->padre->der;
            }

            if (hermano->izq->color == NEGRO && hermano->der->color == NEGRO) {
                /* NIL siempre debe permanecer negro. */
                if (hermano != NIL) hermano->color = ROJO;
                x = x->padre;
            } else {
                if (hermano->der->color == NEGRO) {
                    hermano->izq->color = NEGRO;
                    if (hermano != NIL) hermano->color = ROJO;
                    rotarDerechaRB(hermano);
                    hermano = x->padre->der;
                }
                hermano->color = x->padre->color;
                x->padre->color = NEGRO;
                hermano->der->color = NEGRO;
                rotarIzquierdaRB(x->padre);
                x = raiz;
            }
        } else {
            hermano = x->padre->izq;

            if (hermano->color == ROJO) {
                hermano->color = NEGRO;
                x->padre->color = ROJO;
                rotarDerechaRB(x->padre);
                hermano = x->padre->izq;
            }

            if (hermano->der->color == NEGRO && hermano->izq->color == NEGRO) {
                if (hermano != NIL) hermano->color = ROJO;
                x = x->padre;
            } else {
                if (hermano->izq->color == NEGRO) {
                    hermano->der->color = NEGRO;
                    if (hermano != NIL) hermano->color = ROJO;
                    rotarIzquierdaRB(hermano);
                    hermano = x->padre->izq;
                }
                hermano->color = x->padre->color;
                x->padre->color = NEGRO;
                hermano->izq->color = NEGRO;
                rotarDerechaRB(x->padre);
                x = raiz;
            }
        }
    }
    x->color = NEGRO;
}

void eliminarRB(int dato) {
    NodoRB *z;
    NodoRB *y;
    NodoRB *x;
    Color colorOriginal;

    z = buscarRB(dato);
    if (z == NIL) return;

    y = z;
    colorOriginal = y->color;

    if (z->izq == NIL) {
        x = z->der;
        trasplantarRB(z, z->der);
    } else if (z->der == NIL) {
        x = z->izq;
        trasplantarRB(z, z->izq);
    } else {
        y = minimoRB(z->der);
        colorOriginal = y->color;
        x = y->der;

        if (y->padre == z) {
            x->padre = y;
        } else {
            trasplantarRB(y, y->der);
            y->der = z->der;
            y->der->padre = y;
        }

        trasplantarRB(z, y);
        y->izq = z->izq;
        y->izq->padre = y;
        y->color = z->color;
    }

    free(z);
    if (colorOriginal == NEGRO) corregirEliminacionRB(x);
}

void imprimirArbolRB(NodoRB *n, int nivel, char lado) {
    int i;

    if (n == NIL) return;

    /* Se imprime primero la rama derecha para mostrar el arbol de lado. */
    imprimirArbolRB(n->der, nivel + 1, 'D');

    for (i = 0; i < nivel; i++) {
        printf("    ");
    }

    if (lado == 'D') printf("/--- ");
    else if (lado == 'I') printf("\\--- ");
    else printf("[RAIZ] ");

    printf("%d [%c]\n", n->dato, n->color == ROJO ? 'R' : 'N');

    imprimirArbolRB(n->izq, nivel + 1, 'I');
}

void imprimirInordenRB(NodoRB *n) {
    if (n != NIL) {
        imprimirInordenRB(n->izq);
        printf("%d(%c) ", n->dato, n->color == ROJO ? 'R' : 'N');
        imprimirInordenRB(n->der);
    }
}

void liberarRB(NodoRB *n) {
    if (n != NIL) {
        liberarRB(n->izq);
        liberarRB(n->der);
        free(n);
    }
}

int leerEntero(const char *mensaje) {
    int valor;
    int resultado;
    int caracter;

    while (1) {
        printf("%s", mensaje);
        resultado = scanf("%d", &valor);

        if (resultado == 1) {
            while ((caracter = getchar()) != '\n' && caracter != EOF) {
                /* Limpiar el resto de la linea. */
            }
            return valor;
        }

        if (resultado == EOF) {
            printf("\nFin de la entrada.\n");
            exit(0);
        }

        printf("Entrada no valida. Ingrese un numero entero.\n");
        while ((caracter = getchar()) != '\n' && caracter != EOF) {
            /* Limpiar la entrada incorrecta. */
        }
    }
}

void mostrarMenuRB(void) {
    printf("\n====================================\n");
    printf("       MENU ARBOL ROJO-NEGRO\n");
    printf("====================================\n");
    printf("1. Insertar un elemento\n");
    printf("2. Buscar un elemento\n");
    printf("3. Eliminar un elemento\n");
    printf("4. Mostrar arbol (grafico)\n");
    printf("5. Mostrar elementos ordenados\n");
    printf("0. Salir\n");
    printf("====================================\n");
}

int main(void) {
    int opcion;
    int valor;
    NodoRB *encontrado;

    inicializarRB();

    do {
        mostrarMenuRB();
        opcion = leerEntero("Seleccione una opcion: ");

        switch (opcion) {
            case 1:
                valor = leerEntero("Ingrese el valor que desea insertar: ");
                encontrado = buscarRB(valor);
                if (encontrado != NIL) {
                    printf("El valor %d ya existe. No se permiten repetidos.\n", valor);
                } else {
                    insertarRB(valor);
                    printf("Elemento insertado correctamente.\n");
                }
                printf("\nArbol rojo-negro actualizado:\n");
                if (raiz == NIL) printf("(vacio)\n");
                else imprimirArbolRB(raiz, 0, 'R');
                break;

            case 2:
                valor = leerEntero("Ingrese el valor que desea buscar: ");
                encontrado = buscarRB(valor);
                if (encontrado != NIL) {
                    printf("El valor %d fue encontrado. Color: %s.\n", valor,
                           encontrado->color == ROJO ? "rojo" : "negro");
                } else {
                    printf("El valor %d no se encuentra en el arbol.\n", valor);
                }
                break;

            case 3:
                valor = leerEntero("Ingrese el valor que desea eliminar: ");
                encontrado = buscarRB(valor);
                if (encontrado == NIL) {
                    printf("El valor %d no existe en el arbol.\n", valor);
                } else {
                    eliminarRB(valor);
                    printf("Elemento eliminado correctamente.\n");
                    printf("\nArbol rojo-negro actualizado:\n");
                    if (raiz == NIL) printf("(vacio)\n");
                    else imprimirArbolRB(raiz, 0, 'R');
                }
                break;

            case 4:
                printf("\nARBOL ROJO-NEGRO (vista de lado)\n");
                printf("R = rojo, N = negro. Las ramas derechas aparecen arriba.\n\n");
                if (raiz == NIL) printf("El arbol esta vacio.\n");
                else imprimirArbolRB(raiz, 0, 'R');
                break;

            case 5:
                printf("Elementos en orden ascendente (color): ");
                if (raiz == NIL) printf("(vacio)");
                else imprimirInordenRB(raiz);
                printf("\n");
                break;

            case 0:
                printf("Saliendo del programa rojo-negro...\n");
                break;

            default:
                printf("Opcion incorrecta. Intente nuevamente.\n");
        }
    } while (opcion != 0);

    liberarRB(raiz);
    free(NIL);
    return 0;
}
