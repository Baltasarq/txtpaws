// msg.cpp
/*
	Este mod. contiene los mensajes de la app.
	Es posible traducir de esta forma txtPAWS a cualquier
	otro idioma.
*/

#include "msg.h"

const std::string * TxtPAWS::strMsg = strMsgSp;

const std::string TxtPAWS::strMsgSp[] = {
	"",
	"No tiene sentido",
	"Error indefinido",
	"Error interno",
	"Error de sintaxis",
	"Error, mal ordenado",
	"No es posible definir en esta secc.",
	"El num. de mensaje no se corresponde con el objeto",
	"Ya existe un item con ese nombre",
	"No existe ese identificador",
	"Entrada no preparada",
	"Secc. desordenada",
	"Preprocesador: directiva incorrecta",
	"ID duplicado",
	"Sin memoria o error abriendo: '",
   	"se esperaban comillas",
	"sin memoria",
	"demasiado anidamiento ",
	"no puede haber estados en secciones /DEF ni /PRO",
	"CTL debe aparecer tras DEF",
	"VOC debe aparecer tras CTL",
	"STX debe aparecer tras VOC",
	"MTX debe aparecer tras STX",
	"OTX debe aparecer tras MTX",
	"LTX debe aparecer tras OTX",
	"CON debe aparecer tras LTX",
	"OBJ debe aparecer tras CON",
	"'PRO n' debe aparecer tras OBJ o 'PRO (n-1)'",
	"se esperaba palabra de vocabulario",
	"se esperaba un num. de id. asociado a la palabra de vocabulario",
	"num. identificador de palabra de vocabulario incorrecto",
	"se esperaba tipo de palabra de vocabulario",
	", no es un tipo de vocabulario correcto",
	"palabra repetida: ",
	"identificador incorrecto: '",
	"def. no correspondiente a objeto",
	"def. no correspondiente a localidad",
	"def. no correspondiente a mensaje",
	"def. no correspondiente a bandera",
	"def. no correspondiente a grf. de localidad",
	"def. no correspondiente a efecto de sonido",
	"def. no correspondiente a msc. de localidad",
	"def. no correspondiente a efecto grf.",
	"las constantes se definen en DEF o PRO",
	"las macros se definen en DEF o PRO",
	"': secc. no encontrada",
	"valor de macro nulo",
	"num. identificador de sust. nulo",
	"num. de argumentos incorrecto, falta: '",
	"num. de variables excesivo, se supera %9",
	"se esperaba ',' o ')'",
	"demasiadas sustituciones anidadas",
	"se esperaba identificador de grf. o sonido",
	" sin su ",
	"fichero incorrecto: '",
	"anidamiento incorrecto: directivas desparejadas",
	"macro anterior desconocida, imposible continuar: '",
	"No existe macro previa que continuar",
	", se encuentra repetido con '",
	"Sintaxis: \n\ttxtpaws [-sp] [-uk] <nomfich.txp>"
			"\n\ttxtpaws [-MAXNUMLOCS=x] [-INFORM]"
			" [-DEBUG] [-CLEAN] [-QUIET] [-MOTR] <nomfich.txp>"
			"\n\ttxtpaws [-O<nombre_fichero_salida>]"
			" [-I<rutas_ficheros_recursos>] "
			"<nomfich.txp>\n",
	"\nOpciones:"
	"\n\tsp produce mensajes en ISO 639-1: ES"
	"\n\tuk produce mensajes en ISO 639-1: EN"
	"\n\tMAXNUMLOCS "
	"(num. tope de localidades) es 256 por defecto"
	"\n\tINFORM estilo 'inform', desactivado por defecto"
	"\n\tDEBUG incluye el num. de lin. original en el SCE"
	"\n\tCLEAN hace que no se generen los ficheros de log"
	"\n\tQUIET obliga a que solo se generen mensajes de error"
	"\n\tVERBOSE muestra mucha y horrible info "
	"de debug"
	"\n\tI permite proporcionar rutas alternativas\n"
	"\t\t(separadas por comas, ','),\n"
	"\t\tal directorio actual para encontrar los\n"
	"\t\tficheros de recursos (imgs., sonidos ...)\n"
	"\t\t(si los caminos contienen espacios, debe "
	"encerrarse la totalidad entre comillas,\n"
	"\t\tpor ejemplo, \"-I<ruta1>,<ruta2>\")"
	"\t(')"
	"\n\tO/o permite proporcionar un nombre de fichero\n"
	"\t\tde salida alternativo al nombre por defecto,\n"
	"\t\tque es el nombre del fichero de entrada con la "
	"ext.: .sce\t(')"
	"\n\t(') No deben dejarse espacios entre I, o u O y el\n"
	"\t\tnombre del archivo o las rutas."
	"\n\tNo existen incompatibilidades entre las opciones,\n"
	"\t\tni deben especificarse en un orden especial"
	"\n\n",
	"modificador desconocido '",
	"falta el nombre del archivo SCE PAWS",
	"ERROR FATAL: No pudieron procesarse los includes\n",
	"\n\n*** ERROR interno. "
			"Contacte con el autor en baltasarq@gmail.com\n\n",
    "Aviso: ",
    "debe empezar por: '",
    "versales",
};

const std::string TxtPAWS::strMsgUk[] = {
	"",
	"Nonsense",
	"Undefined error",
	"Internal error",
	"Syntax error",
	"Incorrect place",
	"No place for definitions in this section",
	"Number of message does not correspond with object",
	"There is already an item with that name",
	"Identifier not found",
	"Input not ready",
	"Section misplaced",
	"Preprocessor: erroneous directive",
	"Duplicated ID",
	"Not enough memory or error opening ",
	"double quotes expected",
	"no memory",
	"too much nesting ",
	"there cannot be states in sections /DEF nor /PRO",
	"CTL must appear after DEF",
	"VOC must appear after CTL",
	"STX must appear after VOC",
	"MTX must appear after STX",
	"OTX must appear after MTX",
	"LTX must appear after OTX",
	"CON must appear after LTX",
	"OBJ must appear after CON",
	"'PRO n' must appear after OBJ or 'PRO (n-1)'",
	"vocabulary word expected",
	"identifier number associated to vocabulary word expected",
	"invalid identifier number for vocabulary word",
	"vocabulary word's category expected",
	", is not a valid category for vocabulary words",
	"already existing word: ",
	"invalid identifier: ",
	"definition is not an object definition",
	"definition is not a room definition",
	"definition is not a message definition",
	"definition is not a flag definition",
	"definition is not a room's picture definition",
	"definition is not a sound efect definition",
	"definition is not room's music definition",
	"definition is not a graphic efect definition",
	"constants can only be defined in DEF or PRO",
	"macros can only be defined in DEF or PRO",
	"' section not found",
	"void macros not allowed",
	"identifier number for substitution was not given",
	"invalid number of parameters given, missing: '",
	"too many parameteres, beyond %9",
	"',' or ')' expected",
	"too many nested substitutions",
	"identifier of graphic or sound expected",
	" without ",
	"invalid file: '",
	"invalid nesting: unmatching directives",
	"unknown previous macro, cannot continue: '",
	"unexisting previous macro",
	", is repeated with '",
	"Syntax: \n\ttxtpaws [-sp] [-uk] <filename.txp>"
			"\n\ttxtpaws [-MAXNUMLOCS=x] [-INFORM]"
			" [-DEBUG] [-CLEAN] [-QUIET] [-MOTR] <filename.txp>"
			"\n\ttxtpaws [-O<input_file_name>]"
			" [-I<paths_to_resource_files>] "
			"<filename.txp>\n",
	"\nOptions:"
	"\n\tsp outputs all messages in Spanish"
	"\n\tuk outputs all messages in English"
	"\n\tMAXNUMLOCS "
	"(max. number of rooms), is 256 by default"
	"\n\tINFORM 'inform' style, off by default"
	"\n\tDEBUG includes the original line number in the SCE"
	"\n\tCLEAN log files are not generated"
	"\n\tQUIET only error messages are shown"
	"\n\tVERBOSE shows a lot of ugly debugging info"
	"\n\tI include alternative routes\n"
	"\t\t(delimited by commas, ','),\n"
	"\t\tto the current directory, in order to find\n"
	"\t\tresource files (images, sounds ...)\n"
	"\t\t(should your paths have spaces, enclose them with double quotes,\n"
	"\t\tfor example, \"--I<path1>,<path2>\")"
	"\t(')"
	"\n\tO/o specify output file name\n"
	"\t\tby default, the output file name is\n"
	"\t\tthe input file name with the .sce extension\t(')"
	"\n\t(') Please don't include spaces between I, o or O and paths\n"
	"\t\tor file names."
	"\n\tThere are not incompatibilities among options,\n"
	"\t\tnor a special order is needed."
	"\n\n",
	"unknown option '",
	"missing SCE PAWS file name ",
	"FATAL: it was not possible to process include files",
	"\n\n*** Internal ERROR. "
		"Please contact the author in: baltasarq@yahoo.es\n\n",
    "Warning: ",
    "should start by: '",
    "uppercase",
};
