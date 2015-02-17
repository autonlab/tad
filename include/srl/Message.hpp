#ifndef __SRL_Message_hpp__
#define __SRL_Message_hpp__

#include <string>
#include <sstream>
#include <json/json.h>

namespace al { namespace srl
{
    class Field
    {
        public:
            Field( Json::Value & value ) : value(value) { }

            bool is_empty( void ) const { return value.isNull(); }
            bool is_logical( void ) const { return value.isBool(); }
            bool is_integer( void ) const { return value.isInt() || value.isUInt() || value.isInt64(); }
            bool is_real( void ) const { return value.isDouble(); }
            bool is_string( void ) const { return value.isString(); }
            bool is_array( void ) const { return value.isArray(); }
            bool is_object( void ) const { return value.isObject(); }

            Field & operator=( const long value )
                { this->value = static_cast<Json::Int64>(value); return *this; }
            Field & operator=( const char value ) { return operator=(static_cast<long>(value)); }
            Field & operator=( const unsigned char value ) { return operator=(static_cast<long>(value)); }
            Field & operator=( const short value ) { return operator=(static_cast<long>(value)); }
            Field & operator=( const unsigned short value ) { return operator=(static_cast<long>(value)); }
            Field & operator=( const int value ) { return operator=(static_cast<long>(value)); }
            Field & operator=( const unsigned int value ) { return operator=(static_cast<long>(value)); }
            Field & operator=( const double value ) { this->value = value; return *this; }
            Field & operator=( const std::string value ) { this->value = value; return *this; }
            Field & operator=( const Field & value ) { this->value = &value.value; return *this; }

            Field get( const std::string index ) const { return Field(value[index]); }
            Field get( const int index ) const { return Field(value[index]); }

            Field operator[]( const std::string index ) const { return get(index); }
            Field operator[]( const int index ) const { return get(index); }

            Field create( const std::string name, const bool field_value )
            {
                value[name] = field_value;
                return Field(value[name]);
            }

            Field create( const std::string name, const long field_value )
            {
                value[name] = static_cast<Json::Int64>(field_value);
                return Field(value[name]);
            }
            Field create( const std::string name, const char field_value )
                { return create(name, static_cast<long>(field_value)); }
            Field create( const std::string name, const unsigned char field_value )
                { return create(name, static_cast<long>(field_value)); }
            Field create( const std::string name, const short field_value )
                { return create(name, static_cast<long>(field_value)); }
            Field create( const std::string name, const unsigned short field_value )
                { return create(name, static_cast<long>(field_value)); }
            Field create( const std::string name, const int field_value )
                { return create(name, static_cast<long>(field_value)); }
            Field create( const std::string name, const unsigned int field_value )
                { return create(name, static_cast<long>(field_value)); }

            Field create( const std::string name, const double field_value )
            {
                value[name] = field_value;
                return Field(value[name]);
            }

            Field create( const std::string name, const std::string field_value )
            {
                value[name] = field_value;
                return Field(value[name]);
            }

            Field create_array( const std::string name )
                { return Field(value[name] = Json::Value(Json::arrayValue)); }

            Field create_object( const std::string name )
                { return Field(value[name] = Json::Value(Json::objectValue)); }

            Field create_null( const std::string name )
                { return Field(value[name]); }

            bool logical( void ) const { return value.asBool(); }
            long integer( void ) const { return value.asInt64(); }
            double real( void ) const { return value.asDouble(); }
            std::string string( void ) const { return value.asString(); }
            int size( void ) const { return is_array() ? value.size() : 0; }

            Json::Value & get_raw_value( void ) const { return value; }

        protected:
            Json::Value & value;
    };

    class Message : public Field
    {
        public:
            Message( void ) : Message(internal_message) { }
            Message( const Field message ) : Message(message.get_raw_value()) { }
            Message( Json::Value & message ) : message(message), Field(message), raw_message("") { }

            virtual std::string encode( void ) { return writer.write(message); }
            virtual bool decode( const std::string raw_message )
            {
                this->raw_message = raw_message;
                return reader.parse(raw_message, message, true);
            }

            bool is_valid( void ) const { return reader.good(); }
            std::string get_last_error( void ) const
            {
                if (!is_valid()) return reader.getFormattedErrorMessages();
                else return "No errors.";
            }

        protected:
            std::string raw_message;
            Json::Value & message;
            Json::Reader reader;
            Json::StyledWriter writer;

        private:
            Json::Value internal_message;

    };

    class InterfaceMessage : public Message
    {
        public:
            typedef enum
            {
                TypeUnknown             = 0,

                TypeRegisterService     = 1,
                TypeUnregisterService   = 2,

                TypeCount
            } MessageType;

        private:
            static const std::string message_type_string[TypeCount];

        public:
            InterfaceMessage( const MessageType type = TypeUnknown) : Message(wrapper["body"])
            {
                wrapper.create("protocol-version", 1000);
                wrapper.create("type", message_type_string[type]);
            }

            MessageType get_type( void ) const
            {
                // Match string with type ID.
                for (int i = 0; i < TypeCount; ++i)
                    if (message_type_string[i] == wrapper["type"].string())
                        return static_cast<MessageType>(i);

                // No type was matched, return unknown.
                return TypeUnknown;
            }
            std::string get_type_string( void ) const { return message_type_string[get_type()]; }

            int get_protocol_version( void ) const { return wrapper["protocol-version"].integer(); }
            bool is_protocol_version_supported( void ) const
                { return get_protocol_version() == 1000; }

            virtual std::string encode( void ) { return wrapper.encode(); }
            virtual bool decode( const std::string raw_message ) { return wrapper.decode(raw_message); }

        private:
            Message wrapper;
    };

    class RegisterServiceMessage : public InterfaceMessage
    {
        public:
            RegisterServiceMessage( void ) : InterfaceMessage(InterfaceMessage::TypeRegisterService) { }

            void set_services( const std::vector<std::string> services )
            {
                create_array("services");
                Field services_key = get("services");
                for (int i = 0; i < services.size(); ++i)
                    services_key[i] = services[i];
            }

            std::vector<std::string> get_services( void ) const
            {
                Field services_key = get("services");
                std::vector<std::string> services(services_key.size());
                for (int i = 0; i < services_key.size(); ++i)
                    services[i] = services_key[i].string();
                return services;
            }
    };
} }

#endif
