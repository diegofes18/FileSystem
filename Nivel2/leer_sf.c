//#include <stdlib.h>
#include "ficheros_basico.h"
//#include "bloques.h"
/* Fichero: leer_f.c:
* --------------------s
* La ejecución de este fichero permite mostrar el contenido del superbloque.
*  
*  argc: número de argumentos introducidos por el usuario.
*  argsv: array de strings de argumentos introducidos por el usuario.
*
* return: devuelve Exit_Success o Exit_Failure si ha habido un error.
*/

int main(int argc, char const *argv[]){
       //comprobamos la sintaxis
       if (argc != 2){
              perror("Error de sintaxis: ./leer_sf <nombre_dispositivo>\n");
              return -1;
       }

       //montamos el disco
       if (bmount(argv[1]) == -1){
              perror("Error de montaje de disco.\n");
              return -1;
       }

       //lectura del superbloque
       struct superbloque SB;
       if (bread(0, &SB) == -1){
              perror("Error de lectura del superbloque.\n");
              return -1;
       }

       //muestra por consola el contenido del superbloque
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
        struct inodo inodos[BLOCKSIZE / INODOSIZE];
        int contlibres = 0;

         for (int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++){
            if (bread(i, inodos) == -1){
                return -1;
            }

            for (int j = 0; j < BLOCKSIZE / INODOSIZE; j++){
                if ((inodos[j].tipo == 'l')){
                    contlibres++;
                    if (contlibres < 20){
                        printf("%d ", contlibres);

                    }else if (contlibres == 21){
                    printf("... ");

                }else if ((contlibres > 24990) && (contlibres < SB.totInodos)){
                    printf("%d ", contlibres);

                }else if (contlibres == SB.totInodos){
                    printf("-1 \n");
                }

                contlibres--;
            }
            contlibres++;
        }
    }

       //desmontamos
       bumount();
       
       return EXIT_SUCCESS;
}