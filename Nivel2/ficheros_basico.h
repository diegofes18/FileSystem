#include <bloques.h>

#define posSB 0 // el superbloque se escribe en el primer bloque de nuestro FS
#define tamSB 1

struct superbloque {
   unsigned int posPrimerBloqueMB;                      // Posición del primer bloque del mapa de bits
   unsigned int posUltimoBloqueMB;                      // Posición del último bloque del mapa de bits
   unsigned int posPrimerBloqueAI;                      // Posición del primer bloque del array de inodos
   unsigned int posUltimoBloqueAI;                      // Posición del último bloque del array de inodos
   unsigned int posPrimerBloqueDatos;                   // Posición del primer bloque de datos
   unsigned int posUltimoBloqueDatos;                   // Posición del último bloque de datos
   unsigned int posInodoRaiz;                           // Posición del inodo del directorio raíz
   unsigned int posPrimerInodoLibre;                    // Posición del primer inodo libre
   unsigned int cantBloquesLibres;                      // Cantidad de bloques libres
   unsigned int cantInodosLibres;                       // Cantidad de inodos libres
   unsigned int totBloques;                             // Cantidad total de bloques
   unsigned int totInodos;                              // Cantidad total de inodos
   char padding[BLOCKSIZE - 12 * sizeof(unsigned int)]; // Relleno
};
int tamMB(unsigned int nbloques);
int tamAI(unsigned int ninodos);
int initSB(unsigned int nbloques, unsigned int ninodos);
int initMB(); 
int initAI();
