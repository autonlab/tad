#ifndef __SRL_Builtin_Message_Factor_hpp__
#define __SRL_Builtin_Message_Factor_hpp__

#include "srl/Message.hpp"

namespace al { namespace srl
{
    class ErrorMessageFactory
    {
        public:
            static std::string generate(
                    const InterfaceMessage & original_message,
                    const std::string error_message )
            {
                InterfaceMessage message(original_message.get_module(), original_message.get_service());
                message["error"] = error_message;
                message["original-body"] = static_cast<Field>(original_message.get_wrapper());
                return message.encode();
            }
    };

    class StatusMessageFactory
    {
        public:
            static std::string generate(
                    const InterfaceMessage & original_message,
                    const std::string status_message )
            {
                InterfaceMessage message(original_message.get_module(), original_message.get_service());
                message["status"] = status_message;
                message["original-body"] = static_cast<Field>(original_message);
                return message.encode();
            }
    };

    class RegisterServiceMessageFactory
    {
        public:
            static std::string generate(
                    const std::string provider_name,
                    const std::vector<std::string> services )
            {
                InterfaceMessage message("Builtin", "RegisterService");
                message["provider-name"] = provider_name;
                message["services"] = services;
                return message.encode();
            }

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
