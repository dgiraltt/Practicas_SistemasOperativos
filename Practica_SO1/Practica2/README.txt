@author Daniel Giralt Pascual

Mejoras realizadas: Implementación de un método "int cd_avanzado(char **args)" 
con el que se permite el salto a directorios cuyo nombre contiene algún 
espacio, siendo el formato de este nombre una cadena de carácteres que empieze 
y acaba entre comillas dobles ("") o simples (''), o que antes del espacio 
contenga el carácter '\'. Esta mejora se encuentra en todos los niveles en los 
que se implementa el comando interno "internal_cd(char **args)", a partir del 
nivel2 hasta el final my_shell.


Cada fichero de nivel y el final my_shell cuentan con sus respectivas 
cabeceras, donde podemos encontrar las librerías, constantes, estructuras y 
funciones de estos.


Tras la finalización del programa y el test final de este, no se ha encontrado 
ninguna restricción o falla de la ejecución, mostrándose siempre los errores 
de sintaxis de los comandos, los errores de acceso a ficheros, o los propios 
errores internos de funciones como fork(), chdir(), getcwd() o getenv().


Finalemnte, el programa no cuenta con sintaxis específica más allá de las 
variables globales definidas por el enunciado.
