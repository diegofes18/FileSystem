#include "ficheros.h"

int main(int argc, char **argv) {
    //Validamos que la sintaxis sea correcta
    if (argc != 4) {
		perror("ERROR SINTAXIS: permitir <nombre_dispositivo> <ninodo> <permisos>\n");
		return -1;

    } else {    
        //Montamos el dispositivo
        if (bmount(argv[1]) == -1) {
			perror("Error en permitir.c --> bmount()\n");
			return -1;
		}

		//Modificamos los permisos del Inodo con la llamada a mi_chmod_f()
		unsigned int ninodo = atoi(argv[2]);
		unsigned int permisos = atoi(argv[3]);

		if (mi_chmod_f(ninodo, permisos) == -1) {
			perror("Error en permitir.c --> mi_chmod_f()\n");
			return -1;
		}

        //Desmontamos el dispositivo
        if (bumount() == -1) {
			perror("Error en permitir.c --> bumount()\n");
			return -1;
		}
    }
}
