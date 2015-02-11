/*
   Date:         September 22, 2014
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#ifndef __LIB_CONCURRENT_SHARED_MEMORY_HPP__
#define __LIB_CONCURRENT_SHARED_MEMORY_HPP__

#include <string>

namespace Concurrent
{
    /*!
     * This is a shared memory space object. It's useful for defining spaces of
     * memory to be shared across processes.
     */
    class SharedMemory
    {
        public:
            /*!
             * Create the shared memory space.
             * @param name The name of the shared memory space. This acts as a
             *          sort of discriptor so other processes can attach to the
             *          same space.
             * @param size The size of the memory space. This cannot change during
             *          runtime or the space in other processes may be invalidated.
             */
            SharedMemory( const std::string name, const int size );

            ~SharedMemory( void );

            /*!
             * Determine the address of the shared memory space.
             * @return The memory address of the shared space.
             */
            void * get_address( void ) const;

            /*!
             * Determine the size of a shared memory space.
             * @return The size (in bytes) of a shared memory space.
             */
            int get_size( void ) const;

        private:
            class SharedMemorySpace *space;
    };
}

#endif
