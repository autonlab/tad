/*
   Date:         September 22, 2014
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#include "network/Port.hpp"
#include <sstream>

namespace al { namespace network
{
    int Port::read( std::string & buffer, const int max_read_size )
    {
        if (max_read_size < 1) return 0;
        if (read_buffer_size < max_read_size)
        {
            if (read_buffer) delete [] read_buffer;
            read_buffer = new char[max_read_size + 1];
            read_buffer_size = read_buffer ? max_read_size : 0;
        }
        if (!read_buffer) return 0;

        int bytes_read = read(read_buffer, max_read_size);
        if (bytes_read > 0)
        {
            read_buffer[bytes_read] = 0;

            std::stringstream ss;
            ss << buffer << std::string(read_buffer);
            buffer = ss.str();
        }

        return bytes_read;
    }
} }
