#include <stdlib.h>
#include "ficheros.h"
/**
 * Son programas externos ficticios, sólo para probar, temporalmente, 
 * las funcionalidades de lectura/escritura y cambio de permisos, 
 * que involucran funciones de las 3 capas inferiores de nuestra 
 * biblioteca del sistema de ficheros, pero estos programas 
 * NO forman parte del sistema de ficheros.
 * 
 * Programa que comprueba si el número de argumentos es correcto y
 * en caso contrario mostrar la sintaxis.
 * 
 * ---------------------------------------------------------------------
 *                          leer.c:
 * ---------------------------------------------------------------------
 * 
 * Le pasaremos por línea de comandos un nº de inodo obtenido con el 
 * programa anterior (escribir.c). ninodo, (además del nombre del 
 * dispositivo). Su funcionamiento tiene que ser similar al comando 
 * cat de Linux, explorando TODO el fichero.
 * 
*/

int main(int argc, char const *argv[]){

    //declaración de variables
    struct inodo inodo;
    int ninodo;
    struct superbloque SB;
    int tambuffer = 1500;
    int bytesleidos = 0;
    int offset = 0;
    char buffer[tambuffer];

    //miramos que la sintaxis sea la adecuada
    if (argc != 3){
        perror("Sintaxis: leer <nombre_dispositivo><numero_inodo>\n");
        return -1;
    }

    //ponemos todas las posiciones de buffer a 0 mediante memset
    if(memset(buffer, 0, tambuffer)==NULL){
        return -1;
    }

    ninodo = atoi(argv[2]);

    //montamos el dispositivo
    if (bmount(argv[1]) == -1){
        perror("leer.c: Error al montar el dispositivo.\n");
        return -1;
    }

    //lectura del superbloque con el metodo bread
    if (bread(0, &SB) == -1){
        perror("leer.c: Error de lectura del superbloque.\n");
        return -1;
    }

    // Lee del fichero hasta llenar el buffer o fin de fichero.
    int leidos = mi_read_f(ninodo, buffer, offset, tambuffer);
    while (leidos > 0)
    {
        bytesleidos = bytesleidos + leidos;
        // Escribe el contenido del buffer en el destino indicado.
        write(1, buffer, leidos);

        //Limpiar buffer
        memset(buffer, 0, tambuffer);
        //Actulizar offset
        offset = offset + tambuffer;
        //Leemos otra vez
        leidos = mi_read_f(ninodo, buffer, offset, tambuffer);
    }

    //lectura del inodo
    if (leer_inodo(ninodo, &inodo)){
        perror("Error con la lectura del inodo.\n");
        return -1;
    }

    fprintf(stderr, "total_bytesleidos: %d\ntamEnBytesLog: %d\n", bytesleidos, inodo.tamEnBytesLog);

    // Desmonta el dispositivo virtual
    if (bumount() == -1)
    {
        perror("leer.c: Error al desmonta el dispositivo virtual.\n");
        return -1;
    }
    return EXIT_SUCCESS;
}
