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
            Field( Json::Value * value ) : value(value) { }
            Field( Json::Value & value ) : value(&value) { }

            bool is_empty( void ) const { return value->isNull(); }
            bool is_logical( void ) const { return value->isBool(); }
            bool is_integer( void ) const { return value->isInt() || value->isUInt() || value->isInt64(); }
            bool is_real( void ) const { return value->isDouble(); }
            bool is_string( void ) const { return value->isString(); }
            bool is_array( void ) const { return value->isArray(); }
            bool is_object( void ) const { return value->isObject(); }

            template <class T>
            Field & operator=( const T new_value ) { value = new_value; return *this; }
            /*
            Field & operator=( const long value )
                { *this->value = static_cast<Json::Int64>(value); return *this; }
            Field & operator=( const char value ) { return operator=(static_cast<long>(value)); }
            Field & operator=( const unsigned char value ) { return operator=(static_cast<long>(value)); }
            Field & operator=( const short value ) { return operator=(static_cast<long>(value)); }
            Field & operator=( const unsigned short value ) { return operator=(static_cast<long>(value)); }
            Field & operator=( const int value ) { return operator=(static_cast<long>(value)); }
            Field & operator=( const unsigned int value ) { return operator=(static_cast<long>(value)); }
            Field & operator=( const double value ) { *this->value = value; return *this; }
            */
            Field & operator=( const std::string value ) { *this->value = value; return *this; }
            Field & operator=( const char * const value ) { *this->value = value; return *this; }

            Field & operator=( const Field & value ) { *this->value = *value.value; return *this; }

            template <class T>
            Field & operator=( const std::vector<T> values )
            {
                *value = Json::Value(Json::arrayValue);
                for (int i = 0; i < values.size(); ++i)
                    (*value)[i] = values[i];
                return *this;
            }

            Field get( const std::string index ) const { return Field((*value)[index]); }
            Field get( const int index ) const { return Field((*value)[index]); }

            Field operator[]( const std::string index ) const { return get(index); }
            Field operator[]( const int index ) const { return get(index); }

            Field create( const std::string name, const bool field_value )
            {
                (*value)[name] = field_value;
                return Field((*value)[name]);
            }

            Field create( const std::string name, const long field_value )
            {
                (*value)[name] = static_cast<Json::Int64>(field_value);
                return Field((*value)[name]);
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
                (*value)[name] = field_value;
                return Field((*value)[name]);
            }

            Field create( const std::string name, const std::string field_value )
            {
                (*value)[name] = field_value;
                return Field((*value)[name]);
            }

            Field create_array( const std::string name )
                { return Field((*value)[name] = Json::Value(Json::arrayValue)); }

            Field create_object( const std::string name )
                { return Field((*value)[name] = Json::Value(Json::objectValue)); }

            Field create_null( const std::string name )
                { return Field((*value)[name]); }

            bool logical( void ) const { return value->asBool(); }
            long integer( void ) const { return value->asInt64(); }
            double real( void ) const { return value->asDouble(); }
            std::string string( void ) const { return value->asString(); }
            int size( void ) const { return is_array() ? value->size() : 0; }

            Json::Value * get_raw_value( void ) const { return value; }
            void set_raw_value( Json::Value * const new_value ) { value = new_value; }
            void set_raw_value( Json::Value & new_value ) { set_raw_value(&new_value); }
            void set_raw_value( const Field & field ) { set_raw_value(field.value); }

        protected:
            Json::Value * value;
    };

    class GenericMessage : public Field
    {
        public:
            GenericMessage( void ) : Field(message), raw_message("") { }

            virtual std::string encode( void ) const { return writer.write(message); }
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
            Json::Value message;
            Json::Reader reader;
            mutable Json::StyledWriter writer;
    };

    class InterfaceMessage : public GenericMessage
    {
        public:
            InterfaceMessage( const std::string module = "", const std::string service = "" )
            {
                wrapper.create("protocol-version", 1000);
                wrapper.create("module", module);
                wrapper.create("service", service);
                wrapper.create_object("body");
                set_raw_value(wrapper["body"]);
            }

            int get_protocol_version( void ) const
                { return wrapper["protocol-version"].is_integer() ? wrapper["protocol-version"].integer() : 0; }
            bool is_protocol_version_supported( void ) const
                { return get_protocol_version() == 1000; }

            std::string get_module( void ) const
                { return wrapper["module"].is_string() ? wrapper["module"].string() : ""; }
            std::string get_service( void ) const
                { return wrapper["service"].is_string() ? wrapper["service"].string() : ""; }

            virtual std::string encode( void ) const { return wrapper.encode(); }
            virtual bool decode( const std::string raw_message )
            {
                if (wrapper.decode(raw_message))
                {
                    set_raw_value(wrapper["body"]);
                    return true;
                } else return false;
            }

            const GenericMessage & get_wrapper( void ) const { return wrapper; }

        private:
            GenericMessage wrapper;
    };
} }

#endif
