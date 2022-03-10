#include <stdlib.h>
#include "ficheros_basico.h"


#include <time.h>
/* Fichero: leer_sf.c:
* --------------------s
* La ejecución de este fichero permite mostrar el contenido del superbloque.
*  
*  argc: número de argumentos introducidos por el usuario.
*  argsv: array de strings de argumentos introducidos por el usuario.
*
* return: devuelve Exit_Success o Exit_Failure si ha habido un error.
*/
int main(int argc, char const *argv[])
{
       // Comprueba que la sintaxis sea correcta.
       if (argc != 2)
       {
              fprintf(stderr,
                      "Error de sintaxis: ./leer_sf <nombre_dispositivo>\n");
              return EXIT_FAILURE;
       }

       // Monta el disco en el sistema.
       if (bmount(argv[1]) == -1)
       {
              fprintf(stderr, "Error de montaje de disco.\n");
              return EXIT_FAILURE;
       }

       // Lee el superbloque del disco.
       struct superbloque SB;
       if (bread(0, &SB) == -1)
       {
              fprintf(stderr, "Error de lectura del superbloque.\n");
              return EXIT_FAILURE;
       }

       // Muestra por consola el contenido del superbloque.
       printf("DATOS DEL SUPERBLOQUE\n");
       printf("posPrimerBloqueMB = %d\n", SB.posPrimerBloqueMB);
       printf("posUltimoBloqueMB = %d\n", SB.posUltimoBloqueMB);
       printf("posPrimerBloqueAI = %d\n", SB.posPrimerBloqueAI);
       printf("posUltimoBloqueAI = %d\n", SB.posUltimoBloqueAI);
       printf("posPrimerBloqueDatos = %d\n", SB.posPrimerBloqueDatos);
       printf("posUltimoBloqueDatos = %d\n", SB.posUltimoBloqueDatos);
       printf("posInodoRaiz = %d\n", SB.posInodoRaiz);
       printf("posPrimerInodoLibre = %d\n", SB.posPrimerInodoLibre);
       printf("cantBloquesLibres = %d\n", SB.cantBloquesLibres);
       printf("cantInodosLibres = %d\n", SB.cantInodosLibres);
       printf("totBloques = %d\n", SB.totBloques);
       printf("totInodos = %d\n", SB.totInodos);

        printf("\nsizeof struct superbloque: %ld\n", sizeof(struct superbloque));
    printf("sizeof struct inodo:  %ld\n", sizeof(struct inodo));

    printf("\nRECORRIDO LISTA ENLAZADA DE INODOS LIBRES\n");
    //Podéis hacer también un recorrido de la lista de inodos libres (mostrando para cada inodo el campo punterosDirectos[0]).
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    int contlibres = 0;

    for (int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++)
    {
        //          &inodos
        if (bread(i, inodos) == EXIT_FAILURE)
        {
            return EXIT_FAILURE;
        }

        for (int j = 0; j < BLOCKSIZE / INODOSIZE; j++)
        {
            if ((inodos[j].tipo == 'l'))
            {
                contlibres++;
                if (contlibres < 20)
                {
                    printf("%d ", contlibres);
                }
                else if (contlibres == 21)
                {
                    printf("... ");
                }
                else if ((contlibres > 24990) && (contlibres < SB.totInodos))
                {
                    printf("%d ", contlibres);
                }
                else if (contlibres == SB.totInodos)
                {
                    printf("-1 \n");
                }
                contlibres--;
            }
            contlibres++;
        }
    }
     //Probación el tamaño del tipo time_t para vuestra plataforma/compilador:
    //printf("\nsizeof time_t is: %ld\n", sizeof(time_t));

    printf("\nRESERVAMOS UN BLOQUE Y LUEGO LO LIBERAMOS:\n");
    int reservado = reservar_bloque(); // Actualiza el SB
    bread(posSB, &SB);                 // Actualizar los valores del SB

    printf("Se ha reservado el bloque físico nº %i que era el 1º libre indicado por el MB.\n", reservado);
    printf("SB.cantBloquesLibres: %i\n", SB.cantBloquesLibres);
    liberar_bloque(reservado);
    bread(posSB, &SB); // Actualizar los valores del SB

    printf("Liberamos ese bloque, y después SB.cantBloquesLibres: %i\n\n", SB.cantBloquesLibres);
    printf("MAPA DE BITS CON BLOQUES DE METADATOS OCUPADOS\n");
    int bit = leer_bit(posSB);
    printf("leer_bit(%i) = %i\n", posSB, bit);
    bit = leer_bit(SB.posPrimerBloqueMB);
    printf("leer_bit(%i) = %i\n", SB.posPrimerBloqueMB, bit);
    bit = leer_bit(SB.posUltimoBloqueMB);
    printf("leer_bit(%i) = %i\n", SB.posUltimoBloqueMB, bit);
    bit = leer_bit(SB.posPrimerBloqueAI);
    printf("leer_bit(%i) = %i\n", SB.posPrimerBloqueAI, bit);
    bit = leer_bit(SB.posUltimoBloqueAI);
    printf("leer_bit(%i) = %i\n", SB.posUltimoBloqueAI, bit);
    bit = leer_bit(SB.posPrimerBloqueDatos);
    printf("leer_bit(%i) = %i\n", SB.posPrimerBloqueDatos, bit);
    bit = leer_bit(SB.posUltimoBloqueDatos);
    printf("leer_bit(%i) = %i\n", SB.posUltimoBloqueDatos, bit);
    printf("\nDATOS DEL DIRECTORIO RAIZ\n\n");
    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];

    struct inodo inodo;
    int ninodo = 0; //el directorio raiz es el inodo 0
    leer_inodo(ninodo, &inodo);
    ts = localtime(&inodo.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
    printf("tipo: %c\n", inodo.tipo);
    printf("permisos: %i\n", inodo.permisos);
    printf("ID: %d \nATIME: %s \nMTIME: %s \nCTIME: %s\n", ninodo, atime, mtime, ctime);
    printf("nlinks: %i\n", inodo.nlinks);
    printf("tamaño en bytes lógicos: %i\n", inodo.tamEnBytesLog);
    printf("Número de bloques ocupados: %i\n", inodo.numBloquesOcupados);


       // Desmonta el disco del sistema.
       bumount();
       return EXIT_SUCCESS;
}