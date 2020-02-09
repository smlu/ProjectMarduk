#ifndef LIBIM_TEXT_TOKEN_H
#define LIBIM_TEXT_TOKEN_H
#include <string>
#include <sstream>
#include <type_traits>

#include "diagnostic_location.h"
#include "tokenizer_error.h"

namespace libim::text {
    class Token final
    {
    public:
        enum Type
        {
            Invalid = -1,
            EndOfFile,
            EndOfLine,
            Identifier,
            String,
            Punctuator,
            Integer,
            HexInteger,
            OctInteger,
            FloatNumber
        };

        Token() = default;
        Token(Type type, std::string value) :
            m_type(type),
            m_value(std::move(value))
        {}

        Token(Type type, std::string value, diagnostic_location loc) :
            m_type(type),
            m_value(std::move(value)),
            m_loc(std::move(loc))
        {}

        void append(char c)
        {
            m_value.push_back(c);
        }

        inline void clear()
        {
            m_type = Invalid;
            m_value.clear();
        }

        bool isEmpty() const
        {
            return m_value.empty();
        }

        inline bool isNumber() const
        {
            return m_type == Integer    ||
                   m_type == HexInteger ||
                   m_type == OctInteger ||
                   m_type == FloatNumber;
        }

        void setLocation(diagnostic_location loc)
        {
            m_loc = std::move(loc);
        }

        const diagnostic_location& location() const
        {
            return m_loc;
        }

        diagnostic_location& location()
        {
            return m_loc;
        }

        void reserve(std::size_t len)
        {
            m_value.reserve(len);
        }

        void setValue(std::string value)
        {
            m_value = std::move(value);
        }

        const std::string& value() const &
        {
            return m_value;
        }

        const std::string& value() const &&
        {
            return m_value;
        }

        std::string value() && noexcept
        {
            m_type = Invalid;
            return std::move(m_value);
        }

        void setType(Type type)
        {
            m_type = type;
        }

        Type type() const
        {
            return m_type;
        }

        template<typename T, typename DT = std::decay_t<T>>
        DT getNumber() const
        {
            static_assert(std::is_arithmetic_v<DT>, "T is not a arithmetic type");

            DT result = DT(0);
            std::stringstream ss(m_value);
            switch(m_type)
            {
                case OctInteger: {
                    ss >> std::oct;
                } break;
                case HexInteger: {
                    ss >> std::hex;
                } break;
                default:
                    break;
            }

            ss >> result;
            if(!isNumber() || ss.fail())
            {
                using namespace std::string_view_literals;
                throw TokenizerError("invalid numeric conversion from string"sv, m_loc);
            }

            return result;
         }

    private:
        Type m_type = Invalid;
        std::string m_value;
        diagnostic_location m_loc;
    };

}
#endif // LIBIM_TEXT_TOKEN_H