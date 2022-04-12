#include "directorios.h"

/*
Dada una cadena de caracteres camino (que comience por '/'), separa su contenido en dos
*/
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo){
    
}



/*
Esta función nos buscará una determinada entrada entre las entradas del inodo correspondiente a su directorio padre. 
Dada una cadena de caracteres y el nº de inodo del directorio padr, donde buscar la entrada en cuestión, obtiene:
-El número de inodo (*p_inodo) al que está asociado el nombre de la entrada buscada.
-El número de entrada  (*p_entrada) dentro del inodo *p_inodo_dir que lo contiene (empezando por 0).
*/
 int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos){

    struct entrada entrada;
    struct inodo dir_inodo;
    char inicial[sizeof(entrada.nombre)];      
    char final[strlen(camino_parcial)];
    char tipo[80];
    int cant_entradas_inodo, num_entrada_inodo;

    memset(inicial, 0, sizeof(entrada.nombre));
    memset(final, 0, strlen(camino_parcial));
    memset(entrada.nombre, 0, sizeof(entrada.nombre));

    //el directorio es raiz
    if(strcmp(camino_parcial,"/")!= 0){
        struct superbloque SB;
        bread(posSB, &SB);
        *(p_inodo)=SB.posInodoRaiz;
        *(p_entrada)=0;
        return EXIT_SUCCESS;
    } 
    
    if(extraer_camino(camino_parcial, inicial, final,&tipo)==-1) {
        return ERROR_CAMINO_INCORRECTO;
    }

    if(leer_inodo(*p_inodo_dir, &dir_inodo) == -1){
         return ERROR_PERMISO_LECTURA;
    }      

    //inicializar el buffer de lectura con 0s
    struct entrada buffer[BLOCKSIZE/sizeof(struct entrada)];
    memset(buffer, 0, (BLOCKSIZE/sizeof(struct entrada))*sizeof(struct entrada));

    //cantidad de entradas que contiene el inodo
    cant_entradas_inodo=dir_inodo.tamEnBytesLog/sizeof(struct entrada);
    //nº de entrada inicial
    num_entrada_inodo=0;
    
    //int offset = 0
    if(cant_entradas_inodo>0){
        if((dir_inodo.permisos&4)!=4){
            return ERROR_PERMISO_LECTURA;
        }

        if (mi_read_f(*p_inodo_dir,&entrada,num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) < 0){
            return ERROR_PERMISO_LECTURA;
        }

        while(num_entrada_inodo<cant_entradas_inodo && strcmp(inicial, entrada.nombre) != 0){
            num_entrada_inodo++;
            if (mi_read_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) < 0){
                return ERROR_PERMISO_LECTURA;
            }
                
        }
        
    }
    //si inicial distinto de entrada.nombre
    if(num_entrada_inodo == cant_entradas_inodo && (inicial != buffer[num_entrada_inodo%(BLOCKSIZE/sizeof(struct entrada))].nombre)){
        
        switch(reservar){

            //modo consulta. Como no existe retornamos error
            case 0:
                return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
                break; 

            //modo escritura
            case 1:
            
                if(dir_inodo.tipo=='f'){
                    return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;

                }
                //comprobar que tiene permiso de escritura 
                if ((dir_inodo.permisos & 2)!=2){
                
                    return ERROR_PERMISO_LECTURA;
                }
                else{
                    strcpy(entrada.nombre,inicial);
                   
                    if(tipo=='d'){
                        
                        if(strcmp(final,"/")==0){

                            entrada.ninodo=reservar_inodo(tipo, permisos);
                        }
                        else{

                            return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                        }
                    }
                    else{

                        entrada.ninodo=reservar_inodo(tipo,permisos);
                        
                    }
                    if(mi_write_f(*p_inodo_dir,&entrada,dir_inodo.tamEnBytesLog, sizeof(struct entrada)) == -1){
                        
                        if(entrada.ninodo!=-1){
                            liberar_inodo(entrada.ninodo);
                        }
                        return -1;
                    }
                }
                
        

        }
    }

    //hemos llegado al final del camino
    if(!strcmp(final,"/")|| !strcmp(final,"")){
        if((num_entrada_inodo<cant_entradas_inodo)&& (reservar==1)){
             //modo escritura y la entrada ya existe
            return ERROR_ENTRADA_YA_EXISTENTE;
        }

        //cortamos la recursividad
        *(p_inodo) = num_entrada_inodo;
        *(p_entrada) = entrada.ninodo;

        return EXIT_SUCCESS;
        
    }else{
        *(p_inodo_dir) = entrada.ninodo;
        return buscar_entrada(final,p_inodo_dir,p_inodo, p_entrada, reservar, permisos);
    }

 }
    
    
/*
Se encarga de mostrar un error por pantalla
*/
void mostrar_error_buscar_entrada(int error) {
   switch (error) {
   case -1: fprintf(stderr, "Error: Camino incorrecto.\n"); break;
   case -2: fprintf(stderr, "Error: Permiso denegado de lectura.\n"); break;
   case -3: fprintf(stderr, "Error: No existe el archivo o el directorio.\n"); break;
   case -4: fprintf(stderr, "Error: No existe algún directorio intermedio.\n"); break;
   case -5: fprintf(stderr, "Error: Permiso denegado de escritura.\n"); break;
   case -6: fprintf(stderr, "Error: El archivo ya existe.\n"); break;
   case -7: fprintf(stderr, "Error: No es un directorio.\n"); break;
   }
}
