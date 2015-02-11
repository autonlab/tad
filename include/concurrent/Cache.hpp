/*
   Date:         September 22, 2014
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#ifndef __CACHE_HPP__
#define __CACHE_HPP__

#include <string>
#include <vector>

namespace Concurrent
{
    /*!
     * This is a generic key/value cache interface.
     */
    class Cache
    {
        public:
            Cache( void ) : last_error("") { }

            /*!
             * Initialize the cache.
             * @return True on success, false on failure.
             */
            virtual bool initialize( void ) = 0;

            /*!
             * Cleanup the cache..
             */
            virtual void cleanup( void ) = 0;

            /*!
             * Determine whether or not a key exists.
             * @param key The key to lookup.
             * @return True if the key exists, false otherwise.
             */
            virtual bool key_exists( const std::string key ) const = 0;

            /*!
             * Find a key given the expression. Use * as a wildcard.
             * @param pattern The key name pattern to match.
             * @return A vector of matching key names.
             */
            virtual std::vector<std::string> find_keys( const std::string pattern ) const = 0;

            /*!
             * Returns the value stored at the indicated key.
             * @param key The key to lookup.
             * @return The value at the specified key, or an empty string if non-existant.
             */
            virtual std::string get( const std::string key ) const = 0;

            /*!
             * Returns the binary value stored at the indicated key.
             * @param key The key to lookup.
             * @return The value at the specified key, or an empty vector if non-existant.
             */
            virtual std::vector<char> get_binary( const std::string key ) const = 0;

            /*!
             * Sets the string value at the indicated key. If the entry does not exist,
             * it is created. If it does exist, it is overwritten.
             * @param key The key to lookup.
             * @param value The value to set.
             * @return True if stored successfully, false otherwise.
             */
            virtual bool set( const std::string key, const std::string value ) = 0;

            /*!
             * Sets the binary value at the indicated key. If the entry does not exist,
             * it is created. If it does exist, it is overwritten.
             * @param key The key to lookup.
             * @param value The value to set.
             * @return True if stored successfully, false otherwise.
             */
            virtual bool set_binary(
                    const std::string key,
                    const std::vector<char> value ) = 0;

            /*!
             * Sets the string value at the indicated key only if it does not already
             * exist.
             * @param key The key to lookup.
             * @param value The value to set.
             * @return True if the key didn't previously exist and it was stored
             *          successfully, false otherwise.
             * @note This should be atomic (i.e. two threads or processes setting at the
             *          same time will not run into race conditions using this).
             */
            virtual bool exclusive_set( const std::string key, const std::string value ) = 0;

            /*!
             * Remove the value at the indicated key from the cache.
             * @param key They key to remove.
             * @return True if removed successfully (regardless of whether it was
             *          there initially), false otherwise.
             */
            virtual bool remove( const std::string key ) = 0;

            /*!
             * Returns the last encountered error.
             * @return The last encountered error string, or an empty string if no
             *          errors have been encountered.
             */
            virtual std::string get_last_error( void ) const { return last_error; }

        protected:
            std::string last_error;
    };

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
}

#endif
