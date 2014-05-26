// txtpaws.cpp
/*
  	Este programa toma un fuente en algo parecido al SCE,
  	pero admitiendo comentarios y nombres para las variables,
  	y genera un fuente SCE PAWS puro
*/

#include "txtpaws.h"

#include <iostream>
#include <iomanip>
#include <algorithm>


// Comentar esta macro para integrar txtPAWS en otro programa,
// o bien para sustituir la interfaz de usuario proporcionada por otra
#define __TEXT__UI

namespace TxtPAWS {

const std::string CNDACTSFILE = "txtpaws-cndacs.ini";
const std::string MARCACONST  = "CONST";
const std::string MARCAMACRO  = "MACRO";
const std::string MARCAVOC    = "VOC";
const std::string MARCAOBJ    = "OBJ";
const std::string MARCAFLG    = "FLG";
const std::string MARCALOC    = "LOC";
const std::string MARCAMSG    = "MSG";
const std::string MARCAPIC    = "PIC";
const std::string MARCASND    = "SND";
const std::string MARCAMSC    = "MSC";
const std::string MARCAGRF    = "GRF";
const std::string nombre      = "txtPAWS";
const std::string version     = " v1.27 20140417";
const std::string opInform    = "INFORM";
const std::string opNumLoc    = "MAXNUMLOCS=";
const std::string opDebug     = "DEBUG";
const std::string opClean     = "CLEAN";
const std::string opQuiet     = "QUIET";
const std::string opVerbose   = "VERBOSE";
const std::string opVerify    = "STYLE";
const std::string opInclude   = "I";
const std::string opOutput    = "O";
const std::string opOutput2   = "o";
const std::string opEspanol   = "sp";
const std::string opIngles    = "uk";

const std::string prepIFDEF   = "#ifdef";
const std::string prepIFNDEF  = "#ifndef";
const std::string prepELSE    = "#else";
const std::string prepENDIF   = "#endif";

const std::string extlog      = ".log";
const std::string extmed      = ".txi";
const std::string seqDef      = "#DEFINE";
const std::string seqRep      = "&&";
const std::string seqInd      = "@";
const std::string prefijoVOC  = "_voc_";
const std::string caracteresExtrasIds = "._áéíóúÁÉÍÓÚñÑüÜÇç";
const std::string AttrMacro   = "ATTR";
	// Por si acaso (ISO-8859-1): Punto, underscore, vocales acentuadas,
	// enye, cedillla, mays y mins.

const std::string include     = "#INCLUDE";
const std::string linSeparadora
                         = "==================================================";
const char delimLiteral  = '"';
const char delimContMacro= '+';
const char delimRutas    = ',';

// Constantes según estilo
const std::string COMENT_INFORM = "!";
const std::string COMENT_PAWS   = ";";
const std::string EXTSAL_INFORM = ".inf";
const std::string EXTSAL_PAWS   = ".sce";

// Pueden cambiar por el "estilo" utilizado
std::string coment               = COMENT_PAWS;	 // El comentario a emplear por defecto
std::string extsal               = EXTSAL_PAWS;  // La ext. a emplear por defecto
bool   modoQuiet                 = false;        // No muestra salida por pantalla
bool   modoVerbose               = false;	     // No muestra tope de info

const size_t MAXNUM = std::numeric_limits<size_t>::max();
const size_t MAXANIDAMIENTOSUSTITUCIONES = 50;
const size_t AttrLength = 64;

// ------------------------------------------------------------ Scanner
const std::string Scanner::DELIMITADORES = " \n\t\r";

Scanner::Scanner(const std::string &nf) : nombre(nf), linea(0), blank(0) {
  	f = new InputFile( nf );

	if ( f == NULL ) {
        if ( modoVerbose ) {
            debOut( "scanner: sin memoria abriendo: " + nombre + "..." );
        }
		throw ErrorInterno( strMsg[OPERRNOMEM] + nf + '\'' );
	}

	if ( modoVerbose ) {
		debOut( "scanner: procesando " + nombre + "..." );
	}
}

Scanner::~Scanner() {
  	delete f;
}

bool Scanner::isNumber(const std::string &str)
{
        std::string::const_iterator it = str.begin();

        for(; it != str.end(); ++it) {
            if ( !std::isdigit( *it ) ) {
                break;
            }
        }

        return ( it == str.end() );
}

unsigned int Scanner::toInt(const std::string &str)
{
    char * end = NULL;
    unsigned long int toret = strtoul( str.c_str(), &end, 10 );

    if ( *end != 0 ) {
        throw ErrorSintaxis( "NaN: '" + str + '\'' );
    }

    return (unsigned int) toret;
}

std::string Scanner::getNomFich(const std::string &nom)
/**
	Devuelve el nombre de fichero sin ext., pero manteniendo el PATH
*/
{
        if ( nom.rfind('.') != std::string::npos )
                return nom.substr( 0, nom.rfind('.') );
        else return nom;
}

void Scanner::pasaEsp(const std::string &lin, size_t &pos, const std::string &delim) {

  	while ( pos < lin.length()
            &&  delim.find(lin[ pos ]) != std::string::npos)
	{
    		++pos;
	}
}

std::string Scanner::getToken(const std::string &lin, size_t &pos, const std::string &delim)
{
  	std::string tok = "";

  	while ( pos < lin.length()
	    &&  delim.find(lin[ pos ]) == std::string::npos)
        {
    		tok = tok + lin[pos];
    		++pos;
  	}

  	return tok;
}

std::string Scanner::getLiteral(const std::string &lin, size_t &pos, char limitador)
/*
        Devuelve un literal, como "hola, mundo", si limitador=='"'
        Si no hay literal, devuelve la cadena nula
*/
{
        std::string toret;

        // Buscar las primeras comillas
        pasaEsp( lin, pos );
        if ( lin[pos] == limitador )
        {
            ++pos;

            // Leerlo todo hasta encontrar un delimitador
            while(lin[pos] != limitador
              &&  pos < lin.length())
            {
                  toret += lin[ pos ];
                  ++pos;
            }

            // Pasar tras el delimitador
            if ( lin[ pos ] == limitador ) {
                ++pos;
            } else throw ErrorSintaxis( strMsg[ WAITINGQUOTES ] );
        }

        return toret;
}

const std::string &Scanner::leeLinea() {
  	size_t primerCaracter;
    blank = 0;

  	do {
  	  // Actualizar lin.
  	  primerCaracter = 0;
  	  f->readLn( buf );
      ++linea;

      // encontrar el primer char que no es un espacio
      pasaEsp( buf, primerCaracter );

      // ln. en blanco ?
      if ( primerCaracter      >  (buf.length() - 1)
        || buf[primerCaracter] == '\n'
        || buf[primerCaracter] == '\r'
        || buf.empty())
      {
            buf = "";
            ++blank;
      }
      else {
          // eliminar lns. que empiezan por comentario
          if (buf.compare(primerCaracter, coment.length(), coment) == 0)
          {
                buf  = "";
                ++blank;     // Con esto no se modifican los nums. de ln.
                             // del fichero de salida, excepto
                             // de los includes (ficheros incluidos)
          }
          else {
            // lin. normal: eliminar retorno de carro
            size_t pos = buf.find('\n');

            // Eliminar 0D, e incluso 0D0A
            if ( pos != std::string::npos )
                  buf.erase( pos );

            // Eliminar 0A, si existe, incluso 0A0D (?)
            pos = buf.find('\r'); // solo por si acaso
            if ( pos != std::string::npos ) {
                  buf.erase( pos );
            }
          }
      }
  	} while( buf.empty()
         && !finEntrada() );

	if ( modoVerbose ) {
        debOut( "scanner: lin '" + buf + "'..." );
	}

  	return buf;
}

// --------------------------------------------------------- Parser

Parser::Parser() : scanSCE(NULL), fout(NULL), log(NULL) {};

Parser::Parser(Scanner *sc, bool cl)
		: scanSCE(sc), fout(NULL), log(NULL), modoLimpio(cl)
{

    // El nombre del log se crea concatenando la ext de log
    // el fichero "fich.txp" es fich.txp.log

    nomFich = scanSCE->devNombreEntrada() + extlog;

	if  ( !enModoLimpio() ) {
        log = new OutputFile( nomFich );

		// Podemos permitirnos que el log falle
		ponLogStr(nombre + version);
		ponLogStr("entrada: " + scanSCE->devNombreEntrada());
		ponLogStr("========================");
	}
}

Parser::~Parser() {
    ponLogStr( "Fin de Parser..." );
  	delete fout;
  	delete log;
}

void Parser::ponLogStr(const std::string &txt) {
	std::ostringstream buffer;

    if (   log != NULL
	 && !( enModoLimpio() ) )
    {
        buffer << std::setw(7) << scanSCE->devNumLinea() << ' '
               << txt          << '\n';

        log->writeLn(buffer.str());
    }
}

void Parser::escribeSalidaParcial(const std::string &x)
{
	if ( fout != NULL ) {
		fout->write( x );
	}
}

void Parser::escribeSalida(const std::string &x)
{
    if ( fout != NULL ) {
  	        fout->writeLn( x );
	}
}

void Parser::escribirLineasEnBlanco(size_t num)
{
    for(; num > 0; --num) {
        escribeSalida( "" );
    }
}

// ------------------------------------------------------------------- parseDefs
const std::string ParseDefs::StrEstado[] = {
	"DEF", "CTL", "VOC", "SYSMSG", "MSG", "OBJMSG",
	"LOC", "CON", "OBJ", "PRO"
};

std::auto_ptr<ParseDefs::ListaCondactos> ParseDefs::Condactos;

void ParseDefs::preparaCondactos()
/**
    Si no se encuentra un fichero de cfg, se meten a mano los principales.
*/
{
    if ( Condactos.get() != NULL ) {
        return;
    }

    try {

    Condactos.reset( new ListaCondactos() );

    Condactos->insert( "AT" );      Condactos->insert( "NOTAT" );
    Condactos->insert( "BACKAT" );  Condactos->insert( "BEEP" );
    Condactos->insert( "BORDER" );  Condactos->insert( "CHARSET" );
    Condactos->insert( "ATGT" );    Condactos->insert( "ATLT" );
    Condactos->insert( "ATGE" );    Condactos->insert( "ATLE" );
    Condactos->insert( "PRESENT" ); Condactos->insert( "ABSENT" );
    Condactos->insert( "WORN" );    Condactos->insert( "NOTWORN" );
    Condactos->insert( "CARRIED" ); Condactos->insert( "NOTCARR" );
    Condactos->insert( "ISAT" );    Condactos->insert( "ISNOTAT" );
    Condactos->insert( "ZERO" );    Condactos->insert( "NOTZERO" );
    Condactos->insert( "BZERO" );   Condactos->insert( "BNOTZERO" );
    Condactos->insert( "OZERO" );   Condactos->insert( "ONOTZERO" );
    Condactos->insert( "EQ" );      Condactos->insert( "NOTEQ" );
    Condactos->insert( "GT" );      Condactos->insert( "LT" );
    Condactos->insert( "GE" );      Condactos->insert( "LE" );
    Condactos->insert( "SAME" );    Condactos->insert( "NOTSAME" );
    Condactos->insert( "ADJECT1" ); Condactos->insert( "ADVERB" );
    Condactos->insert( "PREP" );    Condactos->insert( "NOUN2" );
    Condactos->insert( "ADJECT2" ); Condactos->insert( "CHANCE" );
    Condactos->insert( "TIMEOUT" ); Condactos->insert( "QUIT" );
    Condactos->insert( "DROP" );    Condactos->insert( "WEAR" );
    Condactos->insert( "REMOVE" );  Condactos->insert( "CREATE" );
    Condactos->insert( "DESTROY" ); Condactos->insert( "SWAP" );
    Condactos->insert( "PLACE" );   Condactos->insert( "PUTO" );
    Condactos->insert( "PUTIN" );   Condactos->insert( "TAKEOUT" );
    Condactos->insert( "DROPALL" ); Condactos->insert( "AUTOG" );
    Condactos->insert( "AUTOD" );   Condactos->insert( "AUTOW" );
    Condactos->insert( "AUTOR" );   Condactos->insert( "AUTOP" );
    Condactos->insert( "AUTOT" );   Condactos->insert( "COPYOO" );
    Condactos->insert( "COPYOF" );  Condactos->insert( "COPYFO" );
    Condactos->insert( "WHATO" );   Condactos->insert( "WEIGH" );
    Condactos->insert( "SET" );     Condactos->insert( "CLEAR" );
    Condactos->insert( "PLUS" );    Condactos->insert( "MINUS" );
    Condactos->insert( "LET" );     Condactos->insert( "ADD" );
    Condactos->insert( "SUB" );     Condactos->insert( "COPYFF" );
    Condactos->insert( "RANDOM" );  Condactos->insert( "MOVE" );
    Condactos->insert( "GOTO" );    Condactos->insert( "WEIGHT" );
    Condactos->insert( "ABILITY" ); Condactos->insert( "MODE" );
    Condactos->insert( "PROMPT" );  Condactos->insert( "TIME" );
    Condactos->insert( "PRINT" );   Condactos->insert( "TURNS" );
    Condactos->insert( "SCORE" );   Condactos->insert( "CLS" );
    Condactos->insert( "NEWLINE" ); Condactos->insert( "MES" );
    Condactos->insert( "MESSAGE" ); Condactos->insert( "SYSMESS" );
    Condactos->insert( "LISTOBJ" ); Condactos->insert( "LISTAT" );
    Condactos->insert( "INVEN" );   Condactos->insert( "DESC" );
    Condactos->insert( "END" );     Condactos->insert( "DONE" );
    Condactos->insert( "NOTDONE" ); Condactos->insert( "OK" );
    Condactos->insert( "SAVE" );    Condactos->insert( "LOAD" );
    Condactos->insert( "RAMSAVE" ); Condactos->insert( "RAMLOAD" );
    Condactos->insert( "ANYKEY" );  Condactos->insert( "PAUSE" );
    Condactos->insert( "PARSE" );   Condactos->insert( "NEWTEXT" );
    Condactos->insert( "BELL" );	Condactos->insert( "PROCESS" );
    Condactos->insert( "DOALL" );   Condactos->insert( "RESET" );
    Condactos->insert( "ISMUSIC" ); Condactos->insert( "ISNOTMUSIC" );
    Condactos->insert( "ISSOUND" ); Condactos->insert( "ISNOTSOUND" );
    Condactos->insert( "OBJFOUND" );Condactos->insert( "OBJNOTFOUND" );
    Condactos->insert( "ZONE" );    Condactos->insert( "RESTART" );
    Condactos->insert( "EXTERN" );  Condactos->insert( "GET" );
    Condactos->insert( "GETO" );    Condactos->insert( "GRAPHIC" );
    Condactos->insert( "INK" );     Condactos->insert( "INPUT" );
    Condactos->insert( "LINE" );    Condactos->insert( "PAPER" );
    Condactos->insert( "PICTURE" ); Condactos->insert( "PRINTAT" );
    Condactos->insert( "PROTECT" ); Condactos->insert( "SAVEAT" );
    Condactos->insert( "SET" );     Condactos->insert( "OSET" );
    Condactos->insert( "OCLEAR" );  Condactos->insert( "DEBUG" );
    Condactos->insert( "WRITE" );   Condactos->insert( "TRANSCRIPT" );
    Condactos->insert( "WRITELN" ); Condactos->insert( "VERSION" );
    Condactos->insert( "BCLEAR" );  Condactos->insert( "BNEG" );
    Condactos->insert( "BSET" );    Condactos->insert( "CLEAREXIT" );
    Condactos->insert( "DIV" );     Condactos->insert( "EXITS" );
    Condactos->insert( "GETEXIT" ); Condactos->insert( "MOD" );
    Condactos->insert( "MUL" );     Condactos->insert( "NOP" );
    Condactos->insert( "OBJAT" );   Condactos->insert( "EXITS" );
    Condactos->insert( "ONEG" );    Condactos->insert( "RANDOMX" );
    Condactos->insert( "SETEXIT" ); Condactos->insert( "TEXTPIC" );
    Condactos->insert( "WHATOX" );  Condactos->insert( "WHATOX2" );
    Condactos->insert( "GETKEY" );  Condactos->insert( "MESS" );

    } catch(...) {
        throw ErrorInterno( "Error CndActs" );
    }
}

std::string ParseDefs::chk()
{
    MapaSust::ListaIds lIds;
    MapaSust::ListaIds::const_iterator it;
    std::set<std::string> setIdsMays;
    std::string s;
    std::string avisos;
    std::string valor;
    size_t numLinea;

    s.reserve( Parser::MaxId );

    // Comprobar la TDS contra los condactos
    // y preparar lIds para buscar vocabulario (toUpperCase)
    ts->devTodosIds( lIds );
    for(it = lIds.begin(); it != lIds.end(); ++it ) {
        MapaSust * mapa = ts->buscaEnTablas( **it );
        s = StringMan::mays( **it );
        setIdsMays.insert( s );

        if ( Condactos->find( s ) != Condactos->end() ) {
            if ( mapa != NULL
              && !mapa->esUltimaEntrada() )
            {
                valor = mapa->getValorEntradaActString();
                numLinea = mapa->getNumLineaEntradaAct();
            } else {
                numLinea = Error::MaxLineas;
                valor = "N/A";
            }

            throw ErrorIdDuplicado( "CondAct.: "
                                       + **it
                                       + ", id.: " + s + "="
                                       + valor,
                                    numLinea
            );
        }
    }

    // Comprobar toda la TDS contra el vocabulario
    ts->getCtrVocs()->iraPrimeraEntrada();
    while ( !ts->getCtrVocs()->esUltimaEntrada() ) {
        s = ts->getCtrVocs()->getEtqEntradaAct();
        s.erase( 0, prefijoVOC.length() );
        StringMan::maysCnvt( s );

        if ( setIdsMays.find( s ) != setIdsMays.end() ) {
                valor = ts->getCtrVocs()->getValorEntradaActString();
                numLinea = ts->getCtrVocs()->getNumLineaEntradaAct();

                throw ErrorIdDuplicado( "Voc.: "
                            + ts->getCtrVocs()->getEtqEntradaAct()
                            + ", id.: " + s + "="
                            + valor,
                        numLinea
                );
        }

        ts->getCtrVocs()->iraSigEntrada();
    }

    // Comprobar los warnings de los ids
    TDS::ListaTablas::const_iterator itTablas = ts->getTablas().begin();
    for(; itTablas != ts->getTablas().end(); ++itTablas ) {
        avisos += itTablas->second->getAvisos();
    }

    return avisos;
}

ParseDefs::ParseDefs(Scanner *sc,
		     size_t numLocs, bool dbg, bool cl, bool v,
		     const std::string &nomFichS)
	: Parser(sc, cl), numItem(0), numLocalidades(numLocs-1),
      modoDebug(dbg), verify( v )
{
	// Determinar el nombre del archivo de salida
	if ( nomFichS.empty() )
        	nomFich = Scanner::getNomFich( scanSCE->devNombreEntrada() )
		          + extsal
		;
	else    nomFich = nomFichS;

  	fout = new OutputFile( nomFich );

	if ( fout != NULL
	  && fout->isOpen() )
	{
		ponLogStr( "salida: " + nomFich );
		ponLogStr( "==========================" );

		ts = new TDS( numLocs, v );

		if ( ts == NULL ) {
			throw ErrorInterno( strMsg[NOMEM] + "(TDS)" );
		}

		cambiaEstado( DEF );
		cambiaEstadoPreProc( PROCESA );
	}
	else {
		throw ErrorInterno( strMsg[OPERRNOMEM] + nomFich +'\'' );
	}

	if ( modoVerbose ) {
		debOut( "parser: creando " + nomFich + "..." );
	}
}

inline
ParseDefs::TipoEstado ParseDefs::cambiaEstado(ParseDefs::TipoEstado e)
{
	if ( modoVerbose ) {
		debOut( "parser: cambiar estado " + StrEstado[e] );
	}

  	return ( estado = e );
}

inline
ParseDefs::TipoEstadoPreProc ParseDefs::cambiaEstadoPreProc(ParseDefs::TipoEstadoPreProc e)
{
	estadoPreProc.push( e );
  	return e;
}

inline
ParseDefs::TipoEstadoPreProc ParseDefs::getEstadoPreProc() const
{
  	return estadoPreProc.top();
}

inline
void ParseDefs::quitaEstadoPreProc() {
	if ( estadoPreProc.size() > 0 )
  		estadoPreProc.pop();
	else  	throw ErrorPreProc(
			strMsg[NESTING] + '(' + prepENDIF + ')' )
		;
}

bool ParseDefs::cambiaEstado(const std::string &lin)
{
  	bool toret = true;

    // Controlar los estados del AF
  	if (lin[0] == '/'
         && isdigit(lin[1]))
        {
                if (devEstado() == DEF
                 || devEstado() == PRO)
		{
                        throw SeccError( strMsg[CANTHAVESTATES] )
			;
		}

    		sscanf(lin.c_str() + 1, "%u", &numItem);
                ponLogStr("Comprobando Item:");
                ponLogInt(numItem);
        }
  	else
  	if (lin.compare(0, 4, "/CTL") == 0) {
                if (devEstado() != DEF) {
                        throw SeccError( strMsg[CTLORD] );
		}

    		cambiaEstado(CTL);
    		ponLogStr("Secc. Control ...");
  	}
  	else
  	if (lin.compare(0, 4, "/VOC") == 0) {
                if (devEstado() != CTL) {
                        throw SeccError( strMsg[VOCORD] );
		}

    		cambiaEstado(VOC);
    		ponLogStr("Secc. Vocabulario ...");
  	}
  	else
  	if (lin.compare(0, 4, "/STX") == 0) {
                if (devEstado() != VOC) {
                        throw SeccError( strMsg[STXORD] );
		}

    		cambiaEstado(SYSMSG);
    		ponLogStr("Secc. Mensajes de Sistema ...");
  	}
  	else
  	if (lin.compare(0, 4, "/MTX") == 0) {
                if (devEstado() != SYSMSG) {
                        throw SeccError( strMsg[MTXORD] );
		}

    		cambiaEstado(MSG);
    		ponLogStr("Secc. Mensajes de usuario ...");
  	}
  	else
  	if (lin.compare(0, 4, "/OTX") == 0) {
                if (devEstado() != MSG) {
                        throw SeccError( strMsg[OTXORD] );
		}

    		cambiaEstado(OBJMSG);
    		ponLogStr("Secc. Mensajes de Objeto ...");
  	}
  	else
  	if (lin.compare(0, 4, "/LTX") == 0) {
                if (devEstado() != OBJMSG) {
                        throw SeccError( strMsg[LTXORD] );
		}

    		cambiaEstado(LOC);
    		ponLogStr("Secc. Localidades ...");
  	}
  	else
  	if (lin.compare(0, 4, "/CON") == 0) {
                if (devEstado() != LOC) {
                        throw SeccError( strMsg[CONORD] );
		}

    		cambiaEstado(CON);
    		ponLogStr("Secc. Conexiones ...");
  	}
  	else
  	if (lin.compare(0, 4, "/OBJ") == 0) {
                if (devEstado() != CON) {
                        throw SeccError( strMsg[OBJORD] );
		}

    		cambiaEstado(OBJ);
    		ponLogStr("Secc. de Objetos ...");
  	}
  	else
        if (lin.compare(0, 4, "/PRO") == 0) {
        	if (devEstado() != PRO
                 && devEstado() != OBJ)
		{
                	throw SeccError( strMsg[PROORD] );
		}

    	  	ponLogStr("Tabla de procesos ...");
    	  	cambiaEstado(PRO);
        }
    	else toret = false;

  	return toret;
}

void ParseDefs::ponLogInt(int x)
{
  	std::ostringstream buffer;

    if (log != NULL) {
        buffer << ' ' << x << ' ';
        ponLogStr(buffer.str());
	}
}

void ParseDefs::procesaVocabulario(const std::string &lin)
/*
	Este mth es llamado cuando nos encontrarmos en
	la secc. de vocabulario, para procesar las entradas.

	Todas las entradas son insertadas en las tablas de identificadores,
	con el prefijo __voc_ (constante de cadena prefijoVOC).

	El vocabulario es de la siguiente forma:
	coger           23 verb
	gato            21 noun
	lo               1 pronoun
	verde           25 adjective
	tranquilamente  45 adverb
	para            32 preposition
	y                9 conjunction
*/
{
	std::string nombreGuardado;
	std::string nombre;
	std::string categoria;
	std::string numeroId;
	int numId;
	size_t pos = 0;
	const std::string categorias = "NOUN VERB ADJECTIVE ADVERB "
                                   "PRONOUN PREPOSITION CONJUNCTION"
	;

	// Es una cambio de control?
	Scanner::pasaEsp( lin, pos );
	if ( lin[ pos ] == '/' ) {
		return;
	}

	// Tomar el nombre
	nombre = Scanner::getToken( lin, pos );
	StringMan::maysCnvt( nombre );
	if ( nombre.empty() ) {
		throw ErrorSintaxis( strMsg[ WAITINGWORD ] );
	}

	// Tomar el num.
	Scanner::pasaEsp( lin, pos );
	numeroId  = Scanner::getToken( lin, pos );

	if ( numeroId.empty() )
	     throw ErrorSintaxis( strMsg[WAITINGNUMWORD] );
	else numId = std::atoi( numeroId.c_str() );

	if ( numId < 0 ) {
	     throw  ErrorSintaxis( strMsg[WORDIDENTIFIERINVALID] );
	}

	// Tomar el tipo
	Scanner::pasaEsp( lin, pos );
	categoria = Scanner::getToken( lin, pos );
	StringMan::maysCnvt( categoria );
	if (categoria.empty()) {
		throw ErrorSintaxis( strMsg[WAITINGWORDCAT] );
	}

	if ( categorias.find( categoria ) == std::string::npos ) {
		throw ErrorSintaxis( categoria + strMsg[INVALIDWORDCAT] );
	}

	// Preparar para ser almacenado
	nombreGuardado = prefijoVOC + nombre;

	// Comprobar no existente en vocabulario
	if ( !( ts->getCtrVocs()->existeEntrada( nombreGuardado ) ) ) {

		// Meterlo en el vocabulario
		ts->insrtVoc( nombreGuardado, numId, scanSCE->devNumLinea() );

		ponLogStr("almacenando palabra vocabulario: " + nombreGuardado);
		if ( modoVerbose ) {
			std::ostringstream fmt;

			fmt << "Parser: " << nombreGuardado << " ... "
			    << numId
			    << " ... ok"
		        ;
			debOut( fmt.str() );
		}
	}
	else throw MsgYaExisteError( strMsg[EXISTINGWORD]+ nombre );
}

void ParseDefs::reemplaza(std::string &lin)
{
    // Si estamos en la secc. de vocabulario, entonces
    // hay que procesarlo
    if ( devEstado() == VOC ) {
        procesaVocabulario( lin );
    }  else {
        size_t pos = 0;
        std::string tok;

        // Tomar el primer token
        Scanner::pasaEsp( lin, pos );
        tok = Scanner::getToken( lin, pos );
        StringMan::maysCnvt( tok );

        // Ver si tiene un "#" o "##"
        if ( tok[0]=='#'
          && tok[1]=='#' )
        {
            tok.erase( 0, 1 );   // Eliminar la primera '#'

            if ( modoVerbose ) {
                debOut( "Parser: capturado '##' ..." );
            }
        }

        // Coger la def. o reemplazar
        if ( tok == seqDef ) {
                    ponLogStr( "Analizando def..." );

            if ( modoVerbose ) {
                debOut( "Parser: antes de tomar def: '"
                         + lin + '\'');
            }

                tomaDef( lin );
            lin.insert( 0, coment );
        }
        else {
            hazRepl( lin );
        }
    }
}

void ParseDefs::tomaDef(const std::string &l)
/* Sintaxis ##define [const|macro|obj|msg|flg|loc|pic|grf|msc] nombre num|str */
{
        std::string tok;
        std::string secc;
        std::string str;
    	size_t num;
        MapaSust *ctr;
        size_t pos = 0;
        std::string lin(l);

        // Tomar el primer token
        Scanner::pasaEsp( lin, pos );
        tok = Scanner::getToken( lin, pos );

        // Obtener la secc. de la def.
        Scanner::pasaEsp(lin, pos);
        secc = scanSCE->getToken( lin, pos );
        StringMan::maysCnvt(secc);

        // Obtener el identificador
        Scanner::pasaEsp( lin, pos );
        tok = scanSCE->getToken( lin, pos );
        if ( !compruebaId( tok ) ) {
            throw ErrorSintaxis( strMsg[INVALIDID] + tok + '\'');
        }

        // Pasar el '=', si lo hay
        Scanner::pasaEsp(lin, pos);
        if (lin[pos] == '=') {
                ++pos;
        }

        // Comprobar las secciones
        ctr = getCtr( secc );

        if ( ctr == ts->getCtrMacros() ) {
            macroAnterior = tok;
        }

        // No hay errores: aceptar la nueva def.
        if (!(ts->existeItem(tok))) {
                if (dynamic_cast<MapaNombreSust<std::string>*>(ctr) != NULL){

                  // Tomar el valor de la etiqueta
                  str = Scanner::getLiteral(lin, pos, delimLiteral);
                  if ( str.empty() ) {
                        throw ErrorSintaxis( strMsg[VOIDMACRO] );
                  }

                  // Insertar la etiqueta y su valor
                  ((MapaNombreSust<std::string>*) ctr)->
                                insrtEntrada(str, tok, scanSCE->devNumLinea());

                  // Para que conste
                  ponLogStr("Token " + tok + " = " +
                               (((MapaNombreSust<std::string>*) ctr)
                                          ->buscaEntrada(tok))
                           )
                  ;
                }
                else  {
                  // Tomar el valor de la etiqueta
                  Scanner::pasaEsp(lin, pos);
                  str    = scanSCE->getToken(lin, pos);
                  if (str.empty()) {
                    if ( secc != MARCACONST )
                            throw ErrorSintaxis( strMsg[VOIDID] );
                    else    str = "0";
                  }
                  num = std::atoi(str.c_str());

                  // Debe ser igual al num. de item actual
                  // (si no estamos en DEF o en PRO)
                  if ( devEstado() != DEF
                    && devEstado() != PRO
                    && num         != numItem )
                  {
                        throw NumMsgColocaError(str);
                  }

                  // Finalizadas las comrpobaciones, tomar def
                  if (dynamic_cast<MapaNombreNumDesplz *>(ctr) != NULL){
                        // Insertar la etiqueta y su valor
                       ((MapaNombreNumDesplz*) ctr)->
		                 insrtEntrada(num, tok, scanSCE->devNumLinea());

                        // Para que conste
                        num = ((MapaNombreNumDesplz*) ctr)->buscaEntrada(tok);
                  }
                  else {
                        // Insertar la etiqueta y su valor
                        ((MapaNombreSust<size_t>*) ctr)
                                ->insrtEntrada(num, tok, scanSCE->devNumLinea());

                        // Para que conste
                        num = ((MapaNombreSust<size_t>*) ctr)->buscaEntrada(tok);
                  }

                  // Para que conste
                  std::ostringstream buffer;
                  buffer << "Token " << tok << " = " << num;
                  ponLogStr( buffer.str() );
                }
        }
        else throw MsgYaExisteError(tok);
}


bool ParseDefs::sust(std::string &toret, const std::string &etq, const std::string &txt, size_t pos)
/**
	Se le pasa la lin., la etiqueta a sustituir, el texto,
	y la pos. en la que hay que sustituir
*/
{
	bool seHizo = true;

	// Encontrar el lugar
	if ( pos == std::string::npos ) {
		pos = toret.find( etq );
	}

	// Sustituir
	if ( pos != std::string::npos ) {
		toret.erase( pos, etq.length() );
		toret.insert( pos, txt );
	}
	else seHizo = false;

	return seHizo;
}

void ParseDefs::hazMacro(std::string &toret, size_t pos, std::string &macro)
/*
	Va sustituyendo las variables %1, %2 ... etc
*/
{
	int posPar;
	std::string txt;
	int numVble = 1;
	std::ostringstream buffer;
	std::string delimitadores =
		Scanner::DELIMITADORES
		+ coment + ',' + '(' + ')'
	;

	Scanner::pasaEsp(toret, pos);
	posPar = pos;

	if ( modoVerbose ) {
		debOut( "Parser: antes de hacer macro ..." );
	}

	if ( toret[posPar] == '(' ) {
		++pos;
	}

    // Pasar a la siguiente
    Scanner::pasaEsp( toret, pos, delimitadores );
    txt = Scanner::getToken( toret, pos, delimitadores );

    while( !txt.empty() ) {
        // Sustituir %n por el texto
        buffer.str( "" );
        buffer << '%' << numVble;

        // Hacer el primer cambio, que debe de existir
        if  (!( sust( macro, buffer.str(), txt) ) ) {
            throw ErrorSintaxis( strMsg[INVALIDPARAM]
                        + buffer.str() + '\''
                        )
            ;
        }
        // Hacer el resto de sustituciones
        while ( sust(macro, buffer.str(), txt ) );

        ++numVble;
        if (numVble > 9) {
            throw ErrorSintaxis( strMsg[TOOMANYPARAMS] );
        }

        // Llegar hasta la ',' o el ')'
        Scanner::pasaEsp( toret, pos );
        if ( toret[pos] == ','
          || toret[ pos ] ==  ')' )
        {
            ++pos;
        }

        // Pasar a la siguiente
        Scanner::pasaEsp( toret, pos, delimitadores );
        txt = Scanner::getToken( toret, pos, delimitadores );
    }

    // Borrar el indicador de argumentos
    toret.erase( posPar );

    // Comprobar el resto de los identificadores
    for( ; numVble < 10; ++numVble) {
            buffer.str("");

            buffer << '%' << numVble;

            if (macro.find(buffer.str()) != std::string::npos) {
                    throw ErrorSintaxis( strMsg[INVALIDPARAM]
                                        + buffer.str() + '\'' )
                    ;
            }
    }
}

/*
template <typename T>
void printVector(const std::vector<T> &v)
{
     for(size_t i = 0; i < v.size(); ++i) {
        std::cout << v[ i ] << ' ';
    }

    std::cout << std::endl;
}
*/

void ParseDefs::hazAttr(std::string &toret, size_t pos, std::string &txt)
{
    MapaSust * m;
    std::string token;
    std::vector<size_t> params;
    size_t oldPos = pos;
    size_t binPos = 0;
    static std::string delim = Scanner::DELIMITADORES + ",()";
    std::vector<size_t>::iterator newEnd;
    std::vector<size_t>::iterator it;

    // Pass the ATTR
    Scanner::pasaEsp( toret, pos, delim );
    token = Scanner::getToken( toret, pos, delim );

    if ( token != AttrMacro ) {
        throw ErrorSintaxis( AttrMacro + '?' );
    }

    // Get to the parameter list
    Scanner::pasaEsp( toret, pos, delim );

    if ( toret[ pos ] != COMENT_PAWS[ 0 ] ) {
        token = Scanner::getToken( toret, pos, delim );

        // Get parameters
        while( !token.empty() ) {
            StringMan::trimCnvt( token );

            // Process parameter
            if ( Scanner::isNumber( token ) ) {
                params.push_back( Scanner::toInt( token.c_str() ) );
            } else {
                // Last resource, substitute on the fly
                if ( ( m = ( ts->buscaEnTablas( token ) ) ) != NULL ) {
                    params.push_back( Scanner::toInt( m->buscaEntradaStr( token ).c_str() ) );
                } else {
                    throw ErrorSintaxis( strMsg[ INVALIDID ] + token );
                }
            }

            // Get next parameter
            Scanner::pasaEsp( toret, pos, delim );
            if ( toret[ pos ] == COMENT_PAWS[ 0 ] ) {
                break;
            }
            token = Scanner::getToken( toret, pos, delim );
        }
    }

    // Remove duplicated
    std::sort( params.begin(), params.end() );
    newEnd = std::unique( params.begin(), params.end() );
    params.erase( newEnd, params.end() );

    // Generate binary number
    for(binPos = 0, it = params.begin(); it != params.end(); ++it, ++binPos) {
        if ( *it >= AttrLength ) {
            throw ErrorSintaxis( strMsg[ TOOMANYPARAMS ] );
        }

        for(size_t i = binPos; i < *it; ++i, ++binPos) {
            txt.push_back( '0' );
        }

        txt.push_back( '1' );
    }

    // Fill remaining positions with zeroes
    for(; binPos < AttrLength; ++binPos) {
        txt.push_back( '0' );
    }

    // Insert a space in the middle
    txt.insert( AttrLength / 2, " " );

    // Erase parameters
    toret.erase( oldPos );
}

void ParseDefs::hazRepl(std::string &toret)
{
	size_t anidamiento = 0;
	int attrPos = -1;
	bool huboCambio;
    bool habiaArroba;
    bool habiaBarra;
	std::string token;
	std::string etq;
	std::string txt;
	std::string comment;
	std::string toAdd;
	size_t pos;
	size_t tamSeqRep = seqRep.length();
	size_t tamSeqInd = seqInd.length();
	MapaSust *m;
	std::string delimitadores = Scanner::DELIMITADORES + coment + "(),";

	do {
		huboCambio  = false;
		pos         = 0;

		while(pos < toret.length()
		   && anidamiento < MAXANIDAMIENTOSUSTITUCIONES)
		{
			Scanner::pasaEsp(toret, pos, delimitadores);
			token = Scanner::getToken(toret, pos, delimitadores);
            habiaArroba = habiaBarra = false;

            if (!token.empty())
            {
                etq = token;

                // Ver si hay '/' antes, y eliminarlo, si existe
                if (etq[0] == '/')
                {
                        etq.erase(0, 1);
                        habiaBarra = true;
                }

                // Ver si hay '@' antes, y eliminarlo, si existe
                if (etq.length() >= tamSeqRep
                 && etq.compare(0, tamSeqInd, seqInd) == 0)
                {
                        etq = token.substr(tamSeqInd);
                        habiaArroba = true;
                }

                // Ver si hay '&&' antes, y eliminarlo, si existe
                if (etq.length() >= tamSeqRep
                 && etq.compare(0, tamSeqRep, seqRep) == 0)
                {
                        etq.erase(0, tamSeqRep);
                }

                // Buscar la etiqueta, y si existe, sustituir
                if ((m = (ts->buscaEnTablas(etq))) != NULL) {
                        huboCambio = true;
                        txt        = m->buscaEntradaStr(etq);

                        if ( modoVerbose ) {
                            debOut( "Parser: reemplazo '"
                                + toret +'\''
                            );

                            debOut( "Parser: sust( "
                                    + etq + '/'
                                    + txt + ' '
                                    + ')'
                                   )
                            ;
                        }

                        // reponer la barra y la arroba
                        if (habiaArroba) {
                            txt.insert( 0, seqInd );
                        }
                        if (habiaBarra) {
                            txt.insert( 0, 1, '/' );
                        }

                        // Si es una macro, hay que sustituir
                        // variables en el texto
                        comment = txt;
                        if ( m->getID() == MARCAMACRO ) {
                            if ( etq != AttrMacro ) {
                                hazMacro( toret, pos, txt );
                                comment = "macro";
                            } else {
                                attrPos = pos - AttrMacro.length();
                                pos += AttrMacro.length();
                                huboCambio = false;
                                continue;
                            }
                        }

                        // Hacer el cambio
                        pos -= token.length();
                        sust( toret, token, txt, pos );

                        // Para que conste
                        ++anidamiento;
                        toAdd += ' ' + coment + " sust(" + etq + '/' + comment + ')';
                        ponLogStr("Reemplazando: " + etq + '/' + txt);
                }
                else ++pos;
            }
        }

        if ( attrPos >= 0 ) {
            txt = "";
            hazAttr( toret, attrPos, txt );
            sust( toret, token, txt, attrPos );
            toAdd += ' ' + coment + " sust(" + etq + '/' + comment + ')';
            ponLogStr("Reemplazando: " + AttrMacro + '/' + txt);
            huboCambio = false;
        }
	} while( huboCambio
	      && anidamiento < MAXANIDAMIENTOSUSTITUCIONES );

	if ( anidamiento >= MAXANIDAMIENTOSUSTITUCIONES ) {
		throw ErrorSemantico( strMsg[TOOMANYNESTEDSUST] );
	}

	toret += toAdd;
}

void ParseDefs::procIdentificadoresControl(const std::string &lin, size_t &pos)
/**
 * Se trata de identificadores del estilo siguiente:
 * 		/1 = casa( casa.jpg, casa.mod )
**/
{
        std::string tok;
        MapaNombreSust<size_t> * cont(NULL);
        size_t antPos;
        std::string delimitadores = Scanner::DELIMITADORES + '(';

        // Realmente, ¿hay algo que procesar?
        Scanner::pasaEsp( lin, pos );
        antPos = pos;

        if (lin[pos] == '=') {
          // Encontrar el contenedor donde meter el identificador
          if ( devEstado() == LOC ) {
                  cont = ts->getCtrLocs();
          }
          else
          if ( devEstado() == OBJMSG ) {
                  cont = ts->getCtrObjs();
          }
          else
          if ( devEstado() == OBJ ) {
                  cont = ts->getCtrObjs();
          }
          else
          if ( devEstado() == MSG
            || devEstado() == SYSMSG )
          {
                  cont = ts->getCtrMsgs();
          }
          else throw ErrorSemantico( strMsg[CANTDEFINE] + ": " + StrEstado[ devEstado() ] );

          // Leer el identificador
          ++pos;
          Scanner::pasaEsp(lin, pos);
          tok = Scanner::getToken(lin, pos, delimitadores);
          if (!compruebaId(tok)) {
            throw ErrorSintaxis( strMsg[INVALIDID] + ": '" + tok + '\'' );
          }

          // Meter la entrada
          cont->insrtEntrada( getNumItem(), tok, scanSCE->devNumLinea() );

          // Coger la msc. y la imagen, si los hay
          if (devEstado() == LOC) {
	  	  std::string delimitadores = Scanner::DELIMITADORES + ',' + ')';

                Scanner::pasaEsp(lin, pos);

                if (lin[pos] == '(') {
                    ++pos;
                    Scanner::pasaEsp(lin, pos);
                	tok = Scanner::getToken(lin, pos, delimitadores);
                    if (!compruebaId(tok)) {
                        throw ErrorSintaxis(
                            strMsg[INVALIDID] + tok + '\''
                        );
                    }

                	while (!tok.empty())
                    {
                        cont = (MapaNombreSust<size_t> *)
                                     FichRecursos::tablaPorExt(tok, ts)
                        ;

                        if (cont != NULL) {
                            cont->insrtEntrada(getNumItem(),
                                        tok, scanSCE->devNumLinea())
                            ;
                        }
                        else throw ErrorSintaxis(
                                strMsg[WAITINGSNDGRFID]
                                + ": '" + tok + '\''
                        );

                        Scanner::pasaEsp(lin, pos);
                        if (lin[pos] != ')') {
                            ++pos;
                            Scanner::pasaEsp(lin, pos);
                                    tok = Scanner::getToken(
                                lin, pos, delimitadores
                            );
                        } else  tok.erase();
                    }
                    ++pos;
                    Scanner::pasaEsp(lin, pos);
                }
          }

          if ( modoVerbose ) {
            debOut( "Parser: ctrl ids '" + lin + '\'' );
          }
        }
        else pos = antPos;
}

bool ParseDefs::compruebaId(const std::string &x)
{
    size_t i = 1;      // recorrer el id
    const size_t tam = x.length();

	if ( modoVerbose ) {
		debOut( "Parser: comprobando id: '" + x + '\'' );
	}

	if ( tam > Parser::MaxId ) {
	    return false;
	}

    // El id debe empezar por letra o '_'
    if ( isalpha( x[0] ) )
    {
            for(; i < tam; ++i)
            {
                    // Un id puede contener '_', letra o dígito
                    if ( !isdigit( x[i] )
                      && !isalpha( x[i] )
                      && caracteresExtrasIds.find( x[i] ) == std::string::npos )
                    {
                            break;
                    }
            }
    }

    return ( i == tam );
}

// -------------------------------------------------- ParseDefs::preprocesador()
void ParseDefs::preprocesador(std::string &linea)
/*
	El preprocesador procesa las órdens #ifdef ... #else ... #endif,
	actuando en consecuencia.
	Se trata de una pequeña máquina de estados. Como los #if ...
	son anidables, es necesario guardar el estado actual en una pila
*/
{
	size_t pos = 0;
	std::string tok;

    // Tomar el primer token, para ver si es una orden condicional
	Scanner::pasaEsp( linea, pos );
	tok = Scanner::getToken( linea, pos );

    // A lo mejor hay doble almohadilla
    if ( tok[0] == '#'
      && tok[1] == '#' )
    {
            tok.erase( 0, 1 );      // Eliminar una de ellas
    }

	if (  tok == prepIFDEF
	 && ( getEstadoPreProc() == EN_IF_PROCESA
	   || getEstadoPreProc() == PROCESA) )
	{
		// leer el id
		Scanner::pasaEsp( linea, pos );
		tok = Scanner::getToken( linea, pos );

		if ( ts->getCtrConsts()->existeEntrada( tok ) ) {
			cambiaEstadoPreProc( EN_IF_PROCESA );

			if ( modoVerbose ) {
				debOut( "Parser: preprocesador: #ifdef "
					+ tok + " -> procesar"
				);
			}
		} else {
			cambiaEstadoPreProc( EN_IF_TRAGA );

			if ( modoVerbose ) {
				debOut( "Parser: preprocesador: #ifdef "
						+ tok + " -> no procesar"
					   );
			}
		}

		// transforma
		linea = coment + ' ' + linea;
	}
	else
	if (  tok == prepIFNDEF
	 && ( getEstadoPreProc() == EN_IF_PROCESA
	   || getEstadoPreProc() == PROCESA) )
	{
		// leer el id
		Scanner::pasaEsp( linea, pos );
		tok = Scanner::getToken( linea, pos );

		if ( !( ts->getCtrConsts()->existeEntrada( tok ) ) ) {
			cambiaEstadoPreProc( EN_IF_PROCESA );

			if ( modoVerbose ) {
				debOut( "Parser: preprocesador: #ifndef "
						+ tok + " -> procesar"
					   );
			}
		} else {
			cambiaEstadoPreProc( EN_IF_TRAGA );

			if ( modoVerbose ) {
				debOut( "Parser: preprocesador: #ifndef "
						+ tok + " -> no procesar"
					   );
			}
		}

		// transforma
		linea = coment + ' ' + linea;
	}
	else
	if ( tok == prepENDIF )
	{
	  if ( getEstadoPreProc() == EN_IF_TRAGA
	    || getEstadoPreProc() == EN_IF_PROCESA )
	  {

		// leer el id
		Scanner::pasaEsp( linea, pos );
		tok = Scanner::getToken( linea, pos );

		quitaEstadoPreProc();

		if ( modoVerbose ) {
			debOut( "Parser: preprocesador: #endif" );
		}

		// transforma
		linea = coment + ' ' + linea;
	  } else {
	  	throw ErrorPreProc( prepENDIF + strMsg[WITHOUT] + prepIFDEF );
	  }
	}
	else
	if ( tok == prepELSE )
	{
	  if ( getEstadoPreProc() == EN_IF_TRAGA
	    || getEstadoPreProc() == EN_IF_PROCESA )
	  {
		if ( getEstadoPreProc() == EN_IF_TRAGA ) {
			quitaEstadoPreProc();
			cambiaEstadoPreProc( EN_IF_PROCESA );

			if ( modoVerbose ) {
				debOut( "Parser: preprocesador: #else "
						+ tok + " -> procesar"
					   );
			}
		} else {
			quitaEstadoPreProc();
			cambiaEstadoPreProc( EN_IF_TRAGA );

			if ( modoVerbose ) {
				debOut( "Parser: preprocesador: #else "
						+ tok + " -> no procesar"
					   );
			}
		}

		// transforma
		linea = coment + ' ' + linea;
	  } else {
	  	throw ErrorPreProc( prepELSE + strMsg[WITHOUT]
		                     + prepIFDEF + " ... " + prepENDIF );
	  }
	}
}

// ---------------------------------------------------- ParseDefs::procEntrada()
void ParseDefs::procEntrada() throw(Error)
{
  	std::string linact;
	std::ostringstream infoDebug;
	bool infoDebugEscrita = false;

    // Es correcto el fich de entrada ?
    if (    ( scanSCE == NULL )
      || ( !( scanSCE->preparado() ) ) )
    {
        throw ErrorFichEntrada( strMsg[INVALIDFILE]
                + scanSCE->devNombreEntrada() + '\''
        );
    }

    // Tomar primera lin. de la entrada con info
    linact = scanSCE->leeLinea();

    // Hacer sustituciones hasta final de fichero
    while ( !( linact.empty() ) ) {

      // Pasar las líneas en blanco tal y como están
      escribirLineasEnBlanco( scanSCE->devBlank() );
      preprocesador( linact );

      // Preparar info de Debug, si se va a utilizar
      if ( esModoDebug() )
      {
            infoDebugEscrita = false;
            infoDebug.str( "" );

            infoDebug << linact
              << ' ' << coment << '[' << "__txtPAWS_debug:l:"
              << ':' << scanSCE->devNumLinea()
              << ']'
        ;
      }

      if ( !( esContinuacionMacro( linact ) ) )
      {
              macroAnterior.erase();

          if ( getEstadoPreProc() != EN_IF_TRAGA )
          {
               // Hacer los reemplazos/definiciones necesarios
               reemplaza(linact);

               // Es un cod. de ctrl ? (/VOC, /23, /MTX) ...
               if ( cambiaEstado(linact) )
               {
                      // Quitar el cod. de control de esa lin.
                      // Debemos seguir explorando
                      size_t pos = 0;
                      size_t posAnt;
                      Scanner::getToken( linact, pos );
                      Scanner::pasaEsp( linact, pos );

                      // Comprobar si hay tokens de control
                      posAnt = pos;
                      procIdentificadoresControl( linact, pos );

                      // Eliminar control
                      linact.erase( posAnt, pos - posAnt );
                      pos = posAnt;

                      // Guardar lo procesado
                      Scanner::pasaEsp( linact, pos );
                      escribeSalidaParcial( linact.substr( 0, pos ) );

                      // Eliminar lo procesado
                      linact.erase( 0, pos );

                      // Si estamos en modo de debug, añadir el número de línea
                      if ( esModoDebug() ) {
                        linact += infoDebug.str();
                        infoDebugEscrita = true;
                      }
                }
            } else linact = coment + ' ' + linact;
          }

      // Meter info de debug, si es aplicable
      // En las secciones de texto/mensajes, la info de debug
      // solo debe aparece en las lins. de control, como /1
      // y nunca en los mensajes o va a verse por pantalla
      if ( esModoDebug()
        && !infoDebugEscrita
        && devEstado() != LOC
        && devEstado() != OBJMSG
        && devEstado() != MSG
        && devEstado() != SYSMSG )
      {
        linact += infoDebug.str();
      }

          // Escribir la línea ya procesada en la salida
          escribeSalida(linact);

          // Tomar siguiente lin.
          muestraProceso( scanSCE, this );
          linact = scanSCE->leeLinea();
    }

    // Comprobar el estado del preprocesador
    if ( getEstadoPreProc() != PROCESA ) {
        throw ErrorPreProc( strMsg[INVALIDNESTINGPREPROC] );
    }

    quitaEstadoPreProc();

    if ( estadoPreProc.size() != 0 ) {
        throw ErrorPreProc( strMsg[INVALIDNESTINGPREPROC] );
    }

    // Volcados de información varios
    if ( log != NULL ) {
            ts->dumpTxt( *log );
    }

    OutputFile fichXml( Scanner::getNomFich( nomFich ) + ".xml" );
    ts->dumpXml( fichXml );
}

bool ParseDefs::esContinuacionMacro(std::string &lin)
{
	size_t pos   = 0;
	bool toret = false;

	// pasar los espacios
	scanSCE->pasaEsp( lin, pos );

	// Es un signo '+' ?
	// Si lo es, continuamos la macro anterior
	if (lin[pos] == delimContMacro) {
		if (!(macroAnterior.empty())) {
			std::string litAnt;

			// Pasar el '+'
			++pos;
			toret = true;

			// Coger el literal
			std::string lit = Scanner::getLiteral(lin, pos, '"');

			// Incorporarla a la anterior macro
			if (ts->getCtrMacros()->existeEntrada(macroAnterior))
			{
				// Buscarla
				litAnt = ts->getCtrMacros()
				                 ->buscaEntrada(macroAnterior)
				;

				// Modificarla
				ts->getCtrMacros()->insrtEntrada(litAnt
								  + '\n' + lit,
								macroAnterior,
							scanSCE->devNumLinea())
				;

				// Cambiar la línea = ';' + lin
				lin = coment + lin;

				// Para que conste
				ponLogStr( "Insertado al final '" + lit
				         + "' en " + macroAnterior
				);
			}
			else throw ErrorSintaxis(
					strMsg[UNKNOWNPREVMACRO]
					+ macroAnterior + '\''
			);
		}
		else throw ErrorSintaxis( strMsg[UNEXISTINGPREVMACRO] );
	}

	return toret;
}

// =============================================================== parseIncludes
ParseIncludes::ParseIncludes(Scanner *sc) : Parser(sc)
{
    nomFich = Scanner::getNomFich( scanSCE->devNombreEntrada() ) + extmed;
  	fout    = new OutputFile( nomFich );

	if ( fout != NULL
	 &&  fout->isOpen() )
	{
		ponLogStr( "salida: " + nomFich );
		ponLogStr( "==========================" );
	}
	else {
	 throw ErrorInterno( strMsg[OPERRNOMEM] +  nomFich + '\'' );
	}

	if ( modoVerbose ) {
		debOut( "parser: procesando includes a: " + nomFich + "..." );
	}
}

bool ParseIncludes::cambiaEstado(const std::string &lin)
{
        size_t pos = 0;
        std::string tok;

        // Tomar el primer token
        Scanner::pasaEsp( lin, pos );
        tok = Scanner::getToken( lin, pos );
        StringMan::maysCnvt( tok );

        // Si hay dos '#', me cargo una.
        if (tok[0] == '#'
         && tok[1] == '#')
        {
                tok.erase(0, 1);
        }


        return ( tok == include );
}

void ParseIncludes::hazInclude(const std::string &linact)
// Hay un ##include?
// sintaxis ##include <nomfich>
// p.ej., ##include msgsys.txp
{
	size_t pos = 0;
    std::string aux;
    std::string fichincl;
    Scanner *entFich;

	if ( modoVerbose ) {
		debOut( "parser: antes de hacer include ..." );
	}

    // Pasar el #include
    Scanner::pasaEsp(linact, pos);
    aux = Scanner::getToken(linact, pos);
    Scanner::pasaEsp(linact, pos);

    // Obtener el nombre del fichero
	if (linact[pos] == '"') {
		--pos;
		fichincl = Scanner::getLiteral(linact, pos);
	}
	else fichincl = Scanner::getToken(linact, pos);

    // Log
    ponLogStr("Incluyendo Fichero: '"
            + fichincl
            + "' ...")
    ;

	if ( modoVerbose ) {
		debOut( "scanner: incluyendo " + fichincl + "..." );
	}

    // Insertar el fichero
    entFich = new Scanner( fichincl );
    if (    entFich == NULL
	 ||  !( entFich->preparado() ) )
	{
        throw ErrorFichEntrada(
			strMsg[INVALIDFILE] + fichincl + '\''
		);
	}
    else {
            // Marcar en la salida la presencia del include
            aux =  coment;
            aux += ' ';
            aux += linact;
            aux += linSeparadora;
            escribeSalida(aux);

            // "Copiar" el fichero
            aux = entFich->leeLinea();
            while(aux != "")
            {
                    // Pasar las lins. en blanco tal y como aparecen
                    escribirLineasEnBlanco( entFich->devBlank() );

                    // Escribir lin. actual
                    escribeSalida( aux );

                    aux = entFich->leeLinea();
            }

            // Meter una lin. de comentario diferenciadora
            aux = coment;
            aux += linSeparadora;
            escribeSalida( aux );
    }
}

void ParseIncludes::procEntrada() throw(Error) {
  	std::string linact;

        // Comprobar que el fichero es correcto
        if ((scanSCE == NULL)
	||(!(scanSCE->preparado())))
	{
		throw ErrorFichEntrada(
			strMsg[INVALIDFILE] + scanSCE->devNombreEntrada() + '\''
		);
	}

        // Tomar primera lin.
        linact = scanSCE->leeLinea();

        // Procesar todas las lins.
        while( linact != "" ) {
              // Pasar las lins. en blanco tal y como aparecen
              escribirLineasEnBlanco( scanSCE->devBlank() );

              // Hay un ##include?
              if (cambiaEstado(linact))
              {
                // procesar el include
                hazInclude(linact);

                // Meter la lin. como un comentario en la salida
                linact = coment + linact;
              }

              // Escribir la lin., ya procesada
              escribeSalida(linact);

              // Tomar otra lin.
              muestraProceso(scanSCE, this);
              linact = scanSCE->leeLinea();
        }
}

// =================================================================== MapaSust
const int MapaSust::ERR = -1;

// -------------------------------------------------------- mapaNombreNumDesplz
void MapaNombreNumDesplz::insrtEntrada(size_t num, const std::string &txt, size_t nl)
{
        MapaNombreSust<size_t>::insrtEntrada(num + desplz, txt, nl);
}

// ---------------------------------------------------------------- PrefixMixin
PrefixMixin::PrefixesTable PrefixMixin::Prefixes;

const std::string * PrefixMixin::getSuitablePrefix(const std::string &name)
{
    const std::string * toret = NULL;

    if ( Prefixes.size() == 0 ) {
        initPrefixesTable();
    }

    PrefixesTable::const_iterator it = Prefixes.find( name );
    if (  it != Prefixes.end() ) {
        toret = &( it->second );
    }

    return toret;
}

void PrefixMixin::initPrefixesTable()
{
   Prefixes.insert( PrefixesTable::value_type( MARCACONST, "cnst" ) );
   Prefixes.insert( PrefixesTable::value_type( MARCAMACRO, "mcrs" ) );
   Prefixes.insert( PrefixesTable::value_type( MARCALOC,   "l" ) );
   Prefixes.insert( PrefixesTable::value_type( MARCAOBJ,   "o" ) );
   Prefixes.insert( PrefixesTable::value_type( MARCAFLG,   "f" ) );
   Prefixes.insert( PrefixesTable::value_type( MARCAMSG,   "m" ) );
   Prefixes.insert( PrefixesTable::value_type( MARCAPIC,   "p" ) );
   Prefixes.insert( PrefixesTable::value_type( MARCASND,   "s" ) );
   Prefixes.insert( PrefixesTable::value_type( MARCAGRF,   "g" ) );
   Prefixes.insert( PrefixesTable::value_type( MARCAVOC,   prefijoVOC ) );
}


// ========================================================================= TDS
TDS::TDS(size_t numLocs, bool v) {
    // Macros
    tablas.insert(ListaTablas::value_type(MARCAMACRO,
        new TablaMacros( MARCAMACRO, false )));

	// Constantes
    tablas.insert(ListaTablas::value_type(MARCACONST,
        new TablaSusts( MARCACONST, v )));

	// Localidades
    tablas.insert(ListaTablas::value_type(MARCALOC,
		new TablaSusts( MARCALOC, v, numLocs )));


    // Objetos
    tablas.insert(ListaTablas::value_type(MARCAOBJ,
        new TablaSusts( MARCAOBJ, v )));


    // Flags
    tablas.insert(ListaTablas::value_type(MARCAFLG,
        new TablaSusts( MARCAFLG, v )));

    // Mensajes
    tablas.insert(ListaTablas::value_type(MARCAMSG,
        new TablaSusts( MARCAMSG, v )));


    // Gráficos (/localidad)
    tablas.insert(ListaTablas::value_type(MARCAPIC,
        new TablaSusts( MARCAPIC, v, numLocs )));


    // Música (/localidad)
    tablas.insert(ListaTablas::value_type(MARCAMSC,
        new TablaSusts( MARCAMSC, v, numLocs )));


    // Efectos sonoros
    tablas.insert(ListaTablas::value_type(MARCASND,
        new TablaSustsDesplz( MARCASND, numLocs, v )));


    // Efectos gráficos
    tablas.insert(ListaTablas::value_type(MARCAGRF,
        new TablaSustsDesplz( MARCAGRF, numLocs, v )));


	// Vocabulario
    tablas.insert(ListaTablas::value_type(MARCAVOC,
			new TablaSusts( MARCAVOC, v )));

	// Comprobar ahora
	if ( getTabla( MARCAMACRO ) == NULL
	  || getTabla( MARCACONST ) == NULL
	  || getTabla( MARCALOC )   == NULL
	  || getTabla( MARCAFLG )   == NULL
	  || getTabla( MARCAMSG )   == NULL
	  || getTabla( MARCAPIC )   == NULL
	  || getTabla( MARCAMSC )   == NULL
	  || getTabla( MARCASND )   == NULL
	  || getTabla( MARCAGRF )   == NULL
	  || getTabla( MARCAVOC )   == NULL )
	{
		throw ErrorInterno( strMsg[NOMEM] + "(TDS)" );
	}

	// Rellenar con valores iniciales
	( (TablaMacros *) getTabla( MARCAMACRO ) )->insrtEntrada( AttrMacro, AttrMacro, 0 );
}

MapaSust * ParseDefs::getCtr(const std::string &secc)
{
    MapaSust * ctr = NULL;

    if ( secc == MARCAOBJ ) {
            if (devEstado() != DEF
             && devEstado() != OBJ
             && devEstado() != OBJMSG)
                throw SeccionColocaError( strMsg[NOOBJDEF] );
            else ctr = ts->getCtrObjs();
    }
    else
    if ( secc == MARCALOC ) {
            if (devEstado() != DEF
             && devEstado() != LOC)
                throw SeccionColocaError( strMsg[NOLOCDEF] );
            else ctr = ts->getCtrLocs();
    }
    else
    if ( secc == MARCAMSG ) {
            if (devEstado() != DEF
             && devEstado() != MSG
             && devEstado() != SYSMSG)
                throw SeccionColocaError( strMsg[NOMSGDEF] );
            else ctr = ts->getCtrMsgs();
    }
    else
    if ( secc == MARCAFLG ) {
            if (devEstado() != DEF
             && devEstado() != PRO)
                throw SeccionColocaError( strMsg[NOFLGDEF] );
            else ctr = ts->getCtrFlgs();
    }
    else
    if ( secc == MARCAPIC ) {
            if (devEstado() != DEF
             && devEstado() != LOC)
                throw SeccionColocaError( strMsg[NOPICDEF] );
            else ctr = ts->getCtrPics();
    }
    else
    if ( secc == MARCASND ) {
            if (devEstado() != DEF)
        throw SeccionColocaError( strMsg[NOSNDDEF] );
            else ctr = ts->getCtrSnds();
    }
    else
    if ( secc == MARCAMSC ) {
            if (devEstado() != DEF
             && devEstado() != LOC)
        throw SeccionColocaError( strMsg[NOMSCDEF] );
            else ctr = ts->getCtrMscs();
    }
    else
    if ( secc == MARCAGRF ) {
            if (devEstado() != DEF)
        throw SeccionColocaError( strMsg[NOGRFDEF] );
            else ctr = ts->getCtrGrfs();
    }
    else
    if ( secc==MARCACONST ) {
            if (devEstado() != DEF
             && devEstado() != PRO)
                    throw SeccionColocaError( strMsg[CONSTDEFPRO] );
            else ctr = ts->getCtrConsts();
    }
    else
    if ( secc == MARCAMACRO ) {
            if (devEstado() != DEF
             && devEstado() != PRO)
                    throw SeccionColocaError( strMsg[MACRODEFPRO] );
            else ctr = ts->getCtrMacros();
    }
    else throw ErrorSintaxis( '\'' + secc + strMsg[SECNOTFOUND] );

    return ctr;
}

void TDS::devTodosIds(MapaSust::ListaIds &l)
{
    l.clear();

    getCtrMacros()->devIds( l );
    getCtrConsts()->devIds( l );
    getCtrObjs()->devIds( l );
    getCtrVocs()->devIds( l );
    getCtrLocs()->devIds( l );
    getCtrMsgs()->devIds( l );
    getCtrFlgs()->devIds( l );
    getCtrPics()->devIds( l );
    getCtrSnds()->devIds( l );
    getCtrMscs()->devIds( l );
    getCtrGrfs()->devIds( l );
}

TablaMacros * TDS::getCtrMacros(void) {
        return (TablaMacros *) getTabla(MARCAMACRO);
}

TablaSusts * TDS::getCtrConsts(void) {
        return (TablaSusts *) getTabla(MARCACONST);
}

TablaSusts * TDS::getCtrObjs(void) {
        return (TablaSusts *) getTabla(MARCAOBJ);
}

TablaSusts * TDS::getCtrVocs(void) {
        return (TablaSusts *) getTabla(MARCAVOC);
}

TablaSusts * TDS::getCtrLocs(void) {
        return (TablaSusts *) getTabla(MARCALOC);
}

TablaSusts * TDS::getCtrMsgs(void) {
        return (TablaSusts *) getTabla(MARCAMSG);
}

TablaSusts * TDS::getCtrFlgs(void) {
        return (TablaSusts *) getTabla(MARCAFLG);
}

TablaSusts * TDS::getCtrPics(void) {
        return (TablaSusts *) getTabla(MARCAPIC);
}

TablaSustsDesplz * TDS::getCtrSnds(void) {
        return (TablaSustsDesplz *) getTabla(MARCASND);
}

TablaSusts * TDS::getCtrMscs(void) {
        return (TablaSusts *) getTabla(MARCAMSC);
}

TablaSustsDesplz * TDS::getCtrGrfs(void) {
        return (TablaSustsDesplz *) getTabla(MARCAGRF);
}

TDS::~TDS()
{
        ListaTablas::iterator it = tablas.begin();

        while(it != tablas.end()) {
                delete (it->second);
                ++it;
        }
}

MapaSust * TDS::buscaEnTablas(const std::string &id)
{
    MapaSust * toret = NULL;
    ListaTablas::const_iterator it = tablas.begin();

    for(; it != tablas.end(); ++it) {
        if ( it->second->existeEntrada( id ) ) {
            toret = it->second;
            break;
        }
    }

    return toret;
}

MapaSust *TDS::getTabla(const std::string &x)
/*
        Devuelve la tabla buscada por su ID o NULL si no se encuentra.
*/
{
    ListaTablas::iterator it = tablas.find( x );

	if ( it != tablas.end() )
            return it->second;
	else    return NULL;
}

void TDS::insrtConst(const std::string &t, size_t x, size_t nl) {
  	getCtrConsts()->insrtEntrada(x, t, nl);
}

void TDS::insrtVoc(const std::string &t, size_t x, size_t nl) {
  	getCtrVocs()->insrtEntrada(x, t, nl);
}

void TDS::insrtObj(const std::string &t, size_t x, size_t nl) {
  	getCtrObjs()->insrtEntrada(x, t, nl);
}

void TDS::insrtLoc(const std::string &t, size_t x, size_t nl) {
  	getCtrLocs()->insrtEntrada(x, t, nl);
}

void TDS::insrtMsg(const std::string &t, size_t x, size_t nl) {
  	getCtrMsgs()->insrtEntrada(x, t, nl);
}

void TDS::insrtFlg(const std::string &t, size_t x, size_t nl) {
  	getCtrFlgs()->insrtEntrada(x, t, nl);
}

int TDS::buscaObj(const std::string &t) {
  	return getCtrObjs()->buscaEntrada(t);
}

int TDS::buscaLoc(const std::string &t) {
  	return getCtrLocs()->buscaEntrada(t);
}

int TDS::buscaMsg(const std::string &t) {
  	return getCtrMsgs()->buscaEntrada(t);
}

int TDS::buscaFlg(const std::string &t) {
  	return getCtrFlgs()->buscaEntrada(t);
}

bool TDS::existeItem(const std::string &t)
{
    bool toret = false;
    ListaTablas::iterator it = tablas.begin();

	if ( modoVerbose ) {
		debOut( "tds: existe " + t + "? ..." );
	}

    while(it != tablas.end()) {
            if ((it->second)->existeEntrada(t)) {
                    toret = true;
                    break;
            }

            ++it;
    }

	if ( modoVerbose ) {
		if ( toret )
                debOut( "tds: Verdadero, existe" );
		else 	debOut( "tds: No existe" );
	}

    return toret;
}

std::string TDS::buscaMacro(const std::string &x)
{
        return getCtrMacros()->buscaEntrada(x);
}

void TDS::insrtSnd(const std::string & m, size_t v, size_t nl)
{
        getCtrSnds()->insrtEntrada(v, m, nl);
}

void TDS::insrtPic(const std::string & m, size_t v, size_t nl)
{
        getCtrPics()->insrtEntrada(v, m, nl);
}

void TDS::insrtMsc(const std::string & m, size_t v, size_t nl)
{
        getCtrMscs()->insrtEntrada(v, m, nl);
}

void TDS::dumpTxt(OutputFile &f) {
        f.writeLn(linSeparadora + "\n");

        ListaTablas::iterator it = tablas.begin();

        while(it != tablas.end()) {
                f.writeLn((it->second)->getID() + "\n");
                (it->second)->dump(f);

                ++it;
                f.writeLn(linSeparadora + "\n");
        }
}

void TDS::dumpXml(OutputFile &f)
{
	guardar( (FILE *) f.getHandle() );
}

void TDS::guardar(FILE *f)
{
	std::ostringstream buffer;

	escribirCabecera(f);

	escribeAperturaMarca(f, "TDS");

	// Escribir todos y cada uno de los identificadores definidos
        ListaTablas::iterator it = tablas.begin();

        while(it != tablas.end()) {
		(it->second)->iraPrimeraEntrada();

		escribeAperturaMarca(f, (it->second)->getID());

		// Recorrer la tabla
		while(!((it->second)->esUltimaEntrada()))
		{
			// Escribir la info de esta entrada
		        // Escribe el identificador
			escribeAperturaMarca(f, (it->second)->getEtqEntradaAct());

			// Escribe la info
			guardarCampo(f, "VAL", (it->second)->getValorEntradaActString());

			// Lin.?
			buffer.str("");
			buffer << ((it->second)->getNumLineaEntradaAct());
			guardarCampo(f, "LIN", buffer.str());
			escribeCierreMarca(f, (it->second)->getEtqEntradaAct());

			(it->second)->iraSigEntrada();
		}

		escribeCierreMarca(f, (it->second)->getID());
                it++;
        }

	escribeCierreMarca(f, "TDS");
}

Persistente * TDS::recuperar(FILE *f)
{
	return NULL;
}

// -------------------------------------------------------- FichRecursos
const std::string FichRecursos::EXT        = ".blc";
const std::string FichRecursos::linExec    = "Exec";
const std::string FichRecursos::tipoExecGl = "GLUL";
const std::string FichRecursos::linPict    = "Picture";
const std::string FichRecursos::tipoPNG    = "PNG";
const std::string FichRecursos::tipoGIF    = "GIF";
const std::string FichRecursos::tipoJPG    = "JPEG";
const std::string FichRecursos::linSnd     = "Snd";
const std::string FichRecursos::tipoAIFF   = "FORM";
const std::string FichRecursos::tipoMOD    = "MOD";
const std::string FichRecursos::tipoOGG    = "OGGV";
const std::string FichRecursos::tipoSVG    = "SVG";
const std::string FichRecursos::tipoMP3    = "MP3";
const std::string FichRecursos::tipoOGM    = "OGM";
const std::string FichRecursos::EXTEXE     = "ULX";
const std::string FichRecursos::EXTJPG     = "JPG";
const std::string FichRecursos::EXTPNG     = "PNG";
const std::string FichRecursos::EXTGIF     = "GIF";
const std::string FichRecursos::EXTMOD     = "MOD";
const std::string FichRecursos::EXTAIFF    = "AIF";
const std::string FichRecursos::EXTOGG     = "OGG";
const std::string FichRecursos::EXTMP3     = "MP3";
const std::string FichRecursos::EXTSVG     = "SVG";
const std::string FichRecursos::EXTOGM     = "OGM";

FichRecursos::FichRecursos(bool cl)
	: delimDirectorios("/"), pic(NULL), msc(NULL), snd(NULL), grf(NULL),
	  modoLimpio(cl)
{
	// Podemos permitirnos el que el fichero de log no se abra
    flog  = new OutputFile( "fich_recursos.log" );
}

FichRecursos::FichRecursos(const std::string &nom,
               TablaSusts *p,
               TablaSusts *m,
               TablaSustsDesplz *s,
               TablaSustsDesplz *g,
			   bool cl,
			   const std::string &rutas)
	: pic(p), msc(m), snd(s), grf(g),
	  flog(NULL), nombre(nom), modoLimpio(cl)
/*
	Se asume que el nombre llega con el PATH
	pero sin ext.
*/
{
        fres = new OutputFile( nombre + EXT );

	if ( fres != NULL
	  && fres->isOpen() )
	{
		determinaDelimDirectorios( rutas  );
		determinaDelimDirectorios( nombre );
		creaListaRutas( rutas );

		if ( !enModoLimpio() )
		{
			// Podemos permitirnos que falle el fichero de log
			flog = new OutputFile(nombre + EXT + extlog);

			ponLogStr(nombre + EXT);
			ponLogStr("\n====================\n");
		}
	}
	else {
		throw ErrorInterno( strMsg[NOMEM] + '(' + nombre + EXT + ')' );
	}

	if ( modoVerbose ) {
		debOut( "fichrecursos: creando " + nombre + EXT );
		debOut( "fichrecursos: barra de directorios: '"
				  + delimDirectorios + '\''
		      );
	}
}

void FichRecursos::determinaDelimDirectorios(const std::string &rutaEjemplo)
{
	delimDirectorios = "";

	if ( rutaEjemplo.find('/') != std::string::npos ) {
		delimDirectorios += '/';
	}

	if ( rutaEjemplo.find('\\') != std::string::npos ) {
	       delimDirectorios += '\\';
	}
}

void FichRecursos::creaListaRutas(const std::string &rutas)
{
    size_t pos;
    size_t posAnt = 0;
	std::string aux;

	// Borrar la lista de rutas
	lRutas.clear();

	if ( !rutas.empty() )
	{
		// Buscar un separador
		pos    = rutas.find( delimRutas );

		// Encontrado? hay varias rutas!
		while (pos != std::string::npos)
		{
			// Coger la ruta
			aux = rutas.substr( posAnt, pos - posAnt );

			// Comprobar si tiene la barra al final o no
			if ( aux.substr( aux.length() - delimDirectorios.size(),
					aux.length() )
			!= delimDirectorios )
			{
				aux += delimDirectorios;
			}

			// Guardarla
			lRutas.push_back( rutas.substr( posAnt, pos - posAnt ) );

			// Localizar la siguiente
			posAnt = ++pos;
			pos    = rutas.find( delimRutas, posAnt );
		}

		// Coger el id restante
		aux = rutas.substr( posAnt );

		// Comprobar si tiene la barra al final o no
		if ( aux.substr( aux.length() - delimDirectorios.size(),
				aux.length() )
		    != delimDirectorios )
		{
			aux += delimDirectorios;
		}

		// Guardar
		lRutas.push_back( aux );
	}

	if ( modoVerbose ) {
		std::ostringstream dump;

		for(size_t i = 0; i < lRutas.size(); ++i) {
			dump.str( "" );

			dump << "ficherorecursos: ruta " << i << '\''
			     << lRutas[i] << '\'';

			debOut( dump.str() );
		}
	}
}

std::string FichRecursos::buscaFicheroEnRutas(const std::string &nomFich)
/*
	Busca el fichero en el directorio actual, devolviendo
	directamente su nombre si lo encuentra.
	Añade el nombre del fichero a todas las rutas almacenadas,
	hasta encontrarlo. En caso de no encotnrarlo,
	devuelve la cadena vacía.
*/
{
	FILE *fich   = fopen( nomFich.c_str(), "r" );
	std::string toret = nomFich;

	if ( modoVerbose ) {
		debOut( "ficherorecursos: buscando '" + nomFich + '\'' );
	}

	if ( fich == NULL )
	{
		size_t i = 0;
		for (; i < lRutas.size(); ++i)
		{
			toret = lRutas[i] + nomFich;

			// Comprobar esta ruta
			fich = fopen( toret.c_str(), "r" );
			if ( fich != NULL )
				break;
		}

		if ( i >= lRutas.size() ) {
			toret = "";	// Marca que ninguna ruta vale
		}
	}

	// Cerrar el fichero si lo hemos encontrado
	if ( fich != NULL ) {
		fclose( fich );

		if ( modoVerbose ) {
			debOut( "fichrecursos: " + nomFich + " encontrado." );
		}
	}

	if ( modoVerbose ) {
		debOut( "fichrecursos: " + nomFich + " no encontrado." );
	}

	return toret;
}

void FichRecursos::ponLogStr(const std::string &x)
{
	if ( flog != NULL
	 &&  !enModoLimpio() )
	{
		flog->writeLn(x);
	}
}

void FichRecursos::generaFicheroPorTabla(const std::string &marcaID, MapaSust *tbl)
/**
	Llama a buscaFicheroEnRutas() para encontrar la ruta
	al fichero, almacenándola en rFichero.
*/
{
    std::string rFichero;
    const std::string * tipo;

    tbl->iraPrimeraEntrada();

    while ( !tbl->esUltimaEntrada() ) {
        rFichero = buscaFicheroEnRutas( tbl->getEtqEntradaAct() );
        tipo     = completaPorExt( tbl->getEtqEntradaAct() );

        if ( tipo != NULL ) {
            if ( !rFichero.empty() )
            {
                    ponLogStr( "\nEscribiendo línea para "
                            + tbl->getEtqEntradaAct() + '\n')
                    ;

                    fres->writeLn(marcaID + " "
                            + tbl->getValorEntradaActString()
                            + ' ' + ( *tipo ) + ' '
                            + rFichero + '\n'
                    );
            }
            else {
                 ponLogStr( "\nERROR: Archivo de recurso no encontrado: "
                        + tbl->getEtqEntradaAct() )
                 ;

                 throw ErrorFichEntrada(
                         tbl->getEtqEntradaAct() + " no encontrado"
                 );
            }
        }
        else {
            ponLogStr( "\nERROR: " + tbl->getEtqEntradaAct() + " no soportado");

            throw ErrorFichEntrada(
                        tbl->getEtqEntradaAct() + " no soportado"
            );
        }

        tbl->iraSigEntrada();
    }
}

inline
void FichRecursos::comprobarRepeticiones(TablaSusts *m)
{
	if ( !m->compruebaSinRepeticionesEn() ) {
		std::ostringstream msg;

		msg << '\'' <<  m->getEtqEntradaAct()
		    << '\'' << ' ' << '=' << ' '
		    << m->getValorEntradaAct()
		    << strMsg[REPEATEDVAL]
		;

		if ( m->buscaPrimeraEntradaConValor( m->getValorEntradaAct() ) )
                msg << m->getEtqEntradaAct();
		else    msg << strMsg[ INTERNAL ];

		msg << '\'';

		throw ErrorIdDuplicado( msg.str() );
	}
}

void FichRecursos::generaFichero()
/*
        Nombre es un nombre sin ext., pero con PATH
*/
{
    if ( modoVerbose ) {
        debOut( "fichrecursos: generando fichero recursos ..." );
    }

    if (fres != NULL) {
            // Meterlo todo en dos tablas
            TablaSusts snds( "snds" );
            TablaSusts pics( "pics" );

            // Generar las lins. respecto a los sonidos
            ponLogStr("\n\n\n- mezclando tablas para "
                + linSnd
                + " (efectos sonoros y msc.)\n"
            );
            snds.insertaMapa( *msc );
            snds.insertaMapa( *snd );

            // Generar las lins. respecto a los sonidos
            ponLogStr("\n\n\n- mezclando tablas para "
                + linPict
                + " (efectos grfs. y grfs. por localidad)\n"
            );
            pics.insertaMapa( *pic );
            pics.insertaMapa( *grf );

            // Comprobar las tablas de sustituciones
            comprobarRepeticiones( &pics );
            comprobarRepeticiones( &snds );

            // Generar la primera lin., la del ejecutable
            fres->writeLn(linExec + " 0 " + tipoExecGl + " " +
                           nombre + '.' + StringMan::mins( EXTEXE ) + "\n");
            ponLogStr("\nEscribiendo: "+ nombre + " " + EXTEXE);


            // Generar  las lins. respecto a los grfs.
            ponLogStr("\n\n\n- Preparando lns. para grfs. " + linPict );
            generaFicheroPorTabla( linPict, &pics );

            // Generar las lins. respecto a la msc.
            ponLogStr( "\n\n\n- Preparando lns. para sonidos " + linSnd );
            generaFicheroPorTabla( linSnd, &snds );

            ponLogStr( "\nFin\n" );
    } else flog->writeLn("\n\nFichero de recursos no generado.\n\n");

    if ( modoVerbose ) {
        if ( fres != NULL )
            debOut( "fichrecursos: generado." );
        else	debOut( "fichRecursos: no generado." );
    }
}

MapaSust * FichRecursos::tablaPorExt(const std::string &nomfich, TDS *tds)
/**
  Devuelve un puntero a la tabla de sustituciones correspondiente a un nombre
  de fichero, de los que se usan en la tabla de recursos.
 * El objetivo es que cuando se ponga: /loc = casa(casa.png, casa.ogg),
 * se sepa la tabla para almacenar el id.
*/
{
    if ( !nomfich.empty()
       && nomfich.length() > EXTLEN )
    {
        std::string ext = nomfich.substr( nomfich.length() - EXTLEN, EXTLEN );
        StringMan::maysCnvt( ext );

        if ( ext == EXTJPG
          || ext == EXTPNG
          || ext == EXTGIF
          || ext == EXTSVG )
        {
            return tds->getCtrPics();
        }
        else
        if ( ext == EXTAIFF
          || ext == EXTOGG
          || ext == EXTMP3
          || ext == EXTMOD )
        {
            return tds->getCtrMscs();
        }
    }

    return NULL;
}

const std::string *FichRecursos::completaPorExt(const std::string &nomfich)
/**
 * @param nomfich el nombre del archivo de recursos
   @return Devuelve una cadena con el tipo correcto de archivo -JPG, PNG, MOD, AIF-
     (por la ext. del archivo).
*/
{
    const std::string * toret = NULL;

    if ( !nomfich.empty()
       && nomfich.length() > EXTLEN )
    {
        std::string ext = nomfich.substr( nomfich.length() - EXTLEN, EXTLEN );
        StringMan::maysCnvt( ext );

        if ( ext == EXTJPG ) {
            toret = &tipoJPG;
        }
        else
        if ( ext == EXTPNG ) {
            toret = &tipoPNG;
        }
        else
        if ( ext == EXTGIF ) {
            toret = &tipoGIF;
        }
        else
        if ( ext == EXTAIFF ) {
            toret = &tipoAIFF;
        }
        else
        if ( ext == EXTMOD ) {
            toret = &tipoMOD;
        }
        else
        if ( ext == EXTOGG ) {
            toret = &tipoOGG;
        }
        else
        if ( ext == EXTMP3 ) {
            toret = &tipoMP3;
        }
        else
        if ( ext == EXTSVG ) {
            toret = &tipoSVG;
        }
        else
        if ( ext == EXTOGM ) {
            toret = &tipoOGM;
        }
    }


    return toret;
}


// ----------------------------------------------------------- ParseDefsRecursos
ParseDefsRecursos::ParseDefsRecursos(Scanner *sc, size_t nl, bool dbg,
                                     bool cl, bool v, const std::string &n,
                                     const std::string & r)
		: ParseDefs(sc, nl, dbg, cl, v, n), fres(NULL), rutas(r)
{
}

void ParseDefsRecursos::procEntrada(void) throw(Error)
{
        // Hacer el proceso de entrada normalmente
        ParseDefs::procEntrada();

        // Crear fichero de recursos
        ponLogStr("Generando fichero de recursos ...");

	try {
		fres = new FichRecursos(
				Scanner::getNomFich( nomFich ),
				ts->getCtrPics(),
				ts->getCtrMscs(),
				ts->getCtrSnds(),
				ts->getCtrGrfs(),
				enModoLimpio(),
				rutas );

		if ( fres != NULL )
		{
			fres->generaFichero();
		}
		else throw ErrorInterno(
			   	strMsg[NOMEM] + '\''
				+ Scanner::getNomFich( nomFich )
                                + FichRecursos::EXT + '\''
		);
	} catch (Error &e) {
            throw;
	}
        catch( ... ) {
            throw ErrorInterno( "procesando fichero de recursos ..." );
        }
}

// ------------------------------------------------------------------------ main
std::string volcarScannerParserInfo(const std::runtime_error &e, Scanner *sce, Parser *p, size_t &numLinea)
{
    std::ostringstream buffer;
    numLinea = 0;
    const Error * error;

    if ( sce != NULL ) {
        numLinea = sce->devNumLinea();
    }

    if ( ( error = dynamic_cast<const Error *>( &e ) ) != NULL ) {
        if ( error->hayNumLin() ) {
            numLinea = error->getNumLin();
        }
    }

    // Volcar, si procede, la TDS
    if ( p != NULL
      && dynamic_cast<ParseDefsRecursos *>( p ) != NULL )
    {
        ( (ParseDefsRecursos *) p )->forzDump();
    }

    // Volcar la info del scanner para error
    if ( sce != NULL ) {
        buffer << '#' << sce->devNombreEntrada()
             << '(' << numLinea << ')'
             << ':' << '\'' << e.what() << '\'' << ':'
             << ' ' << sce->getLineaAct() << '\n'
             << std::setw(7)
             << numLinea << ',' << ' '
        ;
    }

    return buffer.str();
}

std::string volcarError(const std::runtime_error &e, Scanner *sce, Parser *p)
{
    const Error * error = dynamic_cast<const Error *>( &e );
    std::ostringstream buffer;
    size_t numLinea;

    buffer << volcarScannerParserInfo( e, sce, p, numLinea );
    buffer << "#ERROR: '" << e.what();

    if ( error != NULL
      && !( error->getDetails().empty() ) )
    {
            buffer << '(' << error->getDetails() << ')';
    }

    buffer << '\'' << '.' << '\n';

    return buffer.str();
}

void procError(const std::runtime_error &e, Scanner *sce, Parser *p)
{
    std::string buffer = volcarError( e, sce, p );

    if ( p != NULL ) {
            p->ponLogStr( buffer );
    }

    porPantallaError( buffer );
    porPantallaError( "\n" );
}

void mostrarSintaxis()
{
	porPantalla(nombre);
	porPantalla(version);
	porPantalla("\n\n\n");
    porPantalla( strMsg[CMDSYNTAX] );
    porPantalla( strMsg[CMDOPTIONS] );
}

std::string procesaOpciones(int argc, char *argv[], size_t &numLocs,
		    bool &modoDebug, bool &modoLimpio, bool &modoQuiet,
		    bool &modoVerify,
		    std::string &fichS, std::string &rutas)
/*
	Procesa todas las opciones que hayan sido incluidas en la línea de comando,
	y devuelve, cuando lo encuentra, el nombre del fichero de entrada
*/
{
	int argAct = 1;
	std::string arg;

	modoDebug  = false;
	modoLimpio = false;
	modoQuiet  = false;

	while ( argAct < argc )
	{
		// Eliminar las primeras comillas, si existen
		if ( *( argv[ argAct ] ) == '\"' )
			arg = argv[ argAct ] + 1;
		else	arg = argv[ argAct ];

		// Eliminar las segundas comillas (al final), si existen
		if ( arg[ arg.length() - 1 ] == '\"' ) {
			arg.erase( arg.length() - 1, 1 );
		}

		if ( arg[0] == '-' )
		{
			// Eliminar los guiones precedentes
			{
				size_t guion = 0;

				while ( arg[guion] == '-' ) {
					++guion;
				}

				if ( guion > 0 ) {
					arg.erase( 0, guion );
				}
			}

			// Encontrar la que corresponde
			if ( arg.length() >= opInclude.length()
			  && arg.compare( 0, opInclude.length(), opInclude ) == 0 )
			{
				rutas = arg.substr( opInclude.length(),
						arg.length()
				);
			}
			else
			if ( arg.length() >= opOutput.length()
			  && arg.compare( 0, opOutput.length(), opOutput ) == 0 )
			{
				// Cuidado con el siguiente, que hay dos versiones
				fichS = arg.substr( opOutput.length(),
						arg.length()
				);
			}
			else
			if ( arg.length() >= opOutput2.length()
			  && arg.compare( 0, opOutput2.length(), opOutput2 ) == 0 )
			{
				fichS = arg.substr( opOutput2.length(),
						arg.length()
				);
			}
            else
			if ( arg.length() >= opVerify.length()
			  && arg.compare( 0, opVerify.length(), opVerify ) == 0 )
			{
				modoVerify = true;
			}
			else
			if ( arg.length() >= opNumLoc.length()
			  && arg.compare(0, opNumLoc.length(), opNumLoc) == 0)
			{
				// Se puede cambiar el num. de localidades
				// máximas.
				std::istringstream buffer(
						argv[argAct] + 1
						+ opNumLoc.length()
				);
				buffer >> numLocs;
			}
			else
			if ( arg.length() >= opInform.length()
			  && arg.compare(0, opInform.length(), opInform) == 0)
			{
				// Cambian ciertos aspectos, para poder emplearlo
				// con Inform
				coment = COMENT_INFORM;
				extsal = EXTSAL_PAWS;
			}
			else
			if ( arg.length() >= opDebug.length()
			  && arg.compare(0, opDebug.length(), opDebug) == 0)
			{
				// EL modo de depuración hace que se añada
				// info. de num. de lin. de cada TXP
				// a cada lin. del SCE
				modoDebug = true;
			}
			else
			if ( arg.length() >= opClean.length()
			  && arg.compare(0, opClean.length(), opClean) == 0)
			{
				// No crea los ficheros de log
				modoLimpio = true;
			}
			else
			if ( arg.length() >= 4
			  && arg.compare(0, 4, "MOTR") == 0)
			{
				// Checreto, checreto
				porPantalla( "\ntxtPAWS Primavera de 2014"
					"\nViva ngPAWS!\n\n"
				);
			}
			else
			if ( arg.length() >= opQuiet.length()
			  && arg.compare(0, opQuiet.length(), opQuiet) == 0)
			{
				// No muestra salida por pantalla
				modoQuiet = true;
			}
			else
			if ( arg.length() >= opVerbose.length()
			  && arg.compare(0, opVerbose.length(), opVerbose) == 0)
			{
				modoVerbose = true;
			}
			else
			if ( arg.length() >= opIngles.length()
			  && arg.compare(0, opIngles.length(), opIngles) == 0)
			{
				strMsg = strMsgUk;
			}
			else {
				mostrarSintaxis();
				throw ErrorSintaxis( strMsg[UNKNOWNCMDOP] +
						std::string(argv[argAct]) + '\'');
			}
		} else break;

		++argAct;
	}

	if (argAct >= argc) {
		mostrarSintaxis();
		throw ErrorSintaxis( strMsg[MISSINGSCENAME] );
	}

	return arg;
}

void cargaFicheroCondactos()
{
    std::string ln;
    InputFile f( CNDACTSFILE );

    if ( f.isOpen() ) {
        ParseDefs::Condactos.reset( new ParseDefs::ListaCondactos() );
        porPantalla( "Chk CndActs...\n\n" );

        do {
            StringMan::maysCnvt( StringMan::trimCnvt( f.readLn( ln ) ) );

            if ( !ln.empty() ) {
                ParseDefs::Condactos->insert( ln );
            }
        } while( !f.isEof() );
    }
    else ParseDefs::preparaCondactos();
}

int procesarAventura(int argc, char *argv[])
{
        std::auto_ptr<Scanner> sce;
        std::auto_ptr<Parser>  p;
        int toret            = 0;
        size_t numLocs       = 256;
        bool modoDebug       = false;
        bool modoLimpio      = false;
        bool modoVerify      = false;
        std::string nombreFichEntrada;
        std::string nombreFichSalida;
        std::string rutas;

        try {
          if (argc < 2) {
                mostrarSintaxis();
          }
          else {

          porPantalla( nombre );
          porPantalla( version );
          porPantalla( "\n\n\n" );

	  	  nombreFichEntrada = procesaOpciones( argc, argv,
		  				    numLocs,
						    modoDebug,
						    modoLimpio,
						    modoQuiet,
						    modoVerify,
						    nombreFichSalida,
						    rutas )
		  ;

 		  // Mostrar opciones
 		  porPantalla(opVerify + ':' + ' '+ (modoVerify? "On ":"Off "));
		  porPantalla(opDebug + ':' + ' '+ (modoDebug? "On ":"Off "));
		  porPantalla(opClean + ':' + ' '+ (modoLimpio? "On ":"Off "));
		  porPantalla(opQuiet + ':' + ' '+ (modoQuiet? "On\n":"Off"));
		  porPantalla( "\nRutas: \"" + rutas + "\"\n");

		  // Mostrar nombre de Fichero
		  porPantalla( "\nf  : " );
          porPantalla( nombreFichEntrada );
          porPantalla( "\n\n" );

		  // info de depuración
		  if ( modoVerbose ) {
			std::ostringstream linArg;

			debOut( "toplevel: verbose activado ..." );
			debOut( "toplevel: leyendo '"
			        + nombreFichEntrada + '\''
			);
			debOut( "toplevel: creando scanner para "
				     "leer el fichero ..."
			);

			for(int i = 0; i < argc; ++i) {
				linArg.str( "" );

				linArg << "toplevel: arg "
				       << i << ':' << ' '
				       << '\'' << argv[i] << '\''
				;

				debOut( linArg.str() );
			}
		  }

		  // Preparar el scanner
          sce.reset( new Scanner( nombreFichEntrada ) );

          if ( !sce->preparado() ) {
            porPantallaError(
				strMsg[INVALIDFILE]
				+ sce->devNombreEntrada() + "'\n"
			);
          }
          else {
              // Crear fichero intermedio, para incluirlos todos
              try {
                  if ( modoVerbose ) {
                      debOut( "toplevel: antes de crear el "
                               "parser"
                      );
                  }
                  p.reset( new ParseIncludes( sce.get() ) );

                  if ( modoVerbose ) {
                    debOut( "toplevel: antes de "
                            "procesar los includes ..."
                    );
                   }

                   p->procEntrada();
			  } catch(...) {
				throw ErrorFichEntrada( strMsg[INCLUDESNOPROC] );
			  }

              // Hacer las sustituciones
			  if ( modoVerbose ) {
				  debOut( "toplevel: antes de leer "
					       "fichero intermedio ..."
					     );
			  }

              sce.reset( new Scanner(
			        	Scanner::getNomFich( nombreFichEntrada )
				      + extmed )
			  );

              if ( sce->preparado() ) {
                    if ( modoVerbose ) {
                        debOut( "toplevel: antes de "
                             "crear el parser ..."
                        );
                    }

                    p.reset( new ParseDefsRecursos( sce.get(),
                             numLocs,
							 modoDebug,
							 modoLimpio,
							 modoVerify,
                             nombreFichSalida,
							 rutas )
                    );

                    cargaFicheroCondactos();

                    // procesar
                    if ( modoVerbose ) {
				      debOut( "toplevel: antes de "
                              "procesar susts ..."
				      );
                    }

                    p->procEntrada();
                    std::string avisos = p->chk();

                    if ( !avisos.empty() ) {
                        porPantalla( avisos );
                        porPantalla( "\n" );
                    }

                    porPantalla( "Ok, EOF(" + p->getNomFich() + ")\n" );
                } else {
			       throw ErrorFichEntrada( strMsg[INCLUDESNOPROC] );
			  }
            }
          }
        } catch(Error &e) {
            toret = -1;
            procError( e, sce.get(), p.get() );
        } catch(std::runtime_error &e) {
            toret = -1;
            procError( e, sce.get(), p.get() );
        }

        catch(...) {
            porPantallaError( strMsg[PANIC] );
            porPantallaError( "\n" );
        }

  	return toret;
}

} // de txtPAWS

// ------------------------------------------- Dependientes de la implementación
#ifdef __TEXT__UI
void muestraProceso(TxtPAWS::Scanner *, TxtPAWS::Parser *) {
}

void porPantalla(const std::string &s)
{
	if ( !TxtPAWS::modoQuiet ) {
        	std::cout << s;
	}
}

void debOut(const std::string &s)
{
	porPantalla( "-- " + s + "\n" );
}

void porPantallaError(const std::string &s)
{
	std::cerr << s;
}

int main(int argc, char *argv[])
{
/*
	int x;
    std::string s;

	std::cout << std::endl;
	std::cout << "Msg traducc. #final: " << TxtPAWS::PANIC << std::endl;

	for(;;) {
        std::cout << "\nMsg #: ";
        std::getline( std::cin, s );
        x = std::atoi( s.c_str() );

        std::cout << std::endl;
        std::cout << "Msg lang=ES: " << TxtPAWS::strMsgSp[x] << std::endl;
        std::cout << "Msg lang=EN: " << TxtPAWS::strMsgUk[x] << std::endl;
	}
*/
    return TxtPAWS::procesarAventura(argc, argv);
}
#endif







