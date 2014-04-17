/*
    Classes for simple input and output
    text files.
*/

#ifndef FILEIO_H_INCLUDED
#define FILEIO_H_INCLUDED

#include "osman.h"

#include <cstdio>
#include <string>
#include <ctime>

#ifdef OS_WINDOWS
    #include <windows.h>
#else
    #include <sys/stat.h>
#endif

class FileIO {
protected:
    std::FILE * f;
private:
    std::string fileName;
    bool access;
public:
    static const bool Read  = false;
    static const bool Write = true;

    const FILE * getHandle() const
        { return f; }
    const std::string &getFileName() const
        { return fileName; }
    bool isOpen() const
        { return ( f != NULL ); }
    bool isEof() const
        { return isOpen() ? feof( f ) : true; }
    bool isWritable() const
        { return ( access == Write ); }

    virtual bool open(const std::string &fn, bool acc)
        {
          fileName = fn;
          access = acc;

          if ( isOpen() ) {
              close();
          }

          if ( access == Write )
                f = std::fopen( fn.c_str(), "wt" );
          else  f = std::fopen( fn.c_str(), "rt" );

          return isOpen();
        }

    virtual void close()
        {
          if ( isOpen() ) {
                fclose( f );
                f = NULL;
          }
        }

    FileIO() : f(NULL)
        {}
    FileIO(const std::string &fn, bool acc) : f(NULL), fileName(fn), access(acc)
        { open( fn, acc ); }
    virtual ~FileIO()
        { close(); }

    void reset()
        { fseek( f, 0, SEEK_SET ); }
    unsigned long int getPos() const
        { return ftell( f ); }
    unsigned long int getSize()
        {
          unsigned long int pos = getPos();
          fseek( f, 0, SEEK_END );
          unsigned long int size = getPos();
          fseek( f, pos, SEEK_SET );

          return size;
        }
    void flush()
        {
          if ( isOpen() ) {
            fflush( f );
          }
        }
    time_t getTimeStamp() const
        {
            time_t toret = std::time( NULL );

            #ifdef OS_WINDOWS
                WIN32_FIND_DATA file;
                SYSTEMTIME utcTime;

                // Look for file
                HANDLE f = FindFirstFile( getFileName().c_str(), &file );

                if ( f != INVALID_HANDLE_VALUE ) {
                    // Get windows time stamp
                    FileTimeToSystemTime( &( file.ftLastWriteTime ), &utcTime );
                    CloseHandle( f );

                    // Convert to time_t
                    LARGE_INTEGER jan1970FT;
                    jan1970FT.u.LowPart = 0;
                    jan1970FT.u.HighPart = 116444736;

                    LARGE_INTEGER utcFT;
                    SystemTimeToFileTime( &utcTime, (FILETIME *) &utcFT );

                    unsigned __int64 utcUnixTime = ( utcFT.QuadPart - jan1970FT.QuadPart ) / 10000000;
                    toret = (time_t) utcUnixTime;
                }
            #else
                if ( isOpen() ) {
                    struct stat status;
                    fstat( fileno( f ), &status );
                    toret = status.st_mtime;
                }
            #endif

            return toret;
        }
};

class InputFile : public FileIO {
public:
    InputFile() : FileIO()
        {}
    InputFile(const std::string &fn) : FileIO( fn, Read )
        {}
    virtual bool open(const std::string &fn)
        { return FileIO::open( fn, Read ); }
    virtual int readChar()
        { return isOpen() ? fgetc( f ) : EOF; }
    virtual std::string &readLn(std::string &txt, int delim = '\n')
        {
            int ch;
            txt.clear();

            ch = readChar();

            while( ch != EOF
                && ch != delim )
            {
                txt.append( (char *) &ch, 1 );

                ch = readChar();
            }

            return txt;
        }
};

class OutputFile : public FileIO {
public:
    OutputFile() : FileIO()
        {}
    OutputFile(const std::string &fn) : FileIO( fn, Write )
        {}
    bool open(const std::string &fn)
        { return FileIO::open( fn, Write ); }

    virtual void write(const std::string &txt)
        {
            if ( isOpen() ) {
                fprintf( f, "%s", txt.c_str() );
            }
        }
    virtual void writeLn(const std::string &txt = "")
        {
            if ( isOpen() ) {
                fprintf( f, "%s\n", txt.c_str() );
            }
        }
};

#endif // FILEIO_H_INCLUDED
