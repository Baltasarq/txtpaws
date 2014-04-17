txtpaws
=======

Preprocesador, capaz de convertir variables y constantes a un archivo SCE PAWS.
<p>
<a hreg="http://www.caad.es/baltasarq/prys/txtpaws/">Web</a>
<p>
NOTA Importante: la documentación más detallada se encuentra en la [wiki](Documentación-para-txtPAWS).

Web: <a href="http://caad.es/baltasarq/">http://caad.es/baltasarq/</a>

Este fichero debería acompañar al programa txtPAWS

Se trata de poder utilizar nombres de objetos, localidades y mensajes para
referirse a esos objetos en lugar de por sus números de definición en
SCE PAWS.

Para saber más acerca de los términos aquí empleados, y sobre qué es una
aventura conversacional, consúltese la web del CAAD: http://caad.es/

El formato de entrada es SCE PAWS modificado, un fichero .txp

La salida comprende:
        a) un fichero .sce con comentarios según las sustituciones
        b) un fichero .log

El fichero .sce generado no es exactamente igual al original, se recomienda
guardar el fichero .txp

PawseZ:
        http://www.geocities.com/TimesSquare/Fortress/9939/index.html
Paguaglús:
        http://pagina.de/yokiyoki
PAWS y SCE PAWS:
        http://www.yeandle.plus.com/advent/

El formato de SCE, pawcomp y pawint son copyright de Graham Yeandle.

Un fichero SCE generado con txtPAWS o directamente escrito sin utilizar txtPAWS
sirve de entrada para al menos tres compiladores importantes:
        a) El de Graham Yeandle para MS-DOS, pawcomp. Genera un fichero .pdb que
se puede ejecutar con pawint (esta opción no es muy recomendable).
        b) El compilador pawseZ de Zak, que genera código inform a partir de un
archivo .sce. Esta opción es bastante recomendable, ya que inform puede generar
tanto ficheros .z5 (máquina Z) como .ulx (máquina Glulx).
        c) Paguaglús. Genera directamente código .ulx, es decir, código para la
máquina Glulx. (esta opción no es muy recomendable, pues ya no se mantiene).
        d) Superglús. Genera directamente código .ulx, es decir, código para la
máquina Glulx (esta opción es muy recomendable, pues ya no se mantiene) y JavaScript.

Instrucciones
=============

Incluir otros ficheros
----------------------

Para hacer esto, debe incluirse la línea:

##include <nomfich>

Por ejemplo:

##include sysmsg.txp

Incluyendo toda la información necesaria, incluso el path si no está en el mismo
directorio.

Esto permite factorizar el típico SCE dividido en secciones en varios ficheros
más manejables.

Tb., dependiendo de la habilidad del programador, se podrían definir "librerías",
aunque hay que tener presente que las limitaciones de PAWS siguen siendo las
mismas, ##include simplemente incluye un fichero de texto en la línea en la que
se encuentra.

Definición de identificadores
-----------------------------

Al comienzo del programa, antes de cualquier otra sección se puede hacer
cualquier definición, objeto, localidad o mensaje y NO se comprueban.

La sintaxis es:

##define [obj|flg|loc|msg] identificador numero

Por ejemplo:

##define loc loc_inicial 0

En las secciones de objetos, localidades, procesos y mensajes, se pueden definir identificadores:
Se aconseja dejar líneas de separación con ';' (el comentario).

/LTX
/1
...
/21
esta localidad es una cueva fría
##define loc cueva_fria 21
;

Se comprueba que es una localidad ('loc') definida dentro de la localidad, y que
el número se corresponde con la localidad actual, para evitar errores.

Véase el fuente del ticket para más ejemplos.

Definición de reemplazos
------------------------

Se utiliza la siguiente sintaxis:

&&

Para reemplazar:

CARRIED   7; llevas la linterna

por:

CARRIED  &&linterna ; llevas la linterna

... suponiendo que se haya ##definido el objeto 7 como linterna.

Es IMPORTANTE que haya un espacio delimitando el final del identificador.


Nota
=====

Los símbolos '#' y '&' no pueden ser utilizados duplicados si no se trata de una
definición o de un reemplazo. Sobre todo, no se deben utilizar en los textos
de los mensajes y localidades. Por ejemplo, si apareciera:

/12
Whisky J&&B

El programa trataría de encontrar un identificador B para sustituirlo, provocando
un error de ejecución.

Sí es factible:

/12
Whisky J&B


Errores
=======
Ante cualquier error, consultar la línea en el fichero .txp, .txi, .txp.log
y txi.log. El programa genera dos ficheros, el .txi donde están todos los
ficheros incluidos, y el sce, cuando ya se han hecho todas las sustituciones.
Además, genera dos logs, que describen todos los procesos que realiza.

Si el error sigue sin explicación, consultar el SCE y el TXI: ahí se han anotado
todas las sustituciones e inclusiones realizadas.
