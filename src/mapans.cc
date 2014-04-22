// Implementación de la plantilla "mapaNombreSust"

template <typename T>
MapaNombreSust<T>::MapaNombreSust(const std::string &n, size_t lim) : MapaSust(lim)
{
        id = n;
}

template <typename T>
void MapaNombreSust<T>::insrtEntrada(const T &num, const std::string &etq, size_t nl)
{
	Info aux;

	if ( num > getLimite() ) {
		throw ErrorSemantico( "Límite superado en " + getID() );
	}

	aux.numLinea = nl;
	aux.info     = num;

  	mapa[ etq ]  = aux;
}

template <>
void MapaNombreSust<std::string>::insrtEntrada(const std::string &num, const std::string &etq, size_t nl)
{
	Info aux;

	aux.numLinea = nl;
	aux.info     = num;

  	mapa[etq]    = aux;
}

template <typename T>
T  MapaNombreSust<T>::buscaEntrada(const std::string &etq)
{
  	itact = mapa.find( etq );

    return itact->second.info;
}

template <typename T>
size_t  MapaNombreSust<T>::getNumLinea(const std::string &etq)
{
    unsigned int toret = std::numeric_limits<unsigned int>::max();
  	itact = mapa.find( etq );

    if ( itact != mapa.end() ) {
  	    toret = itact->second.numLinea;
  	}

    return toret;
}

template <typename T>
std::string MapaNombreSust<T>::buscaEntradaStr(const std::string &etq)
{
	std::string toret;
  	itact = mapa.find( etq );
	std::ostringstream buffer;

	if ( itact != mapa.end() ) {
  	    buffer << itact->second.toString();
  	}

    return buffer.str();
}

template <typename T>
void MapaNombreSust<T>::devIds(ListaIds &l) const
{
    typename mapNS::const_iterator it = mapa.begin();

    for(; it != mapa.end(); ++it) {
        l.push_back( &( it->first ) );
    }
}


template <typename T>
const typename MapaNombreSust<T>::Info &
MapaNombreSust<T>::buscaEntradaCompleta(const std::string &etq)
{
  	itact = mapa.find( etq );

    return itact->second;
}

template <typename T>
void MapaNombreSust<T>::dump(OutputFile &f) {
    std::ostringstream lineaFormateada;
    typename mapNS::const_iterator it = mapa.begin();

    f.writeLn(linSeparadora + "\n");
    while(it != mapa.end())
    {
    lineaFormateada.str("");
            lineaFormateada
        << (it->first) << " = " << (it->second.info)
        << " (línea "           << (it->second.numLinea) << ')'
        << '\n';

            f.writeLn(lineaFormateada.str());
            ++it;
    }

    f.writeLn(linSeparadora + "\n");
}

template <typename T>
void MapaNombreSust<T>::iraPrimeraEntrada()
{
    itact = mapa.begin();
}

template <typename T>
void MapaNombreSust<T>::iraSigEntrada()
{
    ++itact;
}

template <typename T>
bool MapaNombreSust<T>::esUltimaEntrada()
{
    return itact == mapa.end();
}

template <typename T>
const std::string &MapaNombreSust<T>::getEtqEntradaAct() const
{
    return ( itact->first );
}

template <typename T>
const typename MapaNombreSust<T>::Info &MapaNombreSust<T>::getInfoEntradaAct() const
{
	return ( itact->second );
}

template <typename T>
const T &MapaNombreSust<T>::getValorEntradaAct() const
{
    return ( itact->second.info );
}

template <typename T>
std::string MapaNombreSust<T>::getValorEntradaActString() const
{
	std::ostringstream buffer;

	buffer << (itact->second.info);

    return buffer.str();
}

template <typename T>
size_t MapaNombreSust<T>::getNumLineaEntradaAct() const
{
    return ( itact->second.numLinea );
}

template <typename T>
bool MapaNombreSust<T>::existeEntrada(const std::string &etq)
{
    bool toret = true;
  	itact = mapa.find( etq );

  	if ( itact == mapa.end() ) {
        toret = false;
    }

    return toret;
}

template <typename T>
bool MapaNombreSust<T>::compruebaSinRepeticionesEn()
/*
	Recorre todas las etiquetas, y mete sus valores en una cadena.
	Antes de introducir, comprueba si el nuevo valor ya existe.
*/
{
	bool toret = true;
	std::set<T> coleccValores;
	const T * x;

	iraPrimeraEntrada();

	while ( !esUltimaEntrada() )
	{
	    x = &getValorEntradaAct();

		if ( coleccValores.find( *x ) == coleccValores.end() )
		{
			coleccValores.insert( *x );
		}
		else { toret = false; break; }

		iraSigEntrada();
	}

	return toret;
}

template <typename T>
bool MapaNombreSust<T>::buscaPrimeraEntradaConValor(const T &x)
{
	bool toret = false;

	iraPrimeraEntrada();

	while ( !esUltimaEntrada() )
	{
		if ( getValorEntradaAct() == x )
		{
			toret = true;
			break;
		}

		iraSigEntrada();
	}

	return toret;
}

template <typename T>
void MapaNombreSust<T>::insertaMapa(MapaNombreSust<T> &t)
{
    t.iraPrimeraEntrada();

    while( !t.esUltimaEntrada() ) {
        this->insrtEntrada( t.getValorEntradaAct(), t.getEtqEntradaAct(), t.getNumLineaEntradaAct() );

        t.iraSigEntrada();
    }
}

// ----------------------------------------------------------------------- Tabla
template <typename T>
bool Tabla<T>::chkId(const std::string &identificador)
{
    std::string aviso;
    std::string id = StringMan::trim( identificador );

    if ( isToVerify() ) {
        const std::string * prefix =
                    PrefixMixin::getSuitablePrefix( MapaNombreSust<T>::getID() )
        ;

        if ( prefix != NULL ) {
            const unsigned int lenPrefix = prefix->length();

            if ( id.length() > prefix->length() ) {
                const char firstLetterAfterPrefix = id[ lenPrefix ];

                if ( id.compare( 0, lenPrefix, *prefix ) == 0
                  && std::isalpha( firstLetterAfterPrefix  )
                  && std::isupper( firstLetterAfterPrefix ) )
                {
                    return true;
                }
            }
            // Create warnings
            aviso = strMsg[ WARNING ]
                + strMsg[ INVALIDID ]
                + identificador
                + "' "
                + strMsg[ SHOULDSTARTWITH ]
                + *prefix
                + "' & " + strMsg[ UPPERCASE ]
                + '\n'
            ;
        }

        // Insertar nuevo warning
        if ( aviso.empty() ) {
            aviso =
                "AVISO el id no es válido: '"
                + identificador
                + "'\n"
            ;
        }

        avisos += aviso;
        return false;
    }

    return true;
}


