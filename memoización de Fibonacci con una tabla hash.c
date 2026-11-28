#include <stdio.h>
#include <stdlib.h>
 
#define M 101                      /* tamaño (primo) de la tabla hash */
 
typedef struct Nodo {
    int clave;                     /* n */
    long long valor;               /* F(n) */
    struct Nodo *siguiente;
} Nodo;
 
static Nodo *tabla[M];             /* encadenamiento: arreglo de listas */
static long llamadas;
Nodo *p; 
static Nodo *buscar_memo(int n) {
    for (p = tabla[n % M]; p != NULL; p = p->siguiente)
        if (p->clave == n) return p;
    return NULL;
}
 
static void guardar_memo(int n, long long f) {
    Nodo *nuevo = malloc(sizeof(Nodo));
    if (nuevo == NULL) return;     /* sin memoria: no se memoriza, el resultado sigue siendo correcto */
    nuevo->clave = n;
    nuevo->valor = f;
    nuevo->siguiente = tabla[n % M];
    tabla[n % M] = nuevo;
}
int i; 
static void liberar_memo(void) {
    for (i = 0; i < M; i++) {
        Nodo *p = tabla[i];
        while (p != NULL) {
            Nodo *tmp = p;
            p = p->siguiente;
            free(tmp);
        }
        tabla[i] = NULL;
    }
}
 
long long fib_simple(int n) {
    llamadas++;
    if (n < 2) return n;
    return fib_simple(n - 1) + fib_simple(n - 2);
}
 
long long fib_memo(int n) {
    llamadas++;
    if (n < 2) return n;
    Nodo *p = buscar_memo(n);
    if (p != NULL) return p->valor;          /* ya calculado: O(1) promedio */
    long long f = fib_memo(n - 1) + fib_memo(n - 2);
    guardar_memo(n, f);
    return f;
}
 
int main(void) {
    int n = 35;
    llamadas = 0;
    long long a = fib_simple(n);
    printf("Sin memoizacion: F(%d) = %lld con %ld llamadas\n", n, a, llamadas);
    llamadas = 0;
    long long b = fib_memo(n);
    printf("Con memoizacion: F(%d) = %lld con %ld llamadas\n", n, b, llamadas);
    liberar_memo();
    return 0;
}

