//MARC CAÑELLAS, DIEGO BERMEJO, GASTON PANIZZA

#include <stdlib.h>
#include "ficheros.h"


#define tambuffer = 1500
#define DEBUG 1

/*
Le pasaremos por línea de comandos un nº de inodo obtenido con el 
programa anterior (escribir.c). ninodo, (además del nombre del 
dispositivo). Su funcionamiento tiene que ser similar al comando 
cat de Linux, explorando TODO el fichero.
*/
int main(int argc, char const *argv[]){

    //Variables
    int ninodo;
    struct superbloque SB;
    struct inodo inodo;
    //Para lectura
    int offset = 0;
    int nbytes = 1500;
    int bytesleidos = 0;
    char buffer[nbytes];

    //Sintaxis correcta
    if (argc != 3){
        perror("Sintaxis: leer <nombre_dispositivo><numero_inodo>\n");
        return -1;
    }

    //Inicializacion del buffer a 0.
    memset(buffer, 0, nbytes);
    ninodo = atoi(argv[2]);
    // Montar el dispositivo en el sistema.
    if (bmount(argv[1]) == -1){
        perror("leer.c: Error al montar el dispositivo.\n");
        return -1;
    }

    //Leer superbloque
    if (bread(0, &SB) == -1){
        perror("leer.c: Error de lectura del superbloque.\n");
        return -1;
    }

    // Lee del fichero hasta llenar el buffer o fin de fichero.
    int auxBytesbytesleidos = mi_read_f(ninodo, buffer, offset, nbytes);
    while (auxBytesbytesleidos > 0){
        bytesleidos = bytesleidos + auxBytesbytesleidos;
        // Escribe el contenido del buffer en el destino indicado.
        write(1, buffer, auxBytesbytesleidos);

        //Limpiar buffer
        memset(buffer, 0, nbytes);
        //Actulizar offset
        offset = offset + nbytes;
        //Leemos otra vez
        auxBytesbytesleidos = mi_read_f(ninodo, buffer, offset, nbytes);
    }

    // Leer el inodo del archivo
    if (leer_inodo(ninodo, &inodo)){
        perror("Error con la lectura del inodo.\n");
        return -1;
    }

#if DEBUG
    fprintf(stderr, "total_bytesleidos: %d\ntamEnBytesLog: %d\n", bytesleidos, inodo.tamEnBytesLog);
#endif

    // Desmonta el dispositivo virtual
    if (bumount() == -1){
        perror("leer.c: Error al desmonta el dispositivo virtual.\n");
        return -1;
    }
    return EXIT_SUCCESS;
}
