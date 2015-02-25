/*
   Date:         February 25, 2015
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#ifndef __SRL_Log_hpp__
#define __SRL_Log_hpp__

#include <cstdio>
#include <cstdarg>
#include <vector>
#include <string>
#include <map>
#include <cassert>

#include "concurrent/Time.hpp"

namespace al { namespace srl
{
    /*!
     * This class implements a simple log interface.
     */
    class Log
    {
        public:
            enum LogLevel
            {
                Info,
                Warning,
                Error,
                Debug
            };

            static const std::vector<LogLevel> AllLevels;
            static const std::vector<LogLevel> NotErrors;

        public:
            Log( void )
            {
                level_files[Info]       = {};
                level_files[Warning]    = {};
                level_files[Error]      = {};
                level_files[Debug]      = {};
            }

            ~Log( void )
            {
                // Free any managed files.
                for (int i = 0; i < files.size(); ++i)
                {
                    if (files[i].managed && files[i].file)
                        fclose(files[i].file);
                }
            }

            /*!
             * This log will append to the given handle.
             *
             * @param output The file handle to append to.
             * @param levels The log levels which should be written to this handle.
             * @param managed True if managed, false otherwise. If managed, the
             *          log will call fclose on the file handle when it is destroyed.
             * @return True if added handle successfully, false otherwise.
             */
            bool append_to_handle(
                    FILE * const output,
                    const std::vector<LogLevel> levels = AllLevels,
                    const bool managed = false )
            {
                if (output) append(output, levels, managed);
                return output != NULL;
            }

            /*!
             * This log will append to the given file.
             *
             * @param output The file name to be written to.
             * @param levels The log levels which should be written to this handle.
             * @return True if added handle successfully, false otherwise.
             */
            bool append_to_file(
                    const std::string file_name,
                    const std::vector<LogLevel> levels = AllLevels )
            {
                FILE * f = fopen(file_name.c_str(), "a");
                if (f) append(f, levels, true);
                return f != NULL;
            }
            
            /*!
             * Write a message to the logs.
             *
             * @param level The message level.
             * @param format The message printf format.
             * @param ... The printf arguments.
             */
            void write( const LogLevel level, std::string format, ... )
            {
                assert((level == Info) || (level == Warning) || (level == Error) || (level == Debug));

                // Format time.
                time_t      timer;
                struct tm * tm_info;
                char        time_buffer[25];
                time(&timer);
                tm_info = localtime(&timer);
                strftime(time_buffer, 25, "%Y-%m-%d\t%H:%M:%S\t\0", tm_info);
                std::string current_time = time_buffer;

                switch (level)
                {
                    case Warning: format = current_time + "*Warning* " + format; break;
                    case Error  : format = current_time + "!*ERROR*! " + format; break;
                    case Debug  : format = current_time + "--Debug-- " + format; break;
                    default     : format = current_time + format; break;
                }
                format += "\n";

                va_list arg_ptr;
                for (int i = 0; i < level_files[level].size(); ++i)
                {
                    va_start(arg_ptr, format);
                    vfprintf(level_files[level][i], format.c_str(), arg_ptr);
                    va_end(arg_ptr);
                }
            }

        private:
            void append(
                    FILE * const output,
                    const std::vector<LogLevel> levels,
                    const bool managed )
            {
                if (output)
                {
                    files.push_back(FileDescriptor(output, managed));
                    for (int i = 0; i < levels.size(); ++i)
                        level_files[levels[i]].push_back(output);
                }
            }

        private:
            struct FileDescriptor
            {
                FILE * file;
                bool managed;

                FileDescriptor( void ) : file(0), managed(false) { }
                FileDescriptor( FILE * const file, const bool managed ) : file(file), managed(managed) { }
            };

        private:
            std::vector<FileDescriptor> files;
            std::map<LogLevel, std::vector<FILE *>> level_files;
    };
} }

#endif
