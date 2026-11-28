/*
 * CONJUNTOS DISJUNTOS (UNION-FIND)
 * Curso: Algoritmos y Estructura de Datos 1
 * Grupo8
 *
 * PARTE A: arreglos + estructura + punteros (padre[], rango[], tamanio[])
 * PARTE B: arbol con nodos y punteros (struct Nodo *padre)
 * PARTE C: comparacion de alturas (sin / con optimizaciones)
 * PARTE D: aplicaciones (conectividad, agrupamiento, ciclos en grafos)
 */

#include <stdio.h>
#include <stdlib.h>

/* ================================================================
 * PARTE A: REPRESENTACION CON ARREGLOS
 * ================================================================ */

typedef struct {
    int *padre;     /* padre[i]: padre de i (la raiz se apunta a si misma) */
    int *rango;     /* cota superior de la altura del arbol (en raices)    */
    int *tamanio;   /* cantidad de elementos del conjunto (en raices)      */
    int n;          /* cantidad total de elementos                         */
} ConjuntosDisjuntos;

/* make_set para todos los elementos: cada uno forma su propio conjunto */
ConjuntosDisjuntos *crear(int n) {
    int i;
    ConjuntosDisjuntos *cd = (ConjuntosDisjuntos *)malloc(sizeof(ConjuntosDisjuntos));
    if (cd == NULL) return NULL;
    cd->n = n;
    cd->padre   = (int *)malloc(n * sizeof(int));
    cd->rango   = (int *)malloc(n * sizeof(int));
    cd->tamanio = (int *)malloc(n * sizeof(int));
    for (i = 0; i < n; i++) {
        cd->padre[i]   = i;   /* es su propio representante */
        cd->rango[i]   = 0;
        cd->tamanio[i] = 1;
    }
    return cd;
}

void destruir(ConjuntosDisjuntos *cd) {
    if (cd == NULL) return;
    free(cd->padre);
    free(cd->rango);
    free(cd->tamanio);
    free(cd);
}

/* ---------- FIND ---------- */

/* Version simple: sube por los padres hasta la raiz */
int find_simple(ConjuntosDisjuntos *cd, int x) {
    while (cd->padre[x] != x)
        x = cd->padre[x];
    return x;
}

/* Version con COMPRESION DE CAMINOS: todos los nodos visitados
   quedan apuntando directamente a la raiz */
int find_comp(ConjuntosDisjuntos *cd, int x) {
    if (cd->padre[x] != x)
        cd->padre[x] = find_comp(cd, cd->padre[x]);
    return cd->padre[x];
}

/* ---------- UNION ---------- */

/* Version ingenua: siempre cuelga la raiz de a bajo la raiz de b */
void union_simple(ConjuntosDisjuntos *cd, int a, int b) {
    int ra = find_simple(cd, a);
    int rb = find_simple(cd, b);
    if (ra != rb)
        cd->padre[ra] = rb;
}

/* Union por RANGO. Devuelve 1 si unio, 0 si ya estaban juntos */
int union_rango(ConjuntosDisjuntos *cd, int a, int b) {
    int ra = find_comp(cd, a);
    int rb = find_comp(cd, b);
    if (ra == rb) return 0;
    if (cd->rango[ra] < cd->rango[rb])
        cd->padre[ra] = rb;
    else if (cd->rango[ra] > cd->rango[rb])
        cd->padre[rb] = ra;
    else {
        cd->padre[rb] = ra;
        cd->rango[ra]++;      /* solo crece si hay empate */
    }
    return 1;
}

/* Union por TAMANIO. Devuelve 1 si unio, 0 si ya estaban juntos */
int union_tamanio(ConjuntosDisjuntos *cd, int a, int b) {
    int ra = find_comp(cd, a);
    int rb = find_comp(cd, b);
    if (ra == rb) return 0;
    if (cd->tamanio[ra] < cd->tamanio[rb]) {
        cd->padre[ra] = rb;
        cd->tamanio[rb] += cd->tamanio[ra];
    } else {
        cd->padre[rb] = ra;
        cd->tamanio[ra] += cd->tamanio[rb];
    }
    return 1;
}

/* ---------- Utilidades ---------- */

int mismo_conjunto(ConjuntosDisjuntos *cd, int a, int b) {
    return find_comp(cd, a) == find_comp(cd, b);
}

/* Cantidad de pasos desde x hasta su raiz (profundidad de x) */
int profundidad(ConjuntosDisjuntos *cd, int x) {
    int pasos = 0;
    while (cd->padre[x] != x) {
        x = cd->padre[x];
        pasos++;
    }
    return pasos;
}

/* Altura del bosque: mayor profundidad entre todos los elementos */
int altura_maxima(ConjuntosDisjuntos *cd) {
    int i, h, max = 0;
    for (i = 0; i < cd->n; i++) {
        h = profundidad(cd, i);
        if (h > max) max = h;
    }
    return max;
}

void imprimir(ConjuntosDisjuntos *cd) {
    int i;
    printf("  indice : ");
    for (i = 0; i < cd->n; i++) printf("%3d", i);
    printf("\n  padre  : ");
    for (i = 0; i < cd->n; i++) printf("%3d", cd->padre[i]);
    printf("\n  rango  : ");
    for (i = 0; i < cd->n; i++) printf("%3d", cd->rango[i]);
    printf("\n  tamanio: ");
    for (i = 0; i < cd->n; i++) printf("%3d", cd->tamanio[i]);
    printf("\n");
}

/* Muestra los conjuntos: elementos agrupados por representante */
void imprimir_conjuntos(ConjuntosDisjuntos *cd) {
    int r, i, primero;
    for (r = 0; r < cd->n; r++) {
        if (find_simple(cd, r) == r) {          /* r es una raiz */
            printf("  { ");
            primero = 1;
            for (i = 0; i < cd->n; i++) {
                if (find_simple(cd, i) == r) {
                    if (!primero) printf(", ");
                    printf("%d", i);
                    primero = 0;
                }
            }
            printf(" }  representante = %d\n", r);
        }
    }
}

/* ================================================================
 * PARTE B: REPRESENTACION CON ARBOL DE NODOS Y PUNTEROS
 * ================================================================ */

typedef struct Nodo {
    int dato;
    int rango;
    struct Nodo *padre;     /* la raiz apunta a si misma */
} Nodo;

Nodo *crear_nodo(int dato) {            /* make_set(dato) */
    Nodo *p = (Nodo *)malloc(sizeof(Nodo));
    if (p == NULL) return NULL;
    p->dato  = dato;
    p->rango = 0;
    p->padre = p;
    return p;
}

Nodo *find_nodo(Nodo *x) {              /* con compresion de caminos */
    if (x->padre != x)
        x->padre = find_nodo(x->padre);
    return x->padre;
}

int union_nodo(Nodo *a, Nodo *b) {      /* union por rango */
    Nodo *ra = find_nodo(a);
    Nodo *rb = find_nodo(b);
    if (ra == rb) return 0;
    if (ra->rango < rb->rango)
        ra->padre = rb;
    else if (ra->rango > rb->rango)
        rb->padre = ra;
    else {
        rb->padre = ra;
        ra->rango++;
    }
    return 1;
}

/* ================================================================
 * PARTE D (estructuras auxiliares): aristas de un grafo
 * ================================================================ */

typedef struct {
    int u, v;
} Arista;

/* Detecta ciclo en un grafo no dirigido de n vertices y m aristas */
int hay_ciclo(Arista aristas[], int m, int n) {
    ConjuntosDisjuntos *cd = crear(n);
    int i, ciclo = 0;
    for (i = 0; i < m && !ciclo; i++) {
        /* si union_rango devuelve 0, u y v ya estaban conectados */
        if (!union_rango(cd, aristas[i].u, aristas[i].v))
            ciclo = 1;
    }
    destruir(cd);
    return ciclo;
}

/* Cuenta las componentes conexas */
int contar_componentes(Arista aristas[], int m, int n) {
    ConjuntosDisjuntos *cd = crear(n);
    int i, componentes = n;
    for (i = 0; i < m; i++) {
        if (union_rango(cd, aristas[i].u, aristas[i].v))
            componentes--;      /* cada union exitosa reduce en 1 */
    }
    destruir(cd);
    return componentes;
}

/* ================================================================
 * PROGRAMA PRINCIPAL: DEMOSTRACIONES
 * ================================================================ */

void demo_basica(void) {
    ConjuntosDisjuntos *cd = crear(8);
    printf("\n=== DEMO 1: operaciones basicas (union por rango) ===\n");
    printf("Estado inicial (8 conjuntos de un elemento):\n");
    imprimir(cd);

    union_rango(cd, 0, 1);
    union_rango(cd, 2, 3);
    union_rango(cd, 1, 3);
    printf("\nDespues de union(0,1), union(2,3), union(1,3):\n");
    imprimir(cd);
    printf("\nConjuntos:\n");
    imprimir_conjuntos(cd);

    printf("\n0 y 2 estan en el mismo conjunto? %s\n",
           mismo_conjunto(cd, 0, 2) ? "SI" : "NO");
    printf("0 y 5 estan en el mismo conjunto? %s\n",
           mismo_conjunto(cd, 0, 5) ? "SI" : "NO");
    destruir(cd);
}

void demo_alturas(void) {
    int n = 16, i;
    ConjuntosDisjuntos *a = crear(n);
    ConjuntosDisjuntos *b = crear(n);
    printf("\n=== DEMO 2: altura del arbol con n = %d ===\n", n);

    /* A: union ingenua en cadena -> arbol degenerado */
    for (i = 0; i < n - 1; i++)
        union_simple(a, i, i + 1);
    printf("Union ingenua      : altura = %d  (profundidad de 0 = %d)\n",
           altura_maxima(a), profundidad(a, 0));

    /* B: union por rango con la misma secuencia */
    for (i = 0; i < n - 1; i++)
        union_rango(b, i, i + 1);
    printf("Union por rango    : altura = %d\n", altura_maxima(b));

    /* C: compresion de caminos sobre el arbol ingenuo */
    printf("\nAntes de find_comp(0) en el arbol ingenuo, profundidad de 0 = %d\n",
           profundidad(a, 0));
    find_comp(a, 0);
    printf("Despues de find_comp(0), profundidad de 0 = %d\n", profundidad(a, 0));
    printf("Despues de find_comp(0), profundidad de 1 = %d\n", profundidad(a, 1));
    destruir(a);
    destruir(b);
}

void demo_tamanio(void) {
    ConjuntosDisjuntos *cd = crear(6);
    printf("\n=== DEMO 3: union por tamanio ===\n");
    union_tamanio(cd, 0, 1);
    union_tamanio(cd, 1, 2);
    union_tamanio(cd, 3, 4);
    union_tamanio(cd, 2, 4);
    imprimir(cd);
    printf("Tamanio del conjunto de 0 = %d\n", cd->tamanio[find_comp(cd, 0)]);
    printf("Tamanio del conjunto de 5 = %d\n", cd->tamanio[find_comp(cd, 5)]);
    destruir(cd);
}

void demo_nodos(void) {
    Nodo *nodos[5];
    int i;
    printf("\n=== DEMO 4: arbol con nodos y punteros ===\n");
    for (i = 0; i < 5; i++)
        nodos[i] = crear_nodo(i * 10);          /* datos 0,10,20,30,40 */

    union_nodo(nodos[0], nodos[1]);
    union_nodo(nodos[2], nodos[3]);
    union_nodo(nodos[1], nodos[3]);

    for (i = 0; i < 5; i++)
        printf("  dato %2d -> representante %2d\n",
               nodos[i]->dato, find_nodo(nodos[i])->dato);

    for (i = 0; i < 5; i++)
        free(nodos[i]);
}

void demo_grafos(void) {
    /* Grafo 1: 0-1, 1-2, 3-4  (6 vertices, sin ciclo, 3 componentes) */
    Arista g1[3];
    /* Grafo 2: 0-1, 1-2, 2-0  (triangulo: tiene ciclo) */
    Arista g2[3];

    g1[0].u = 0; g1[0].v = 1;
    g1[1].u = 1; g1[1].v = 2;
    g1[2].u = 3; g1[2].v = 4;

    g2[0].u = 0; g2[0].v = 1;
    g2[1].u = 1; g2[1].v = 2;
    g2[2].u = 2; g2[2].v = 0;

    printf("\n=== DEMO 5: aplicaciones en grafos ===\n");
    printf("Grafo 1 (6 vertices, 3 aristas):\n");
    printf("  Componentes conexas: %d\n", contar_componentes(g1, 3, 6));
    printf("  Tiene ciclo? %s\n", hay_ciclo(g1, 3, 6) ? "SI" : "NO");
    printf("Grafo 2 (triangulo 0-1-2):\n");
    printf("  Tiene ciclo? %s\n", hay_ciclo(g2, 3, 3) ? "SI" : "NO");
}

int main(void) {
    demo_basica();
    demo_alturas();
    demo_tamanio();
    demo_nodos();
    demo_grafos();

    printf("\n");
    system("pause");        /* para que Dev C++ no cierre la consola */
    return 0;
}
