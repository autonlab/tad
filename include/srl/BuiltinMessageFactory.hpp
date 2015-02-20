/*
   Date:         February 18, 2015
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#ifndef __SRL_Builtin_Message_Factor_hpp__
#define __SRL_Builtin_Message_Factor_hpp__

/*!
 * This file implements some generators for facilitating message creation.
 */

#include "srl/Message.hpp"

namespace al { namespace srl
{
    /*!
     * This class allows generation of error messages.
     */
    class ErrorMessageFactory
    {
        public:
            /*!
             * Generate an error message.
             *
             * @param original_message The original message that caused the error. It will be includes
             *          as part of the body.
             * @param error_message The specific message to send along.
             * @return The encoded string message to be sent along.
             */
            static std::string generate(
                    const InterfaceMessage & original_message,
                    const std::string error_message )
            {
                InterfaceMessage message(original_message.get_module(), original_message.get_service());
                message["error"] = error_message;
                message["original-message"] = static_cast<Field>(original_message.get_wrapper());
                return message.encode();
            }
    };

    /*!
     * This class generates a generic status message.
     */
    class StatusMessageFactory
    {
        public:
            /*!
             * Generate a status message.
             *
             * @param original_message The original message. It will be includes as part of the body.
             * @param status_message The specific message to send along.
             * @return The encoded string message to be sent along.
             */
            static std::string generate(
                    const InterfaceMessage & original_message,
                    const std::string status_message )
            {
                InterfaceMessage message(original_message.get_module(), original_message.get_service());
                message["status"] = status_message;
                message["original-message"] = static_cast<Field>(original_message.get_wrapper());
                return message.encode();
            }
    };

    /*!
     * This class generates a no-op message. This does nothing, but allows clients and services
     * who wish to maintain a connection the ability to reset their idle period so their
     * connection is not dropped by the server.
     */
    class NoOpMessageFactory
    {
        public:
            /*!
             * Generate a no-op message.
             *
             * @return The encoded string message to be sent along.
             */
            static std::string generate( void )
            {
                InterfaceMessage message("Builtin", "NoOp");
                return message.encode();
            }
    };

    /*!
     * This class generates a disconnect message, sent to providers who are disconnected.
     */
    class DisconnectMessageFactory
    {
        public:
            /*!
             * Generate a shutdown message.
             *
             * @param reason The reason the provider is being disconnected.
             * @return The encoded string message to be sent along.
             */
            static std::string generate( const std::string reason )
            {
                InterfaceMessage message("Builtin", "Disconnect");
                message["disconnect-reason"] = reason;
                return message.encode();
            }
    };

    /*!
     * This class generates a shutdown message sent to providers when the serve is shut down.
     */
    class ShutdownMessageFactory
    {
        public:
            /*!
             * Generate a shutdown message.
             *
             * @return The encoded string message to be sent along.
             */
            static std::string generate( void )
            {
                InterfaceMessage message("Builtin", "Shutdown");
                return message.encode();
            }
    };

    /*!
     * This class alows generation of and extraction from RegisterService messages.
     */
    class RegisterServiceMessageFactory
    {
        public:
            /*!
             * Generate a message.
             *
             * @param provider_name The provider of the services.
             * @param services A list of the services to register.
             * @return The encoded string message to send along.
             */
            static std::string generate(
                    const std::string provider_name,
                    const std::vector<std::string> services )
            {
                InterfaceMessage message("Builtin", "RegisterService");
                message["provider-name"] = provider_name;
                message["services"] = services;
                return message.encode();
            }

            /*!
             * Parse a RegisterService message.
             *
             * @param message The decodes message to parse.
             * @param [out] provider_name The provider name.
             * @param [out] services The services to register.
             * @return True if the message was successfully parsed. False otherwise.
             * @note The output parameters may be modified when the parse fails, depending
             *          on when it fails.
             */
            static bool parse(
                    const InterfaceMessage & message,
                    std::string & provider_name,
                    std::vector<std::string> & services )
            {
                bool parse_error = false;
                if (message["provider-name"].is_string() && message["services"].is_array())
                {
                    provider_name = message["provider-name"].string();
                    services.resize(message["services"].size());
                    for (int i = 0; i < message["services"].size(); ++i)
                    {
                        if (message["services"][i].is_string())
                            services[i] = message["services"][i].string();
                        else
                        {
                            parse_error = true;
                            break;
                        }
                    }
                } else parse_error = true;
                
                return !parse_error;
            }
    };
} }

#endif
