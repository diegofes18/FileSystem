#include "ficheros.h"
//Le pasaremos por línea de comandos un nº de inodo obtenido con el programa
//anterior (además del nombre del dispositivo). Su funcionamiento tiene que ser
// similar a la función cat de linux, explorando TODO el fichero

int main(int argc, char **argv) {
  //Tamaño del Buffer
  int tbuffer = 1500;
  //Control de sintaxis
  if (argc != 3) {
    perror ("Sintaxis: leer <nombre_dispositivo> <ninodo>\n");
    return -1;
  }
  //Control de bmount
  if (bmount(argv[1]) == -1) {
      perror("Error en leer.c --> Fallo en bmount\n");
      return -1;
    }
  //Inicializamos las variables y limpiamos el buffer.
  int ninodo = atoi(argv[2]);
  int offset = 0;
  int bytes_leidos = 0;
  int valor = 0;
  struct STAT stat;
  unsigned char buffer[tbuffer];
  memset(buffer, 0, tbuffer);

  //Leemos el Inodo indicado por parametro mientras haya valores y permisos.
  while ((valor = mi_read_f(ninodo, buffer, offset, tbuffer)) > 0){
    //Evitamos basura del primer valor
    bytes_leidos += valor;
    write(1, buffer, valor);
    offset += tbuffer;
    memset(buffer, 0, tbuffer);
  }

}

  //printf("Leyendo archivo\n");
  while ((valor = mi_read_f(ninodo, buffer, offset, tam_lectura)) > 0) {
    B_leidos += valor;
    write(1, buffer, valor);
    offset += tam_lectura;
    memset(buffer, 0, tam_lectura);
  }
  fprintf(stderr ,"\nBytes leídos: %d\n", B_leidos);
  mi_stat_f(ninodo, &stat);
  fprintf(stderr ,"Tamaño en bytes lógicos: %d\n", stat.tamEnBytesLog);

  if (bumount() == -1) {
    fprintf(stderr, "Error en leer.c --> %d: %s\n", errno, strerror(errno));
}

    //Hacemos una lectura del Inodo indicado por parametro
    valor = mi_read_f(ninodo, buffer, offset, tambuffer);
    //Mientras hay valores y hay permisos para realizar una lectura 
    while (valor > 0)
    {
        //Evitamos basura
        write(1, buffer, valor);
        //Acumulamos el número de bytes leídos
        nbytes += valor;
        offset += tambuffer;
        //Limpiamos para quitar basura del buffer
        memset(buffer, 0, tambuffer);

    }

    //Mostramos por pantalla los bytes leidos
    fprintf(stderr, "\nBytes leídos: %d\n", nbytes);
    //Cargamos el Inodo en el stat
    mi_stat_f(ninodo, &stat);
    //Mostramos el tamaño en bytes lógicos del stat
    fprintf(stderr, "Tamaño en bytes lógicos: %d\n", stat.tamEnBytesLog);
    //Desmontamos disco
    bumount();
