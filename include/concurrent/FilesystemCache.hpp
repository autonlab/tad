/*
   Date:         September 22, 2014
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#ifndef __Concurrent_Filesystem_Cache_hpp__
#define __Concurrent_Filesystem_Cache_hpp__

#include "concurrent/Cache.hpp"

namespace al { namespace concurrent
{
    /*!
     * This creates a filesystem client to act as a cache. A filesystem must exist.
     */
    class FilesystemCache : public Cache
    {
        public:
            /*!
             * Create a filesystem shared cache.
             * @param directory The directory to store cache objects. It must already
             *          exist or this will fail!
             */
            FilesystemCache( const std::string directory );
            virtual ~FilesystemCache( void );

            /*!
             * For these methods @see Cache.
             */

            virtual bool initialize( void );
            virtual void cleanup( void );

            virtual bool key_exists( const std::string key ) const;
            virtual std::vector<std::string> find_keys( const std::string pattern ) const;
            virtual std::string get( const std::string key ) const;
            virtual std::vector<char> get_binary( const std::string key ) const;
            virtual bool set( const std::string key, const std::string value );
            virtual bool set_binary( const std::string key, const std::vector<char> value );
            virtual bool exclusive_set( const std::string key, const std::string value );
            virtual bool remove( const std::string key );

        private:
            std::string directory;
    };
} }

#endif
