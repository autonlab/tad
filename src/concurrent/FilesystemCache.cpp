/*
   Date:         September 22, 2014
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#include "concurrent/Cache.hpp"
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string>
#include <fstream>
#include <errno.h>

namespace al { namespace concurrent
{
    namespace __utils
    {
        /* Slightly modified from here:
         * http://www.geeksforgeeks.org/wildcard-character-matching/
         */
        bool wildcard_match(
                const char * pattern,
                const char * test )
        {
            if (*pattern == '\0' && *test == '\0') return true;

            // Make sure that the characters after '*' are present in second string.
            // This function assumes that the first string will not contain two
            // consecutive '*'
            if (*pattern == '*' && *(pattern+1) != '\0' && *test == '\0')
                return false;

            // If the first string contains '?', or current characters of both
            // strings match
            if (*pattern == *test) return wildcard_match(pattern+1, test+1);

            // If there is *, then there are two possibilities
            // a) We consider current character of second string
            // b) We ignore current character of second string.
            if (*pattern == '*')
                return wildcard_match(pattern+1, test) || wildcard_match(pattern, test+1);

            return false;
        }
    }

    FilesystemCache::FilesystemCache( const std::string directory )
        : directory(directory)
    { }

    FilesystemCache::~FilesystemCache( void )
    { }

    bool FilesystemCache::initialize( void )
    {
        // Nothing to do.
        return true;
    }

    void FilesystemCache::cleanup( void )
    {
        // Nothing to do.
    }

    bool FilesystemCache::key_exists( const std::string key ) const
    {
        struct stat buffer;
        std::string filename = directory + key;
        return (stat (filename.c_str(), &buffer) == 0);
    }

    std::vector<std::string> FilesystemCache::find_keys( const std::string pattern ) const
    {
        std::vector<std::string> results;

        DIR           *d    = NULL;
        struct dirent *dir  = NULL;

        d = opendir(directory.c_str());
        if (d)
        {
            while ((dir = readdir(d)) != NULL)
                if (__utils::wildcard_match(pattern.c_str(), dir->d_name))
                    results.push_back(dir->d_name);
            closedir(d);
        }

        return results;
    }

    std::string FilesystemCache::get( const std::string key ) const
    {
        std::vector<char> data = get_binary(key);
        return std::string(&data[0], data.size());
    }

    std::vector<char> FilesystemCache::get_binary( const std::string key ) const
    {
        try
        {
            std::string filename = directory + key;
            std::ifstream ifs(
                    filename.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
            if (!ifs.is_open()) throw 0;

            std::ifstream::pos_type size = ifs.tellg();
            ifs.seekg(0, std::ios::beg);

            std::vector<char> data(size);
            ifs.read(&data[0], size);
            ifs.close();

            return data;
        }
        catch (...)
        {
            return std::vector<char>();
        }
    }

    bool FilesystemCache::set( const std::string key, const std::string value )
    {
        std::vector<char> data(value.begin(), value.end());
        return set_binary(key, data);
    }

    bool FilesystemCache::set_binary( const std::string key, const std::vector<char> value )
    {
        try
        {
            std::string filename = directory + key;
            std::ofstream ofs(
                    filename.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
            if (!ofs.is_open()) throw 0;
            ofs.write(&value[0], value.size());
            ofs.close();
        }
        catch (...)
        {
            return false;
        }

        return true;
    }

    bool FilesystemCache::exclusive_set( const std::string key, const std::string value )
    {
        /* Note: This method of file opening should work to atomically check a
         * file's existance and create it if not already there for most systems
         * except, notably, on NFS.
         */
        std::string filename = directory + key;
        int file_handle = open(
                filename.c_str(), O_WRONLY | O_CREAT | O_EXCL,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (file_handle == -1) return false;
        else
        {
            // File was created. Close and perform an normal set.
            close(file_handle);
            return set(key, value);
        }
    }

    bool FilesystemCache::remove( const std::string key )
    {
        std::string filename = directory + key;
        int result = ::remove(filename.c_str());
        return (result == 0) || (errno == ENOENT);
    }
} }
