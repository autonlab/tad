#include "srl/Message.hpp"

namespace al { namespace srl
{
    const std::string InterfaceMessage::message_type_string[TypeCount] =
    {
        "Unknown",

        "RegisterService",
        "UnregisterService"
    };
} }
