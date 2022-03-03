#include "ficheros_basico.h"

struct superbloque SB; //Definimos la zona de memoria (variable de tipo superbloque)
int initSB(unsigned int nbloques, unsigned int ninodos)
{
    //posicion del primer bloque del mapa de bits bits
    SB.posPrimerBloqueMB = posSB + tamSB;
    //Posición del último bloque del mapa de bits
    SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;
    //Posición del primer bloque del array de inodos
    SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;
    //Posición del último bloque del array de inodos
    SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) - 1;
    //Posición del primer bloque de datos
    SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;
    //Posición del último bloque de datos
    SB.posUltimoBloqueDatos = nbloques - 1;
    //Posición del inodo del directorio raíz en el array de inodos
    SB.posInodoRaiz = 0;
    //Posición del primer inodo libre en el array de inodos
    SB.posPrimerInodoLibre = 0;
    //Cantidad de bloques libres en el SF
    SB.cantBloquesLibres = nbloques;
    //Cantidad de inodos libres en el array de inodo
    SB.cantInodosLibres = ninodos;
    //Cantidad total de bloques
    SB.totBloques = nbloques;
    //Cantidad total de inodos
    SB.totInodos = ninodos;
	
	if(bwrite(posSB,&sb) < 0) return -1; //Error BWRITE	
	return 0;
}