// txtpaws.h
/*
  	Cabecera
  	Este programa toma un fuente en algo parecido al SCE,
  	pero admitiendo comentarios y nombres para las variables,
  	y genera SCE PAWS puro
*/

#include <cstdio>
#include <cstdlib>
#include <string>
#include <map>
#include <stack>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <cctype>
#include <set>
#include <limits>
#include <memory>

#include "persistente.h"
#include "msg.h"
#include "fileio.h"
#include "stringman.h"
#include "excep.h"

namespace TxtPAWS {

extern const size_t MAXNUM;
extern const std::string MARCADEFINE;
extern const std::string MARCACONST;
extern const std::string MARCAMACRO;
extern const std::string MARCAOBJ;
extern const std::string MARCAFLG;
extern const std::string MARCALOC;
extern const std::string MARCAMSG;
extern const std::string MARCAPIC;
extern const std::string MARCASND;
extern const std::string MARCAMSC;
extern const std::string MARCAGRF;
extern const std::string linSeparadora;

// ------------------------------------------------------   El scanner

class Scanner {
protected:
    std::string buf;
    std::string nombre;
private:
    InputFile *f;
    size_t linea;
    size_t blank;

public:
    const static std::string DELIMITADORES;
    static const size_t MAXBUFFER = 16384;
    static void   pasaEsp(const std::string &, size_t &, const std::string &delim = DELIMITADORES);
    static std::string getToken(const std::string &, size_t &, const std::string &delim = DELIMITADORES);
    static std::string getNomFich(const std::string &);
    static std::string getLiteral(const std::string &, size_t &, char = '"');

    Scanner(const std::string &);
    ~Scanner();

    const std::string &leeLinea(void);

    bool finEntrada(void) const
        { return f->isEof(); }
    bool preparado(void)  const
        { return f->isOpen(); }
    size_t devNumLinea(void) const
        { return linea; }
    const std::string &getLineaAct() const
        { return buf; }
    const std::string &devNombreEntrada(void) const
        { return nombre; }
    size_t devBlank(void) const
        { return blank; }

    static bool isNumber(const std::string &str);
    static unsigned int toInt(const std::string &str);
};

// ---------------------------------------------- La tabla de s�mbolos
class MapaSust {
private:
        size_t limite;
public:
        typedef std::vector<const std::string *> ListaIds;
        static const int ERR;
        size_t getLimite() const { return limite; }

        MapaSust(size_t lim = 256) : limite(lim - 1) {}
        virtual ~MapaSust() {}

        virtual const std::string &getID(void) const                     = 0;

        virtual bool existeEntrada(const std::string &)                  = 0;

        virtual std::string buscaEntradaStr(const std::string &)         = 0;
        virtual void iraPrimeraEntrada()                                 = 0;
        virtual void iraSigEntrada()                                     = 0;
        virtual bool esUltimaEntrada()                                   = 0;
        virtual const std::string &getEtqEntradaAct() const              = 0;
        virtual std::string getValorEntradaActString() const             = 0;
        virtual size_t getNumLineaEntradaAct() const                     = 0;
        virtual void dump(OutputFile &)                                  = 0;
        virtual void devIds(ListaIds &l) const                           = 0;
        virtual const std::string &getAvisos() const                     = 0;
        virtual size_t getNumLinea(const std::string &etq)               = 0;

};

// =============================================================================
template <typename T>
class MapaNombreSust : public MapaSust {
  	public:
		struct Info {
			T info;
			size_t numLinea;

			std::string toString() const
                { std::ostringstream cnvt; cnvt << info; return cnvt.str(); }
		};

   		typedef std::map<std::string, Info> mapNS;
  	private:
            std::string id;
    		mapNS mapa;

            typename mapNS::iterator itact;

  	public:
            MapaNombreSust(const std::string &, size_t lim = MAXNUM);
            virtual ~MapaNombreSust() {}

            const std::string &getID() const { return id; }
    		virtual void insrtEntrada(const T &num, const std::string &txt, size_t nl = 0);
    		T buscaEntrada(const std::string &txt);
            bool existeEntrada(const std::string &);
            std::string buscaEntradaStr(const std::string &);
            bool buscaPrimeraEntradaConValor(const T &x);
            const Info &buscaEntradaCompleta(const std::string &);
            size_t getNumLinea(const std::string &etq);

            typename mapNS::iterator getEntradaAct() const
                { return itact; }

            void iraPrimeraEntrada();
            void iraSigEntrada();
            bool esUltimaEntrada();
            const std::string &getEtqEntradaAct() const;
            const Info &getInfoEntradaAct() const;
            const T &getValorEntradaAct() const;
            std::string getValorEntradaActString() const;
            size_t getNumLineaEntradaAct() const;
            void devIds(ListaIds &l) const;

            void dump(OutputFile &);
            bool compruebaSinRepeticionesEn();
            void insertaMapa(MapaNombreSust<T> &t);
};

// =============================================================================

/**
    Esta clase guarda valores, comprobando que cumplan con un determinado
    prefijo
*/
class PrefixMixin {
public:
  typedef std::map<std::string, std::string> PrefixesTable;

  static PrefixesTable Prefixes;
  static const std::string * getSuitablePrefix(const std::string &);
  static void initPrefixesTable();
};

template <typename T>
class Tabla : public MapaNombreSust<T>, PrefixMixin {
  bool verify;
  std::string avisos;
public:
  static const bool VerifyStyle = true;
  static const bool DoNotVerifyStyle = false;

  Tabla(const std::string &n, bool v = VerifyStyle, size_t lim = MAXNUM)
    : MapaNombreSust<T>( n, lim ), verify( v )
    {}

   bool chkId(const std::string &);
   bool isToVerify() const
    { return ( verify == VerifyStyle ); };

   void insrtEntrada(const T &num, const std::string &txt, size_t nl = 0)
    { chkId( txt ); MapaNombreSust<T>::insrtEntrada( num, txt, nl ); }
   const std::string &getAvisos() const
    { return avisos; }
};

// Implementaci�n de las plantillas
#include "mapans.cc"

/**
        Esta clase guarda n�meros, como podr�a hacer la anterior, si bien,
        a�ade un desplazamiento a todos los valores insertados en ella.
*/
class MapaNombreNumDesplz : public Tabla<size_t> {
private:
    size_t desplz;
public:
    const static size_t DSPSND = 256;
	const static size_t DSPGRF = 256;

    MapaNombreNumDesplz(const std::string &x,
                size_t desp = 0,
                bool v = VerifyStyle,
                size_t lim = MAXNUM)
            : Tabla<size_t>(x, v, lim), desplz(desp)
    {};

    size_t getDesplz(void) { return desplz; }
    void insrtEntrada(size_t num, const std::string &txt, size_t nl = 0);
};

class TablaMacros : public Tabla<std::string> {
public:
    TablaMacros(const std::string &n, bool v = VerifyStyle, size_t lim = MAXNUM)
    : Tabla<std::string>( n, v, lim )
    {}
};

class TablaSustsDesplz : public MapaNombreNumDesplz {
public:
    TablaSustsDesplz(const std::string &n, int desp = 0, bool v = VerifyStyle, size_t lim = MAXNUM)
    : MapaNombreNumDesplz( n, desp, v, lim )
    {}
};

class TablaSusts : public Tabla<size_t> {
public:
    TablaSusts(const std::string &n, bool v = VerifyStyle, size_t lim = MAXNUM)
    : Tabla<size_t>( n, v, lim )
    {}
};

class TDS : public Persistente {
public:
    typedef std::map <std::string, MapaSust *> ListaTablas;
private:
    ListaTablas tablas;
public:
    TDS(size_t numLocs = 256, bool v=true);
    ~TDS();

    MapaSust *getTabla(const std::string &);

    TablaMacros *getCtrMacros(void);
    TablaSusts *getCtrConsts(void);
    TablaSusts *getCtrObjs(void);
    TablaSusts *getCtrLocs(void);
    TablaSusts *getCtrMsgs(void);
    TablaSusts *getCtrFlgs(void);
    TablaSusts *getCtrPics(void);
    TablaSustsDesplz *getCtrSnds(void);
    TablaSustsDesplz *getCtrGrfs(void);
    TablaSusts *getCtrMscs(void);
    TablaSusts *getCtrVocs(void);

    void insrtMcr(const std::string &, const std::string &, size_t = 0);
    void insrtConst(const std::string &, size_t, size_t = 0);
    void insrtObj(const std::string &, size_t, size_t = 0);
    void insrtLoc(const std::string &, size_t, size_t = 0);
    void insrtMsg(const std::string &, size_t, size_t = 0);
    void insrtFlg(const std::string &, size_t, size_t = 0);
    void insrtSnd(const std::string &, size_t, size_t = 0);
    void insrtPic(const std::string &, size_t, size_t = 0);
    void insrtMsc(const std::string &, size_t, size_t = 0);
    void insrtVoc(const std::string &, size_t, size_t = 0);

    int buscaObj(const std::string &);
    int buscaLoc(const std::string &);
    int buscaMsg(const std::string &);
    int buscaFlg(const std::string &);
    std::string buscaMacro(const std::string &);
    MapaSust * buscaEnTablas(const std::string &);
    void devTodosIds(MapaSust::ListaIds &);

    bool existeItem(const std::string &);
    ListaTablas &getTablas()
        { return tablas; }

    Persistente * recuperar(FILE *);
    void guardar(FILE *);

    void dumpTxt(OutputFile &);
    void dumpXml(OutputFile &);
};

// ------------------------------------------------------------------ Los parser

class FichRecursos {
public:
		typedef std::vector<std::string> ListaRutas;
protected:
		std::string delimDirectorios;
        TablaSusts *pic;
        TablaSusts *msc;
        TablaSustsDesplz *snd;
        TablaSustsDesplz *grf;
        OutputFile    *fres;
        OutputFile    *flog;
        std::string   nombre;
		bool modoLimpio;

        static const std::string *completaPorExt(const std::string &);

		void creaListaRutas(const std::string &);
		std::string buscaFicheroEnRutas(const std::string &);
		void determinaDelimDirectorios(const std::string &rutaEjemplo);

        FichRecursos(bool cl = false);
public:
		ListaRutas lRutas;
        static const std::string EXT;
        static const std::string linExec;
        static const std::string tipoExecGl;
        static const std::string linPict;
        static const std::string tipoPNG;
        static const std::string tipoGIF;
        static const std::string tipoJPG;
        static const std::string linSnd;
        static const std::string tipoAIFF;
        static const std::string tipoMOD;
        static const std::string tipoOGG;
        static const std::string tipoSVG;
        static const std::string tipoMP3;
        static const std::string tipoOGM;
        static const std::string EXTEXE;
        static const std::string EXTJPG;
        static const std::string EXTPNG;
        static const std::string EXTGIF;
        static const std::string EXTMOD;
        static const std::string EXTAIFF;
        static const std::string EXTMP3;
        static const std::string EXTOGG;
        static const std::string EXTSVG;
        static const std::string EXTOGM;

        static const size_t EXTLEN = 3;

		static MapaSust * tablaPorExt(const std::string &, TDS *);

        FichRecursos(const std::string&,
            TablaSusts *,
            TablaSusts *,
            TablaSustsDesplz *,
            TablaSustsDesplz *,
            bool cl = false,
            const std::string & = "")
		;

		void generaFicheroPorTabla(const std::string &, MapaSust *);
                void generaFichero();

		bool enModoLimpio() const { return modoLimpio; }
		void comprobarRepeticiones(TablaSusts *);

		void ponLogStr(const std::string &);
};



// -------------------------------------------------------- El parser
class Parser {
protected:
        std::string nomFich;
        Scanner *scanSCE;
        OutputFile *fout;
        OutputFile *log;
		bool modoLimpio;

        void escribeSalida(const std::string &);
		void escribeSalidaParcial(const std::string &);
        void escribirLineasEnBlanco(size_t);

        Parser();
public:
        static const size_t MaxId = 256;

        Parser(Scanner *, bool cl = false);
        virtual ~Parser();

        const std::string &getNomFich() const { return nomFich; }

        virtual bool cambiaEstado(const std::string &) = 0;

        virtual void procEntrada(void) = 0;

        void ponLogStr(const std::string &);

		bool enModoLimpio()        const { return modoLimpio; }

		virtual std::string chk() { return ""; };
};

class ParseIncludes : public Parser {
public:
        ParseIncludes(Scanner *);

        bool cambiaEstado(const std::string &);
        void procEntrada();
protected:
        void hazInclude(const std::string &);
private:
        size_t numLinGlobal;
        size_t numLinInclude;
};

class ParseDefs : public Parser {
private:
        size_t numItem;
		size_t numLocalidades;
		bool modoDebug;
		bool verify;

public:
        typedef std::set<std::string> ListaCondactos;
        static std::unique_ptr<ListaCondactos> Condactos;
        typedef enum { DEF, CTL, VOC, SYSMSG, MSG, OBJMSG,
                       LOC, CON, OBJ, PRO
        } TipoEstado;

		static const std::string StrEstado[];

		typedef enum { PROCESA, EN_IF_TRAGA, EN_IF_PROCESA }
		        TipoEstadoPreProc
		;

private:
		std::stack<TipoEstadoPreProc> estadoPreProc;
        TipoEstado estado;
		std::string macroAnterior;

protected:
        TDS *ts;

        TipoEstado cambiaEstado(TipoEstado);

		TipoEstadoPreProc cambiaEstadoPreProc(TipoEstadoPreProc);
		TipoEstadoPreProc getEstadoPreProc() const;

        MapaSust *getCtr(const std::string &);

		void quitaEstadoPreProc();

		bool sust(  std::string &toret,
                    const std::string &etq,
                    const std::string &txt,
                    size_t pos = std::string::npos );

        bool compruebaId(const std::string &);
    	void reemplaza(std::string &);
		void hazMacro(std::string &toret, size_t pos, std::string &macro);
		void hazAttr(std::string &toret, size_t pos, std::string &txt);

        void ponLogInt(int);

        void tomaDef(const std::string &);
        void hazRepl(std::string &);

		void preprocesador(std::string &);

		bool esContinuacionMacro(std::string &lin);

        void procIdentificadoresControl(const std::string &lin, size_t &pos);
		void procesaVocabulario(const std::string &);

public:
        void forzDump() { if ( log != NULL ) ts->dumpTxt( *log ); }

        ParseDefs(Scanner *, size_t numLocs = 256,
		          bool dbg = false, bool cl = false, bool v = true,
                  const std::string &nomFichSal = "" )
		;

        TipoEstado devEstado(void) const { return estado; }

		bool esModoDebug()         const { return modoDebug; }

        bool cambiaEstado(const std::string &);

        void procEntrada(void);

		size_t getMaxNumLocalidades() const { return numLocalidades; }

        size_t getNumItem() const { return numItem; }

        std::string chk();

        static void preparaCondactos();
};

class ParseDefsRecursos : public ParseDefs {
private:
        FichRecursos *fres;
		std::string rutas;

public:
       using ParseDefs::cambiaEstado;

       ParseDefsRecursos(Scanner *, size_t numLocs = 256,
            bool dbg = false, bool cl = false, bool v = true,
            const std::string &nomFichSal = "",
            const std::string &rutas = "");

       void procEntrada(void);
};

} // de namespace txtPAWS


// Interfaz IU  ================================================================
extern void porPantalla(const std::string &);
extern void debOut(const std::string &);
extern void porPantallaError(const std::string &);
extern void muestraProceso(TxtPAWS::Scanner *, TxtPAWS::Parser *);
extern int  procesarAventura(int argc, char *argv[]);
extern void procError(const TxtPAWS::Error &e, TxtPAWS::Scanner *sce, TxtPAWS::Parser *p);
