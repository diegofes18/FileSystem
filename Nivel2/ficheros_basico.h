#include <bloques.h>

#define posSB 0 // el superbloque se escribe en el primer bloque de nuestro FS
#define tamSB 1

int tamMB(unsigned int nbloques);
int tamAI(unsigned int ninodos);
int initSB(unsigned int nbloques, unsigned int ninodos);
int initMB(); 
int initAI();
