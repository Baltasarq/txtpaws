txtpaws
=======

Preprocesador, capaz de convertir variables y constantes a un archivo SCE PAWS.
<p>
<a target="_blank" href="http://www.caad.es/baltasarq/prys/txtpaws/">Web</a>
<p>
NOTA Importante: la documentación detallada se encuentra en la <a href="https://github.com/Baltasarq/txtpaws/wiki/">wiki</a>.

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

PawseZ
Paguaglús
PAWS y SCE PAWS

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

d) <a target="_blank" href="http://www.caad.es/superglus/">Superglús</a>. Genera directamente código .ulx, es decir, código para la
máquina Glulx (esta opción es muy recomendable).

e) <a target="_blank" href="https://github.com/Utodev/ngPAWS/wiki/">ngPAWS</a> Es una versión nueva de Superglús que genera aventuras en html. Esta opción es realmente muy recomendable.

<b>¿Ya estás trabajando con Superglús o ngPAWS? ¡No te preocupes más! txtPAWS está incluido "de serie" en estos sistemas.</b>
