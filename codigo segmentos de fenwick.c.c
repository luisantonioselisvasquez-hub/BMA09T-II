
/* En MinGW antiguo esto hace que printf soporte %lld correctamente. */
#ifndef __USE_MINGW_ANSI_STDIO
#define __USE_MINGW_ANSI_STDIO 1
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Valores "infinito" propios (no dependen de limits.h). */
#define INF_POS  9223372036854775807LL
#define INF_NEG  (-9223372036854775807LL - 1)

/* ============================================================================
 *  PARTE 1: ARBOL DE SEGMENTOS CON PROPAGACION DIFERIDA
 *  Indices en base 0, intervalos cerrados [l, r].
 *  Cada nodo guarda la suma, el minimo y el maximo de su intervalo.
 *    construccion O(n) | consulta O(log n) | actualizacion puntual O(log n)
 *    suma sobre un rango con lazy O(log n)
 * ============================================================================ */

typedef struct {
    long long suma;
    long long min;
    long long max;
} Resumen;

typedef struct {
    int n;               /* numero de elementos del arreglo             */
    long long *sum;      /* sum[nodo]  = suma del intervalo del nodo    */
    long long *mn;       /* mn[nodo]   = minimo del intervalo           */
    long long *mx;       /* mx[nodo]   = maximo del intervalo           */
    long long *lazy;     /* lazy[nodo] = incremento pendiente a hijos   */
    long long visitas;   /* nodos visitados (para mostrar el O(log n))  */
} SegTree;

/* Raiz en el nodo 1; hijos del nodo i: 2i (izquierdo) y 2i+1 (derecho). */
#define IZQ(i) (2 * (i))
#define DER(i) (2 * (i) + 1)

/* Elemento neutro: no altera el resultado al combinar intervalos. */
static Resumen neutro(void) {
    Resumen r;
    r.suma = 0;
    r.min = INF_POS;
    r.max = INF_NEG;
    return r;
}

static Resumen combinar(Resumen a, Resumen b) {
    Resumen r;
    r.suma = a.suma + b.suma;
    r.min = (a.min < b.min) ? a.min : b.min;
    r.max = (a.max > b.max) ? a.max : b.max;
    return r;
}

/* Recalcula un nodo a partir de sus dos hijos. */
static void subir(SegTree *t, int nodo) {
    long long mn_i = t->mn[IZQ(nodo)], mn_d = t->mn[DER(nodo)];
    long long mx_i = t->mx[IZQ(nodo)], mx_d = t->mx[DER(nodo)];
    t->sum[nodo] = t->sum[IZQ(nodo)] + t->sum[DER(nodo)];
    t->mn[nodo] = (mn_i < mn_d) ? mn_i : mn_d;
    t->mx[nodo] = (mx_i > mx_d) ? mx_i : mx_d;
}

/* Aplica "sumar v a todo el intervalo" al nodo y deja la marca pendiente.
 * len = cantidad de elementos que cubre el nodo. */
static void aplicar(SegTree *t, int nodo, int len, long long v) {
    t->sum[nodo] += v * len;   /* cada uno de los len elementos sube v */
    t->mn[nodo] += v;
    t->mx[nodo] += v;
    if (len > 1)               /* las hojas no tienen hijos a quienes avisar */
        t->lazy[nodo] += v;    /* los hijos aun no se enteraron */
}

/* Empuje (push down): traslada la marca pendiente del nodo a sus hijos. */
static void empujar(SegTree *t, int nodo, int l, int r) {
    int m;
    if (t->lazy[nodo] != 0) {
        m = (l + r) / 2;
        aplicar(t, IZQ(nodo), m - l + 1, t->lazy[nodo]);
        aplicar(t, DER(nodo), r - m, t->lazy[nodo]);
        t->lazy[nodo] = 0;
    }
}

/* Construccion recursiva: O(n). */
static void st_construir(SegTree *t, const long long *a, int nodo, int l, int r) {
    int m;
    t->lazy[nodo] = 0;
    if (l == r) {                       /* hoja: un solo elemento */
        t->sum[nodo] = a[l];
        t->mn[nodo] = a[l];
        t->mx[nodo] = a[l];
        return;
    }
    m = (l + r) / 2;
    st_construir(t, a, IZQ(nodo), l, m);
    st_construir(t, a, DER(nodo), m + 1, r);
    subir(t, nodo);                     /* el nodo combina a sus hijos */
}

static void st_liberar(SegTree *t) {
    if (t == NULL) return;
    free(t->sum);
    free(t->mn);
    free(t->mx);
    free(t->lazy);
    free(t);
}

static SegTree *st_crear(const long long *a, int n) {
    SegTree *t;
    if (n <= 0) return NULL;
    t = (SegTree *)malloc(sizeof(SegTree));
    if (t == NULL) return NULL;
    t->n = n;
    t->visitas = 0;
    /* 4n posiciones bastan para cualquier n */
    t->sum  = (long long *)calloc(4 * (size_t)n, sizeof(long long));
    t->mn   = (long long *)calloc(4 * (size_t)n, sizeof(long long));
    t->mx   = (long long *)calloc(4 * (size_t)n, sizeof(long long));
    t->lazy = (long long *)calloc(4 * (size_t)n, sizeof(long long));
    if (!t->sum || !t->mn || !t->mx || !t->lazy) {
        st_liberar(t);
        return NULL;
    }
    st_construir(t, a, 1, 0, n - 1);
    return t;
}

/* Consulta de rango: O(log n). Tres casos: disjunto, contenido, parcial. */
static Resumen st_consultar_rec(SegTree *t, int nodo, int l, int r, int ql, int qr) {
    Resumen res;
    int m;
    t->visitas++;
    if (qr < l || r < ql)               /* caso 1: disjunto */
        return neutro();
    if (ql <= l && r <= qr) {           /* caso 2: contenido */
        res.suma = t->sum[nodo];
        res.min = t->mn[nodo];
        res.max = t->mx[nodo];
        return res;
    }
    empujar(t, nodo, l, r);             /* caso 3: parcial, hay que bajar */
    m = (l + r) / 2;
    return combinar(st_consultar_rec(t, IZQ(nodo), l, m, ql, qr),
                    st_consultar_rec(t, DER(nodo), m + 1, r, ql, qr));
}

static Resumen st_consultar(SegTree *t, int l, int r) {
    t->visitas = 0;
    return st_consultar_rec(t, 1, 0, t->n - 1, l, r);
}

/* Actualizacion puntual (A[i] = valor): O(log n). */
static void st_asignar_rec(SegTree *t, int nodo, int l, int r, int i, long long valor) {
    int m;
    t->visitas++;
    if (l == r) {                       /* llegamos a la hoja */
        t->sum[nodo] = valor;
        t->mn[nodo] = valor;
        t->mx[nodo] = valor;
        t->lazy[nodo] = 0;
        return;
    }
    empujar(t, nodo, l, r);
    m = (l + r) / 2;
    if (i <= m) st_asignar_rec(t, IZQ(nodo), l, m, i, valor);
    else        st_asignar_rec(t, DER(nodo), m + 1, r, i, valor);
    subir(t, nodo);                     /* recalcula los ancestros al volver */
}

static void st_actualizar_punto(SegTree *t, int i, long long valor) {
    t->visitas = 0;
    st_asignar_rec(t, 1, 0, t->n - 1, i, valor);
}

/* Actualizacion de rango con propagacion diferida (A[l..r] += v): O(log n). */
static void st_sumar_rec(SegTree *t, int nodo, int l, int r, int ql, int qr, long long v) {
    int m;
    t->visitas++;
    if (qr < l || r < ql) return;       /* disjunto: nada que hacer */
    if (ql <= l && r <= qr) {           /* contenido: actualizar y dejar marca */
        aplicar(t, nodo, r - l + 1, v);
        return;
    }
    empujar(t, nodo, l, r);             /* parcial: primero bajar lo pendiente */
    m = (l + r) / 2;
    st_sumar_rec(t, IZQ(nodo), l, m, ql, qr, v);
    st_sumar_rec(t, DER(nodo), m + 1, r, ql, qr, v);
    subir(t, nodo);
}

static void st_sumar_rango(SegTree *t, int l, int r, long long v) {
    t->visitas = 0;
    st_sumar_rec(t, 1, 0, t->n - 1, l, r, v);
}

/* Dibuja el arbol por niveles (cada nivel se indenta 4 espacios). */
static void st_imprimir_rec(const SegTree *t, int nodo, int l, int r, int prof) {
    int m;
    printf("%*s[%d,%d] suma=%lld min=%lld max=%lld", prof * 4, "", l, r,
           t->sum[nodo], t->mn[nodo], t->mx[nodo]);
    if (t->lazy[nodo] != 0)
        printf("  (lazy pendiente=%lld)", t->lazy[nodo]);
    printf("\n");
    if (l == r) return;
    m = (l + r) / 2;
    st_imprimir_rec(t, IZQ(nodo), l, m, prof + 1);
    st_imprimir_rec(t, DER(nodo), m + 1, r, prof + 1);
}

static void st_imprimir(const SegTree *t) {
    st_imprimir_rec(t, 1, 0, t->n - 1, 0);
}

/* ============================================================================
 *  PARTE 2: ARBOL DE FENWICK (arbol indexado binario)
 *  Indices publicos en BASE 1 (A[1..n]). fw_crear recibe un arreglo C normal
 *  (base 0) y lo copia como A[1..n].
 *    construccion O(n) | prefijo(i) O(log n) | actualizar(i, d) O(log n)
 *    rango(l, r) = prefijo(r) - prefijo(l-1)
 * ============================================================================ */

typedef struct {
    int n;
    long long *f;        /* f[1..n]; f[i] = suma del intervalo (i - lowbit(i), i] */
    long long visitas;   /* posiciones de f recorridas en la ultima operacion */
} Fenwick;

/* Bit 1 menos significativo de i. Ej.: lowbit(6)=2, lowbit(8)=8, lowbit(7)=1. */
static int lowbit(int i) {
    return i & (-i);
}

static void fw_liberar(Fenwick *t) {
    if (t == NULL) return;
    free(t->f);
    free(t);
}

/* Construccion lineal O(n): se copia A en F y cada posicion "empuja" su valor
 * a la siguiente posicion que la cubre, i + lowbit(i). */
static Fenwick *fw_crear(const long long *a, int n) {
    Fenwick *t;
    int i, j;
    if (n <= 0) return NULL;
    t = (Fenwick *)malloc(sizeof(Fenwick));
    if (t == NULL) return NULL;
    t->n = n;
    t->visitas = 0;
    t->f = (long long *)calloc((size_t)n + 1, sizeof(long long));
    if (t->f == NULL) {
        free(t);
        return NULL;
    }
    for (i = 1; i <= n; i++)
        t->f[i] = a[i - 1];
    for (i = 1; i <= n; i++) {
        j = i + lowbit(i);
        if (j <= n) t->f[j] += t->f[i];
    }
    return t;
}

/* Suma de los primeros i elementos: en cada paso se quita el bit menos
 * significativo, asi que hay a lo sumo floor(log2 n)+1 pasos. */
static long long fw_prefijo(Fenwick *t, int i) {
    long long s = 0;
    t->visitas = 0;
    while (i > 0) {
        t->visitas++;
        s += t->f[i];
        i -= lowbit(i);
    }
    return s;
}

/* A[i] += delta: se actualizan todas las posiciones cuyo intervalo contiene a i. */
static void fw_actualizar(Fenwick *t, int i, long long delta) {
    t->visitas = 0;
    while (i <= t->n) {
        t->visitas++;
        t->f[i] += delta;
        i += lowbit(i);
    }
}

/* Suma de A[l..r] como diferencia de dos prefijos (la suma es invertible). */
static long long fw_rango(Fenwick *t, int l, int r) {
    return fw_prefijo(t, r) - fw_prefijo(t, l - 1);
}

static void fw_imprimir_tabla(const Fenwick *t) {
    char intervalo[32];
    int i, lb;
    printf("  i | lowbit | intervalo | F[i]\n");
    printf("  --+--------+-----------+------\n");
    for (i = 1; i <= t->n; i++) {
        lb = lowbit(i);
        sprintf(intervalo, "[%d,%d]", i - lb + 1, i);
        printf(" %2d | %6d | %-9s | %lld\n", i, lb, intervalo, t->f[i]);
    }
}

static void fw_trazar_consulta(const Fenwick *t, int i) {
    int k;
    (void)t;
    printf("  posiciones de F recorridas en prefijo(%d): ", i);
    for (k = i; k > 0; k -= lowbit(k)) {
        printf("%d", k);
        if (k - lowbit(k) > 0) printf(" -> ");
    }
    printf("\n");
}

static void fw_trazar_actualizacion(const Fenwick *t, int i) {
    int k;
    printf("  posiciones de F modificadas al actualizar A[%d]: ", i);
    for (k = i; k <= t->n; k += lowbit(k)) {
        printf("%d", k);
        if (k + lowbit(k) <= t->n) printf(" -> ");
    }
    printf("\n");
}

/* ============================================================================
 *  PARTE 3: DEMOSTRACION
 * ============================================================================ */

static void titulo(const char *texto) {
    printf("\n==============================================================\n");
    printf(" %s\n", texto);
    printf("==============================================================\n");
}

/* Detiene el programa para poder tomar la captura de pantalla. */
static void pausar(void) {
    int c;
    printf("\n>>> Presione ENTER para continuar...");
    fflush(stdout);
    while ((c = getchar()) != '\n' && c != EOF) { }
}

static void imprimir_arreglo(const char *nombre, const long long *a, int n) {
    int i;
    printf("%s = [", nombre);
    for (i = 0; i < n; i++) {
        printf("%lld", a[i]);
        if (i < n - 1) printf(", ");
    }
    printf("]\n");
}

/* Cantidad de niveles del arbol: ceil(log2 n) + 1 (cota de nodos = 4 * niveles). */
static int niveles(int n) {
    int k = 0;
    while ((1 << k) < n) k++;
    return k + 1;
}

/* Solucion ingenua de referencia (fuerza bruta). */
static Resumen fuerza_bruta(const long long *a, int l, int r) {
    Resumen res;
    int i;
    res.suma = 0;
    res.min = INF_POS;
    res.max = INF_NEG;
    for (i = l; i <= r; i++) {
        res.suma += a[i];
        if (a[i] < res.min) res.min = a[i];
        if (a[i] > res.max) res.max = a[i];
    }
    return res;
}

static double segundos(clock_t ini) {
    return (double)(clock() - ini) / CLOCKS_PER_SEC;
}

/* ---------------- 1. El problema ---------------- */
static void seccion_problema(void) {
    long long a[6];
    long long s = 0;
    int i;
    a[0] = 5; a[1] = 2; a[2] = 7; a[3] = 1; a[4] = 4; a[5] = 3;

    titulo("1. PROBLEMA DE LAS CONSULTAS DE RANGO");
    imprimir_arreglo("A", a, 6);
    printf("Consulta de ejemplo: suma de A[1..4]\n");
    for (i = 1; i <= 4; i++) s += a[i];
    printf("  Recorriendo el intervalo (O(n)): %lld\n", s);
    printf("  Con sumas de prefijos, la consulta es O(1) pero actualizar un\n");
    printf("  elemento obliga a recalcular O(n) prefijos. Los arboles equilibran\n");
    printf("  ambas operaciones en O(log n).\n");
    pausar();
}

/* ---------------- 2. Arbol de segmentos ---------------- */
static void seccion_segmentos(void) {
    long long a[6];
    int n = 6, k;
    int consultas[3][2];
    SegTree *t;
    Resumen res;
    a[0] = 5; a[1] = 2; a[2] = 7; a[3] = 1; a[4] = 4; a[5] = 3;
    consultas[0][0] = 1; consultas[0][1] = 4;
    consultas[1][0] = 0; consultas[1][1] = 5;
    consultas[2][0] = 2; consultas[2][1] = 2;

    titulo("2. ARBOL DE SEGMENTOS");
    t = st_crear(a, n);
    imprimir_arreglo("A", a, n);
    printf("\nArbol construido (cada nodo muestra su intervalo [l,r]):\n");
    st_imprimir(t);
    pausar();

    titulo("2. ARBOL DE SEGMENTOS: consultas y actualizacion puntual");
    printf("-- Consultas (indices base 0) --\n");
    for (k = 0; k < 3; k++) {
        res = st_consultar(t, consultas[k][0], consultas[k][1]);
        printf("consulta [%d,%d]: suma=%lld min=%lld max=%lld  (nodos visitados: %lld, cota 4*%d=%d)\n",
               consultas[k][0], consultas[k][1], res.suma, res.min, res.max,
               t->visitas, niveles(n), 4 * niveles(n));
    }
    printf("\n-- Actualizacion puntual: A[2] = 10 --\n");
    st_actualizar_punto(t, 2, 10);
    a[2] = 10;
    printf("nodos visitados: %lld (camino raiz-hoja)\n", t->visitas);
    imprimir_arreglo("A", a, n);
    res = st_consultar(t, 1, 4);
    printf("consulta [1,4] despues de actualizar: suma=%lld min=%lld max=%lld\n",
           res.suma, res.min, res.max);
    st_liberar(t);
    pausar();
}

/* ---------------- 3. Propagacion diferida ---------------- */
static void seccion_lazy(void) {
    long long a[6];
    int n = 6;
    SegTree *t;
    Resumen res;
    a[0] = 5; a[1] = 2; a[2] = 10; a[3] = 1; a[4] = 4; a[5] = 3;

    titulo("3. PROPAGACION DIFERIDA (LAZY PROPAGATION)");
    t = st_crear(a, n);
    imprimir_arreglo("A", a, n);
    printf("\nActualizacion de rango: sumar 3 a A[1..4]\n");
    st_sumar_rango(t, 1, 4, 3);
    printf("nodos visitados: %lld (una sola operacion, sin tocar las 4 hojas)\n", t->visitas);
    printf("Arbol: observe las marcas pendientes; los hijos aun no se actualizaron:\n");
    st_imprimir(t);
    pausar();

    titulo("3. PROPAGACION DIFERIDA: consulta que empuja las marcas");
    printf("Consulta [2,3]: obliga a empujar las marcas hacia abajo\n");
    res = st_consultar(t, 2, 3);
    printf("resultado: suma=%lld min=%lld max=%lld\n", res.suma, res.min, res.max);
    printf("(A[2]=10+3=13, A[3]=1+3=4 -> suma 17, min 4, max 13)\n");
    printf("Arbol despues de empujar:\n");
    st_imprimir(t);
    st_liberar(t);
    pausar();
}

/* ---------------- 4. Arbol de Fenwick ---------------- */
static void seccion_fenwick(void) {
    long long a[8];
    int n = 8;
    long long p7;
    Fenwick *f;
    a[0] = 2; a[1] = 5; a[2] = 1; a[3] = 4; a[4] = 3; a[5] = 6; a[6] = 7; a[7] = 8;

    titulo("4. ARBOL DE FENWICK (indices base 1: A[1..8])");
    f = fw_crear(a, n);
    imprimir_arreglo("A[1..8]", a, n);
    printf("\nIntervalo que cubre cada posicion de F:\n");
    fw_imprimir_tabla(f);

    printf("\n-- Consulta de prefijo: prefijo(7) = A[1]+...+A[7] --\n");
    fw_trazar_consulta(f, 7);
    p7 = fw_prefijo(f, 7);
    printf("  resultado = %lld (esperado 2+5+1+4+3+6+7 = 28), pasos: %lld\n", p7, f->visitas);
    printf("\n-- Suma del intervalo [3,6] = prefijo(6) - prefijo(2) --\n");
    printf("  resultado = %lld (esperado 1+4+3+6 = 14)\n", fw_rango(f, 3, 6));
    pausar();

    titulo("4. ARBOL DE FENWICK: actualizacion");
    printf("-- Actualizacion: A[3] += 10 --\n");
    fw_trazar_actualizacion(f, 3);
    fw_actualizar(f, 3, 10);
    printf("  pasos: %lld\n", f->visitas);
    printf("  nuevo prefijo(7) = %lld (esperado 38)\n", fw_prefijo(f, 7));
    printf("  nuevo rango [3,6] = %lld (esperado 24)\n", fw_rango(f, 3, 6));
    printf("\nTabla de F despues de la actualizacion:\n");
    fw_imprimir_tabla(f);
    fw_liberar(f);
    pausar();
}

/* ---------------- 5. Verificacion y comparacion ---------------- */
static void seccion_comparacion(void) {
    int n = 500, ops = 20000;
    int N = 50000, Q = 20000;
    int i, k, l, r, tipo, tmp, errores, errores_fw;
    long long v, d, x, max_visitas_st, max_pasos_fw;
    long long *a, *b, *ref, *xs, *ys;
    int *ql, *qr, *pos;
    SegTree *st, *s2;
    Fenwick *fw, *f2;
    Resumen rx, ry;
    clock_t ini;
    volatile long long sumidero = 0;      /* evita que se optimice el bucle */

    titulo("5. VERIFICACION Y COMPARACION");
    srand(12345);

    /* ---- 5.1 Verificacion contra fuerza bruta ---- */
    a = (long long *)malloc(n * sizeof(long long));
    b = (long long *)malloc(n * sizeof(long long));
    ref = (long long *)malloc(n * sizeof(long long));
    for (i = 0; i < n; i++) {
        a[i] = rand() % 2001 - 1000;
        b[i] = a[i];
        ref[i] = a[i];
    }
    st = st_crear(a, n);
    fw = fw_crear(b, n);
    errores = 0;
    max_visitas_st = 0;
    max_pasos_fw = 0;

    for (k = 0; k < ops; k++) {
        tipo = rand() % 3;
        l = rand() % n;
        r = rand() % n;
        if (l > r) { tmp = l; l = r; r = tmp; }
        if (tipo == 0) {                              /* consulta */
            rx = st_consultar(st, l, r);
            ry = fuerza_bruta(ref, l, r);
            if (rx.suma != ry.suma || rx.min != ry.min || rx.max != ry.max) errores++;
            if (st->visitas > max_visitas_st) max_visitas_st = st->visitas;
        } else if (tipo == 1) {                       /* asignacion puntual */
            v = rand() % 2001 - 1000;
            st_actualizar_punto(st, l, v);
            ref[l] = v;
        } else {                                      /* suma sobre un rango */
            v = rand() % 21 - 10;
            st_sumar_rango(st, l, r, v);
            for (i = l; i <= r; i++) ref[i] += v;
        }
    }
    printf("Arbol de segmentos: %d operaciones aleatorias contra fuerza bruta -> %s\n",
           ops, errores == 0 ? "TODO CORRECTO" : "HAY ERRORES");
    printf("  maximo de nodos visitados en una consulta: %lld (cota teorica %d)\n",
           max_visitas_st, 4 * niveles(n));

    for (i = 0; i < n; i++) ref[i] = a[i];
    errores_fw = 0;
    for (k = 0; k < ops; k++) {
        l = rand() % n;
        r = rand() % n;
        if (l > r) { tmp = l; l = r; r = tmp; }
        if (rand() % 2) {
            d = rand() % 2001 - 1000;
            fw_actualizar(fw, l + 1, d);              /* base 0 -> base 1 */
            ref[l] += d;
            if (fw->visitas > max_pasos_fw) max_pasos_fw = fw->visitas;
        } else {
            x = fw_rango(fw, l + 1, r + 1);
            if (x != fuerza_bruta(ref, l, r).suma) errores_fw++;
        }
    }
    printf("Arbol de Fenwick:   %d operaciones aleatorias contra fuerza bruta -> %s\n",
           ops, errores_fw == 0 ? "TODO CORRECTO" : "HAY ERRORES");
    printf("  maximo de pasos en una actualizacion: %lld (cota teorica %d)\n",
           max_pasos_fw, niveles(n));
    st_liberar(st);
    fw_liberar(fw);
    free(a);
    free(b);
    free(ref);

    /* ---- 5.2 Comparacion de tiempos ---- */
    xs = (long long *)malloc(N * sizeof(long long));
    ys = (long long *)malloc(N * sizeof(long long));
    ql = (int *)malloc(Q * sizeof(int));
    qr = (int *)malloc(Q * sizeof(int));
    pos = (int *)malloc(Q * sizeof(int));
    for (i = 0; i < N; i++) {
        xs[i] = rand() % 1000;
        ys[i] = xs[i];
    }
    for (k = 0; k < Q; k++) {
        ql[k] = rand() % N;
        qr[k] = rand() % N;
        if (ql[k] > qr[k]) { tmp = ql[k]; ql[k] = qr[k]; qr[k] = tmp; }
        pos[k] = rand() % N;
    }
    printf("\nTiempos con n = %d elementos y %d pares (actualizacion + consulta de suma):\n", N, Q);

    ini = clock();
    for (k = 0; k < Q; k++) {
        long long s = 0;
        xs[pos[k]] += 1;
        for (i = ql[k]; i <= qr[k]; i++) s += xs[i];
        sumidero += s;
    }
    printf("  recorrer el intervalo (ingenuo) : %.4f s\n", segundos(ini));

    s2 = st_crear(ys, N);
    ini = clock();
    for (k = 0; k < Q; k++) {
        st_sumar_rango(s2, pos[k], pos[k], 1);
        sumidero += st_consultar(s2, ql[k], qr[k]).suma;
    }
    printf("  arbol de segmentos              : %.4f s\n", segundos(ini));
    st_liberar(s2);

    f2 = fw_crear(ys, N);
    ini = clock();
    for (k = 0; k < Q; k++) {
        fw_actualizar(f2, pos[k] + 1, 1);
        sumidero += fw_rango(f2, ql[k] + 1, qr[k] + 1);
    }
    printf("  arbol de Fenwick                : %.4f s\n", segundos(ini));
    fw_liberar(f2);
    free(xs);
    free(ys);
    free(ql);
    free(qr);
    free(pos);

    printf("\nNota: Fenwick solo sirve para operaciones invertibles (suma); para\n");
    printf("minimo/maximo y actualizaciones de rango se usa el arbol de segmentos.\n");
    pausar();
}

int main(void) {
    seccion_problema();
    seccion_segmentos();
    seccion_lazy();
    seccion_fenwick();
    seccion_comparacion();
    return 0;
}
