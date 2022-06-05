//MARC CAÑELLAS, DIEGO BERMEJO, GASTON PANIZZA
#include "directorios.h"


#define DEBUGGER 1

int main(int argc, char const *argv[])
{
    // Comprueba que la sintaxis sea correcta.
    if (argc != 3){
        fprintf(stderr,
                "Error de sintaxis: ./mi_ls <disco></ruta_directorio>\n");
        return EXIT_FAILURE;
    }

    // Monta el disco en el sistema.
    if (bmount(argv[1]) == -1){
        fprintf(stderr, "Error de montaje de disco.\n");
        return EXIT_FAILURE;
    }
    char tipo = '\0';
    char buffer[TAMBUFFER];
    memset(buffer, 0, TAMBUFFER);
    int total;
    if ((total = mi_dir(argv[2], buffer, tipo)) < 0){
        mostrar_error_buscar_entrada(total);
        return EXIT_FAILURE;
    }
    if (total > -1){

#if DEBUGGER
        printf("Total: %d\n", total);
#endif
        printf("Tipo\tModo\tmTime\t\t\tTamaño\tNombre\n");
        printf("----------------------------------------------------------"
               "----------------------\n");
        printf("%s\n", buffer);
    }
    bumount();
    return EXIT_SUCCESS;
}