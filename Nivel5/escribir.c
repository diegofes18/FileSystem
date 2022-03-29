#include "ficheros.h"
#include <stdlib.h>

/*
Se encarga de escribir texto en uno o varios inodos haciendo uso de reservar_inodo 
para obtener un numero de inodo, que mostraremos por pantalla y utilizaremos 
como parametro para mi_write_f()
*/

int main(int argc, char *argv[]) {
    //Validamos que la sintaxis sea correcta
    if (argc != 4) {
		    perror("ERROR SINTAXIS: permitir <nombre_dispositivo> <ninodo> <permisos>\n");
		    return -1;
    }

    printf("Longitud del texto: %ld\n", strlen(argv[2]));
    int offsets[5] = {9000, 209000, 30725000, 409605000, 480000000};

    //Montamos el dispositivo
    if (bmount(argv[1]) == -1) {
		    perror("Error en permitir.c --> bmount()\n");
		    return -1;
	  }

    //obtenemos el numero de inodo
    int ninodo=reservar_inodo('f',6);
    if(ninodo==-1){
        perror("Error en reservar_inodo");
        return -1;
    }

    //mostramos por pantalla los ninodos
    for (int i=0; i<(sizeof(offsets)/sizeof(int));i++){
        printf("Nº inodo reservado: %d\n", ninodo);
        printf("offset: %d\n", offsets[i]);

        //usamos ninodo como parámetro para mi_write_f
        int BytesE = mi_write_f(ninodo, argv[2], offsets[i], strlen(argv[2]));

        if (BytesE == -1){
          perror("Error en mi_write_f().\n");
          return -1;
        }

        printf("Bytes escritos: %d\n", BytesE);

        //usamos strlen para calcular la longitud del texto
        int length = strlen(argv[2]);
        char *buffer = malloc(length);
        //Ponemos todas las posiciones de buffer a 0 mediante memset
        if(memset(buffer, 0, length)==NULL){
          return -1;
        }

        //Comprobamos el funcionamiento de las funciones
        int BytesL = mi_read_f(ninodo, buffer, offsets[i], length);

        printf("Bytes leídos: %d\n", BytesL);

        struct STAT stat;

        //Obtenemos los datos del inodo escrito
        if (mi_stat_f(ninodo, &stat)){
            perror("Error en mi_stat_f()\n");
            return -1;
        }

        printf("stat.tamEnBytesLog = %d\n", stat.tamEnBytesLog);
        printf("stat.numBloquesOcupados = %d\n\n", stat.numBloquesOcupados);

        //Si diferentes_inodos = 0 se reserva un solo inodo para todos los offsets
        if (strcmp(argv[3], "0")){
            ninodo=reservar_inodo('f', 6);
            if (ninodo == -1){
              perror("Error en reservar_inodo\n");
              return -1;
          }

        }
    }

    //Desmontamos el dispositivo
    if (bumount() == -1) {
			perror("Error en permitir.c --> bumount()\n");
			return -1;
		}

    return EXIT_SUCCESS;

}

