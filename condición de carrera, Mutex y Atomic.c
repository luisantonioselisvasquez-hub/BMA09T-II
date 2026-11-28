#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
 
#define HILOS 4
#define INCREMENTOS 1000000
 
long contador_normal = 0;
long contador_mutex = 0;
atomic_long contador_atomico = 0;
pthread_mutex_t candado = PTHREAD_MUTEX_INITIALIZER;
 int i;
void *sin_proteccion(void *arg) {
    for (i = 0; i < INCREMENTOS; i++) {
        contador_normal++;}
    return NULL;}
 
void *con_mutex(void *arg) {
    for (i = 0; i < INCREMENTOS; i++) {
        pthread_mutex_lock(&candado);
        contador_mutex++;
        pthread_mutex_unlock(&candado);}
    return NULL;}
 
void *con_atomic(void *arg) {
    for (i = 0; i < INCREMENTOS; i++) {
        atomic_fetch_add(&contador_atomico, 1);}
    return NULL;}
 
void ejecutar(void *(*funcion)(void *)) {
    pthread_t hilos[HILOS];
    for (i = 0; i < HILOS; i++) {
        pthread_create(&hilos[i], NULL, funcion, NULL);}
    for (i = 0; i < HILOS; i++) {
        pthread_join(hilos[i], NULL);}}
 
int main(void) {
    long esperado = (long)HILOS * INCREMENTOS;
    ejecutar(sin_proteccion);
    ejecutar(con_mutex);
    ejecutar(con_atomic);
    printf("Valor esperado:  %ld\n", esperado);
    printf("Sin proteccion:  %ld\n", contador_normal);
    printf("Con mutex:       %ld\n", contador_mutex);
    printf("Con atomic:      %ld\n", (long)atomic_load(&contador_atomico));
    return 0;}
 
