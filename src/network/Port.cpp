/*
   Date:         September 22, 2014
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#include "network/Port.hpp"
#include <sstream>

namespace al { namespace network
{
    int Port::read( std::string & buffer, int max_read_size )
    {
        if (max_read_size == 0) return 0;   

        const bool read_all = max_read_size < 0;
        if (read_all) max_read_size = 4096;

        if (read_buffer_size < max_read_size)
        {
            if (read_buffer) delete [] read_buffer;
            read_buffer = new char[max_read_size + 1];
            read_buffer_size = read_buffer ? max_read_size : 0;
        }
        if (!read_buffer) return 0;

        int total_bytes_read = 0;
        int bytes_read = 0;
        std::stringstream ss;
        while ((bytes_read = read(read_buffer, max_read_size)) > 0)
        {
            total_bytes_read += bytes_read;

            read_buffer[bytes_read] = 0;
            ss << buffer << std::string(read_buffer);

            if (!read_all) break;
        }
        buffer = ss.str();

        return total_bytes_read;
    }
} }
