#ifndef EXCEP_INCLUDED
#define EXCEP_INCLUDED


namespace TxtPAWS {

#include <stdexcept>

// ----------------------------------------------------------------- Excepciones

class Error : public std::runtime_error {
protected:
	Mensaje error;
	std::string details;
	size_t numLin;
	Error(Mensaje x, const std::string &d = strMsg[ NOMSG ], size_t nl = MaxLineas)
            : runtime_error( d.c_str() ), error( x ), details( d ), numLin( nl )
        {}
public:
    static const size_t MaxLineas;

    bool hayNumLin() const
        { return ( numLin != MaxLineas ); }
    size_t getNumLin() const
        { return numLin; }
	const std::string &getDetails() const
		{ return details; }
	const char * what() const throw()
		{ return strMsg[ error].c_str(); }
	Error()
        : runtime_error( strMsg[ NOMSG ] ),
        error( UNDEFINED ), numLin( MaxLineas )
		{ details = strMsg[NOMSG]; }
	virtual ~Error() throw() {}
};

const size_t Error::MaxLineas = std::numeric_limits<size_t>::max();

class ErrorPreProc : public Error {
public:
	ErrorPreProc(const std::string &d = strMsg[NOMSG], size_t nl = MaxLineas)
	: Error(PREPROC, d, nl) {}
};

class ErrorSemantico : public Error {
public:
	ErrorSemantico(const std::string &d = strMsg[NOMSG], size_t nl = MaxLineas)
	: Error(NONSENSE, d, nl)
        {}
    ErrorSemantico(Mensaje error, const std::string &d = strMsg[NOMSG], size_t nl = MaxLineas )
    : Error(error, d, nl)
        {}
};

class ErrorIdDuplicado : public ErrorSemantico {
public:
	ErrorIdDuplicado(const std::string &d = strMsg[NOMSG], size_t nl = MaxLineas)
	: ErrorSemantico( DUP, d, nl ) {}
};

class ErrorInterno : public Error {
public:
	ErrorInterno(const std::string &d = strMsg[NOMSG], size_t nl = MaxLineas)
	: Error(INTERNAL, d, nl) {}
};

class ErrorFichEntrada : public Error {
public:
	ErrorFichEntrada(const std::string &d = strMsg[NOMSG], size_t nl = MaxLineas)
	: Error(INPUT, d, nl) {}
};

class ErrorSintaxis : public Error {
protected:
	ErrorSintaxis(Mensaje msg, const std::string &d = strMsg[NOMSG], size_t nl = MaxLineas)
	: Error(msg, d, nl) {}
public:
	ErrorSintaxis(const std::string &d = strMsg[NOMSG], size_t nl = MaxLineas)
	: Error(SYNTAX, d, nl) {}
};

class SeccError : public ErrorSintaxis {
public:
	SeccError(const std::string &d = strMsg[NOMSG], size_t nl = MaxLineas)
	: ErrorSintaxis(UNORDERED, d, nl) {}
};

class MsgYaExisteError : public ErrorSintaxis {
public:
	MsgYaExisteError(const std::string &d = strMsg[NOMSG], size_t nl = MaxLineas)
	: ErrorSintaxis(EXISTING, d, nl) {}
};

class MsgNoExisteError : public ErrorSintaxis {
public:
	MsgNoExisteError(const std::string &d = strMsg[NOMSG], size_t nl = MaxLineas)
	: ErrorSintaxis(INEXISTING, d, nl) {}
};

class ColocaError : public ErrorSintaxis {
protected:
	ColocaError(Mensaje msg, const std::string &d = strMsg[NOMSG], size_t nl = MaxLineas)
		: ErrorSintaxis(msg, d, nl)
	{}
public:
	ColocaError(const std::string &d = strMsg[NOMSG]) : ErrorSintaxis(LOCATION, d) {}
};

class SeccionColocaError : public ColocaError {
public:
	SeccionColocaError(const std::string &d = strMsg[NOMSG], size_t nl = MaxLineas)
	: ColocaError(UNORDERED, d, nl) {}
};

class NumMsgColocaError : public ColocaError {
public:
	NumMsgColocaError(const std::string &d = strMsg[NOMSG], size_t nl = MaxLineas)
	: ColocaError(DUP, d, nl) {}
};

}

#endif // EXCEP_INCLUDED
