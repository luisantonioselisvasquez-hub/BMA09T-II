#include <stdio.h>
#include <stdlib.h>

typedef struct NodoAVL {
    int dato;
    int altura;
    struct NodoAVL *izq;
    struct NodoAVL *der;
} NodoAVL;

int maximo(int a, int b) {
    return (a > b) ? a : b;
}

int altura(NodoAVL *n) {
    return (n == NULL) ? 0 : n->altura;
}

int factorEquilibrio(NodoAVL *n) {
    if (n == NULL) return 0;
    return altura(n->izq) - altura(n->der);
}

NodoAVL *crearNodoAVL(int dato) {
    NodoAVL *nuevo;

    nuevo = (NodoAVL *)malloc(sizeof(NodoAVL));
    if (nuevo == NULL) {
        printf("Error: no hay memoria suficiente.\n");
        exit(1);
    }

    nuevo->dato = dato;
    nuevo->altura = 1;
    nuevo->izq = NULL;
    nuevo->der = NULL;
    return nuevo;
}

NodoAVL *rotarDerechaAVL(NodoAVL *y) {
    NodoAVL *x;
    NodoAVL *t2;

    x = y->izq;
    t2 = x->der;

    x->der = y;
    y->izq = t2;

    y->altura = 1 + maximo(altura(y->izq), altura(y->der));
    x->altura = 1 + maximo(altura(x->izq), altura(x->der));

    return x;
}

NodoAVL *rotarIzquierdaAVL(NodoAVL *x) {
    NodoAVL *y;
    NodoAVL *t2;

    y = x->der;
    t2 = y->izq;

    y->izq = x;
    x->der = t2;

    x->altura = 1 + maximo(altura(x->izq), altura(x->der));
    y->altura = 1 + maximo(altura(y->izq), altura(y->der));

    return y;
}

NodoAVL *insertarAVL(NodoAVL *raiz, int dato) {
    int fe;

    if (raiz == NULL) return crearNodoAVL(dato);

    if (dato < raiz->dato) {
        raiz->izq = insertarAVL(raiz->izq, dato);
    } else if (dato > raiz->dato) {
        raiz->der = insertarAVL(raiz->der, dato);
    } else {
        return raiz; /* No se permiten datos repetidos. */
    }

    raiz->altura = 1 + maximo(altura(raiz->izq), altura(raiz->der));
    fe = factorEquilibrio(raiz);

    /* Caso LL: rotacion simple a la derecha. */
    if (fe > 1 && dato < raiz->izq->dato) {
        return rotarDerechaAVL(raiz);
    }

    /* Caso RR: rotacion simple a la izquierda. */
    if (fe < -1 && dato > raiz->der->dato) {
        return rotarIzquierdaAVL(raiz);
    }

    /* Caso LR: rotacion doble izquierda-derecha. */
    if (fe > 1 && dato > raiz->izq->dato) {
        raiz->izq = rotarIzquierdaAVL(raiz->izq);
        return rotarDerechaAVL(raiz);
    }

    /* Caso RL: rotacion doble derecha-izquierda. */
    if (fe < -1 && dato < raiz->der->dato) {
        raiz->der = rotarDerechaAVL(raiz->der);
        return rotarIzquierdaAVL(raiz);
    }

    return raiz;
}

NodoAVL *minimoAVL(NodoAVL *n) {
    while (n != NULL && n->izq != NULL) {
        n = n->izq;
    }
    return n;
}

NodoAVL *eliminarAVL(NodoAVL *raiz, int dato) {
    NodoAVL *hijo;
    NodoAVL *sucesor;
    int fe;

    if (raiz == NULL) return NULL;

    if (dato < raiz->dato) {
        raiz->izq = eliminarAVL(raiz->izq, dato);
    } else if (dato > raiz->dato) {
        raiz->der = eliminarAVL(raiz->der, dato);
    } else {
        if (raiz->izq == NULL || raiz->der == NULL) {
            if (raiz->izq != NULL) hijo = raiz->izq;
            else hijo = raiz->der;

            free(raiz);
            return hijo;
        } else {
            sucesor = minimoAVL(raiz->der);
            raiz->dato = sucesor->dato;
            raiz->der = eliminarAVL(raiz->der, sucesor->dato);
        }
    }

    raiz->altura = 1 + maximo(altura(raiz->izq), altura(raiz->der));
    fe = factorEquilibrio(raiz);

    /* Caso LL o hijo izquierdo equilibrado. */
    if (fe > 1 && factorEquilibrio(raiz->izq) >= 0) {
        return rotarDerechaAVL(raiz);
    }

    /* Caso LR. */
    if (fe > 1 && factorEquilibrio(raiz->izq) < 0) {
        raiz->izq = rotarIzquierdaAVL(raiz->izq);
        return rotarDerechaAVL(raiz);
    }

    /* Caso RR o hijo derecho equilibrado. */
    if (fe < -1 && factorEquilibrio(raiz->der) <= 0) {
        return rotarIzquierdaAVL(raiz);
    }

    /* Caso RL. */
    if (fe < -1 && factorEquilibrio(raiz->der) > 0) {
        raiz->der = rotarDerechaAVL(raiz->der);
        return rotarIzquierdaAVL(raiz);
    }

    return raiz;
}

NodoAVL *buscarAVL(NodoAVL *raiz, int dato) {
    while (raiz != NULL && raiz->dato != dato) {
        if (dato < raiz->dato) raiz = raiz->izq;
        else raiz = raiz->der;
    }
    return raiz;
}

void imprimirArbolAVL(NodoAVL *raiz, int nivel, char lado) {
    int i;

    if (raiz == NULL) return;

    /* Se imprime primero la rama derecha para mostrar el arbol de lado. */
    imprimirArbolAVL(raiz->der, nivel + 1, 'D');

    for (i = 0; i < nivel; i++) {
        printf("    ");
    }

    if (lado == 'D') printf("/--- ");
    else if (lado == 'I') printf("\\--- ");
    else printf("[RAIZ] ");

    printf("%d [FE=%d]\n", raiz->dato, factorEquilibrio(raiz));

    imprimirArbolAVL(raiz->izq, nivel + 1, 'I');
}

void imprimirInordenAVL(NodoAVL *raiz) {
    if (raiz != NULL) {
        imprimirInordenAVL(raiz->izq);
        printf("%d ", raiz->dato);
        imprimirInordenAVL(raiz->der);
    }
}

void liberarAVL(NodoAVL *raiz) {
    if (raiz != NULL) {
        liberarAVL(raiz->izq);
        liberarAVL(raiz->der);
        free(raiz);
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

void mostrarMenuAVL(void) {
    printf("\n====================================\n");
    printf("          MENU ARBOL AVL\n");
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
    NodoAVL *raiz;
    int opcion;
    int valor;

    raiz = NULL;

    do {
        mostrarMenuAVL();
        opcion = leerEntero("Seleccione una opcion: ");

        switch (opcion) {
            case 1:
                valor = leerEntero("Ingrese el valor que desea insertar: ");
                if (buscarAVL(raiz, valor) != NULL) {
                    printf("El valor %d ya existe. No se permiten repetidos.\n", valor);
                } else {
                    raiz = insertarAVL(raiz, valor);
                    printf("Elemento insertado correctamente.\n");
                }
                printf("\nArbol AVL actualizado:\n");
                if (raiz == NULL) printf("(vacio)\n");
                else imprimirArbolAVL(raiz, 0, 'R');
                break;

            case 2:
                valor = leerEntero("Ingrese el valor que desea buscar: ");
                if (buscarAVL(raiz, valor) != NULL) {
                    printf("El valor %d fue encontrado.\n", valor);
                } else {
                    printf("El valor %d no se encuentra en el arbol.\n", valor);
                }
                break;

            case 3:
                valor = leerEntero("Ingrese el valor que desea eliminar: ");
                if (buscarAVL(raiz, valor) == NULL) {
                    printf("El valor %d no existe en el arbol.\n", valor);
                } else {
                    raiz = eliminarAVL(raiz, valor);
                    printf("Elemento eliminado correctamente.\n");
                    printf("\nArbol AVL actualizado:\n");
                    if (raiz == NULL) printf("(vacio)\n");
                    else imprimirArbolAVL(raiz, 0, 'R');
                }
                break;

            case 4:
                printf("\nARBOL AVL (vista de lado)\n");
                printf("La rama derecha aparece arriba y la izquierda abajo.\n\n");
                if (raiz == NULL) printf("El arbol esta vacio.\n");
                else imprimirArbolAVL(raiz, 0, 'R');
                break;

            case 5:
                printf("Elementos en orden ascendente: ");
                if (raiz == NULL) printf("(vacio)");
                else imprimirInordenAVL(raiz);
                printf("\n");
                break;

            case 0:
                printf("Saliendo del programa AVL...\n");
                break;

            default:
                printf("Opcion incorrecta. Intente nuevamente.\n");
        }
    } while (opcion != 0);

    liberarAVL(raiz);
    return 0;
}
