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
    }

    for (int i=0; i<(sizeof(offset)/sizeof(int));i++){

    }



}

