/*
   Date:         September 22, 2014
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#ifndef __Network_Port_hpp__
#define __Network_Port_hpp__

#include <string>

namespace al { namespace network
{
    /*!
     * This class provides an interface to a generic port used for communication.
     * It doesn't actually implement much of use. There is an internal buffer allocated
     * for making reading to a string a bit easier.
     */
    class Port
    {
        public:
            Port( void ) : read_buffer(0), read_buffer_size(0) { }
            ~Port( void ) { if (read_buffer) delete [] read_buffer; }

            /*!
             * Read available data into a buffer.
             *
             * @param buffer The buffer to read in to.
             * @param buffer_size The size of the buffer.
             * @return The number of bytes read.
             */
            virtual int read( char buffer[], const int buffer_size ) = 0;

            /*!
             * Read available data into a string. Internally this calls the more generic
             * read function previously declared so it is not necessary to reimplement this.
             *
             * @param buffer The string to read in to.
             * @param max_read_size The maximum amount of data to read in. A value less
             *          than zero will attempt to read in all available data.
             * @return The number of bytes actually read in.
             */
            virtual int read( std::string & buffer, const int max_read_size = -1 );


            /*!
             * Write data from a buffer.
             *
             * @param buffer The data to write from.
             * @param buffer_size The size of the buffer.
             * @return The number of bytes written.
             */
            virtual int write( const char buffer[], const int buffer_size ) = 0;

            /*!
             * Write data from a string. Internallyt this calls the more generic write
             * function previously declared so it is not necessary to reimplement this.
             *
             * @param buffer The string buffer to write from.
             * @return The number of bytes written.
             */
            virtual int write( const std::string & buffer ) { return write(buffer.c_str(), buffer.size()); }

        private:
            char *  read_buffer;
            int     read_buffer_size;
    };
} }

#endif
