/*
   Date:         September 22, 2014
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#include "concurrent/SharedMemory.hpp"

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>

namespace Concurrent
{
    using namespace boost::interprocess;

    class SharedMemorySpace
    {
        private:
            class SharedMemoryObject
            {
                public:
                    SharedMemoryObject( const std::string name, const int size )
                        : name(name),
                            shared_memory(open_or_create, name.c_str(), read_write)
                    {
                        shared_memory.truncate(size);
                    }

                public:
                    std::string name;
                    shared_memory_object shared_memory;
            };

        public:
            SharedMemorySpace( const std::string name, const int size )
                : shared_memory(name, size),
                    region(shared_memory.shared_memory, read_write)
            { }

            inline void * get_address( void ) const
            {
                return region.get_address();
            }

            inline int get_size( void ) const
            {
                return region.get_size();
            }

        private:
            SharedMemoryObject shared_memory;
            mapped_region region;
    };

    SharedMemory::SharedMemory( const std::string name, const int size )
    {
        space = new SharedMemorySpace(name, size);
    }

    SharedMemory::~SharedMemory( void )
    {
        delete space;
    }

    void * SharedMemory::get_address( void ) const
    {
        return space ? space->get_address() : NULL;
    }

    int SharedMemory::get_size( void ) const
    {
        return space ? space->get_size() : NULL;
    }
}
