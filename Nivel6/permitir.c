#include "ficheros.h"

int main(int argc, char **argv) {
	//Modificamos los permisos del Inodo con la llamada a mi_chmod_f()
	int ninodo = atoi(argv[2]);
	int permisos = atoi(argv[3]);

    //Validamos que la sintaxis sea correcta
    if (argc != 4) {
		perror("ERROR SINTAXIS: permitir <nombre_dispositivo> <ninodo> <permisos>\n");
		return -1;
    }  

    //Montamos el dispositivo
    if (bmount(argv[1]) == -1) {
		perror("Error en permitir.c --> bmount()\n");
		return -1;
	}

	if (mi_chmod_f(ninodo, permisos)) {
		perror("Error en permitir.c --> mi_chmod_f()\n");
		return -1;
	}

    //Desmontamos el dispositivo
    if (bumount() == -1) {
		perror("Error en permitir.c --> bumount()\n");
		return -1;
	}

	return EXIT_SUCCESS;
    
}
