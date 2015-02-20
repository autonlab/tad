/*
   Date:         February 18, 2015
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#ifndef __SRL_Message_hpp__
#define __SRL_Message_hpp__

/*!
 * The classes in this file create a standard interface to handling messages in
 * the code so it will be consistent regardless of whether or not a different
 * JSON library is used or even if the message format is changed entirely. It
 * basically acts as a convinience wrapper to make things easier and quicker
 * for lazy people like me. :-)
 */

#include <string>
#include <sstream>
#include <json/json.h>

namespace al { namespace srl
{
    /*!
     * This class implements a field of a message. It provides an interface for
     * determining the field's type, extracting its data, and creating new fields.
     */
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

            Field & operator=( const long value )
                { *this->value = static_cast<Json::Int64>(value); return *this; }
            Field & operator=( const char value ) { return operator=(static_cast<long>(value)); }
            Field & operator=( const unsigned char value ) { return operator=(static_cast<long>(value)); }
            Field & operator=( const short value ) { return operator=(static_cast<long>(value)); }
            Field & operator=( const unsigned short value ) { return operator=(static_cast<long>(value)); }
            Field & operator=( const int value ) { return operator=(static_cast<long>(value)); }
            Field & operator=( const unsigned int value ) { return operator=(static_cast<long>(value)); }
            Field & operator=( const double value ) { *this->value = value; return *this; }
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

            /*!
             * Accessors.
             * @note Depending on the backend used, these may call an assertion if the
             *          accessor used is inconsistent with the data stored (e.g. the
             *          field is null and you try to extract an integer). For that
             *          reason, make sure to call the is_* function first to determine
             *          if the accessor is appropriate.
             * @note wrt the last note, the current backend JsonCpp will assert.
             */

            bool logical( void ) const { return value->asBool(); }
            long integer( void ) const { return value->asInt64(); }
            double real( void ) const { return value->asDouble(); }
            std::string string( void ) const { return value->asString(); }
            int size( void ) const { return is_array() ? value->size() : 0; }

        protected:
            Json::Value * get_raw_value( void ) const { return value; }
            void set_raw_value( Json::Value * const new_value ) { value = new_value; }
            void set_raw_value( Json::Value & new_value ) { set_raw_value(&new_value); }
            void set_raw_value( const Field & field ) { set_raw_value(field.value); }

        protected:
            Json::Value * value;
    };

    /*!
     * This class encapsulates a generic message. It offers the interface for encoding
     * and decoding messages to and from string representations.
     */
    class GenericMessage : public Field
    {
        public:
            GenericMessage( void ) : Field(message), raw_message(""), parsed_okay(false), last_error("") { }

            virtual std::string encode( void ) const { return writer.write(message); }
            virtual bool decode( const std::string raw_message )
            {
                this->raw_message = raw_message;
                parsed_okay = reader.parse(raw_message, message, true);
                if (!parsed_okay) last_error = reader.getFormattedErrorMessages();
                return parsed_okay;
            }

            bool is_valid( void ) const { return parsed_okay; }
            std::string get_last_error( void ) const { return last_error; }

        protected:
            std::string raw_message;
            bool parsed_okay;
            std::string last_error;
            Json::Value message;
            Json::Reader reader;
            mutable Json::StyledWriter writer;
    };

    /*
     * This class implements an interface message, which includes some header information
     * wrapped around the message data. It's used exactly the same as a GenericMessage
     * except that it has custom accessors for header fields like protocol version.
     */
    class InterfaceMessage : public GenericMessage
    {
        public:
            InterfaceMessage(
                    const std::string module = "",
                    const std::string service = "",
                    const int client_id = -1 )
            {
                wrapper.create("protocol-version", 1000);
                wrapper.create("module", module);
                wrapper.create("service", service);
                wrapper.create("client-id", client_id);
                wrapper.create_object("body");
                set_raw_value(wrapper["body"]);
            }

            InterfaceMessage( const InterfaceMessage & original_message )
            {
                wrapper.create("protocol-version", 1000);
                wrapper.create("module", original_message.get_module());
                wrapper.create("service", original_message.get_service());
                wrapper.create("client-id", original_message.get_client_id());
                wrapper.create_object("body");
                set_raw_value(wrapper["body"]);
            }

            int get_protocol_version( void ) const
                { return wrapper["protocol-version"].is_integer() ? wrapper["protocol-version"].integer() : 0; }
            bool is_protocol_version_supported( void ) const
                { return get_protocol_version() == 1000; }

            std::string get_module( void ) const
                { return wrapper["module"].is_string() ? wrapper["module"].string() : ""; }
            void set_module( const std::string module ) { wrapper["module"] = module; }

            std::string get_service( void ) const
                { return wrapper["service"].is_string() ? wrapper["service"].string() : ""; }
            void set_service( const std::string service ) { wrapper["service"] = service; }

            int get_client_id( void ) const
                { return wrapper["client-id"].is_integer() ? wrapper["client-id"].integer() : -1; }
            void set_client_id( const int client_id ) { wrapper["client-id"] = client_id; }

            virtual std::string encode( void ) const { return wrapper.encode(); }
            virtual bool decode( const std::string raw_message )
            {
                if (wrapper.decode(raw_message))
                {
                    set_raw_value(wrapper["body"]);
                    return true;
                }
                else
                {
                    parsed_okay = false;
                    last_error = wrapper.get_last_error();
                    return false;
                }
            }

            const GenericMessage & get_wrapper( void ) const { return wrapper; }

        private:
            GenericMessage wrapper;
    };
} }

#endif
