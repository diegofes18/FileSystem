#include "directorios.h"
#define DEBUG 1

/*
Dada una cadena de caracteres camino (que comience por '/'), separa su contenido en dos
*/
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo){

    //error si se empieza con /
    if (camino[0] != '/'){
        return -1;
    }

    //sumamos 1 para evitar la primera /
    char *rest = strchr((camino + 1), '/');
    strcpy(tipo, "f");

    //si encontramos '/'
    if (rest){

        //copiamos todo en inicial menos el resto
        strncpy(inicial, (camino + 1), (strlen(camino) - strlen(rest) - 1));
        //copiamos el resto en final
        strcpy(final, rest);

        if (final[0] == '/'){
            strcpy(tipo, "d");
        }

    }else {
        strcpy(inicial, (camino + 1));
        strcpy(final, "");
    }
    

    return EXIT_SUCCESS;
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
    char tipo;
    int cant_entradas_inodo, num_entrada_inodo;

    memset(inicial, 0, sizeof(entrada.nombre));
    memset(final, 0, strlen(camino_parcial));
    memset(entrada.nombre, 0, sizeof(entrada.nombre));

    //el directorio es raiz
    if(!strcmp(camino_parcial,"/")){
        struct superbloque SB;
        bread(posSB, &SB);
        *(p_inodo)=SB.posInodoRaiz;
        *(p_entrada)=0;
        return EXIT_SUCCESS;
    } 
    
    if(extraer_camino(camino_parcial, inicial, final, &tipo) == -1) {
        return ERROR_CAMINO_INCORRECTO;
    }

#if DEBUG
    printf("[buscar_entrada()->inicial: %s, final: %s, reservar: %d]\n", inicial,final, reservar);
#endif

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
                   
                    if(tipo == 'd'){
                        
                        if(strcmp(final,"/")==0){

                            entrada.ninodo=reservar_inodo(tipo, permisos);
#if DEBUG
                        printf("[buscar_entrada()->reservado inodo: %d tipo %c con permisos %d para '%s']\n", entrada.ninodo, tipo, permisos, entrada.nombre);
#endif
                        }
                        else{

                            return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                        }
                    }
                    else{

                        entrada.ninodo=reservar_inodo(tipo,permisos);
#if DEBUG
                    printf("[buscar()->reservado inodo: %d tipo %c con permisos %d para '%s']\n", entrada.ninodo, tipo, permisos, entrada.nombre);
#endif
                        
                    }

#if DEBUG
                fprintf(stderr, "[buscar_entrada()->creada entrada: %s, %d] \n", inicial, entrada.ninodo);
#endif

                    if(mi_write_f(*p_inodo_dir,&entrada,dir_inodo.tamEnBytesLog, sizeof(struct entrada)) == -1){
                        
                        if(entrada.ninodo!=-1){
                            liberar_inodo(entrada.ninodo);
#if DEBUG
                        fprintf(stderr, "[buscar_entrada()-> liberado inodo %i, reservado a %s\n", num_entrada_inodo, inicial);
#endif
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

 int mi_creat(const char *camino, unsigned char permisos){     
    unsigned int p_inodo_dir=0;
    unsigned int p_inodo=0;
    unsigned int p_entrada=0;
    int err = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos);
    if (err< 0){
        mostrar_error_buscar_entrada(err);
        return -1;
  }
    return 0;  
 }

 
 int mi_chmod(const char *camino, unsigned char permisos){
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int errEnt = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, permisos);
    if (errEnt<0){
        //perror("Error: Ha ocurrido un error por la entrada.");
        return errEnt;
    }
    errEnt = mi_chmod_f(p_inodo, permisos);
    if (errEnt<0){
        perror("Error: No se han actualizado correctamente los permisos del inodo");
        return errEnt;
    }
    return 0;
 }


/*
Pone el contenido del directorio en un buffer de memoria y devuelve el número de entradas
*/
int mi_dir(const char *camino, char *buffer, char *tipo){

    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int entrada;
    int numEntradas=0;
    struct tm *tm; //ver info: struct tm

    entrada=buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada,0,4);

    if(entrada<0){
        mostrar_error_buscar_entrada(entrada);
        return entrada;
    }

    struct inodo inodo;
    if (leer_inodo(p_inodo, &inodo) == -1){
        return -1;
    }

    if ((inodo.permisos & 4) != 4){
        return -1;
    }

    char tmp[100]; //tiempo
    char tamEnBytes[10]; 

    if (leer_inodo(p_inodo, &inodo) == -1){
            return -1;
        }

        *tipo = inodo.tipo;

        //Buffer de salida
        struct entrada entradas[BLOCKSIZE / sizeof(struct entrada)];
        memset(&entradas, 0, sizeof(struct entrada));

        numEntradas = inodo.tamEnBytesLog / sizeof(struct entrada);

        int offset = 0;
        offset += mi_read_f(p_inodo, entradas, offset, BLOCKSIZE);

        //lectura de las entradas
        for(int i=0;i<numEntradas;i++){

            //lectura del inodo
            if (leer_inodo(entradas[i % (BLOCKSIZE / sizeof(struct entrada))].ninodo, &inodo) == -1){
                return -1;
            }

            if (inodo.tipo == 'd'){
                strcat(buffer, "d");

            }else{
                strcat(buffer, "f");
            }

            strcat(buffer, "\t"); 

            //incorporamos la información sobre los permisos 
            strcat(buffer, ((inodo.permisos & 4) == 4) ? "r" : "-");
            strcat(buffer, ((inodo.permisos & 2) == 2) ? "w" : "-");
            strcat(buffer, ((inodo.permisos & 1) == 1) ? "x" : "-");
            strcat(buffer, "\t");

            //mTime
            tm = localtime(&inodo.mtime);
            sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
            strcat(buffer, tmp);
            strcat(buffer, "\t");

            //Tamaño
            sprintf(tamEnBytes, "%d", inodo.tamEnBytesLog);
            strcat(buffer, tamEnBytes);
            strcat(buffer, "\t");

            //Nombre
            strcat(buffer, entradas[i % (BLOCKSIZE / sizeof(struct entrada))].nombre);
            while ((strlen(buffer) % TAMFILA) != 0){
                strcat(buffer, " ");
            }

            strcat(buffer, "\n"); //Preparamos el string para la siguiente entrada

            if (offset % (BLOCKSIZE / sizeof(struct entrada)) == 0){
                offset += mi_read_f(p_inodo, entradas, offset, BLOCKSIZE);
            }
        }

        return numEntradas;

}

 int mi_stat(const char *camino, struct STAT *p_stat){
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int errEnt = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, p_stat->permisos);
    if (errEnt<0){
        //perror("Error: Ha ocurrido un error por la entrada.");
        return errEnt;
    }
    errEnt = mi_stat_f(p_inodo, p_stat);
    if (errEnt<0){
        //perror("Error: No se ha podido obtener la información del inodo");
        return -1;
    }
    return p_inodo;
 }