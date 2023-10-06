// @copyright Copyright (c) 2023. Created by Konstantin Belousov.
// All rights reserved.

#include <sstream>
#include "json.h"


namespace json {

    using std::operator""s;
    using std::operator""sv;

    // class Node definitions --------------------------------------------------

    using value = Node::value;

    bool Node::is_int() const {
        return std::holds_alternative<int>(*this);
    }

    int Node::as_int() const {
        if (!is_int()) throw std::logic_error("Not an int"s);
        return std::get<int>(*this);
    }

    bool Node::is_pure_double() const {
        return std::holds_alternative<double>(*this);
    }

    bool Node::is_double() const {
        return is_int() || is_pure_double();
    }

    double Node::as_double() const {
        if (!is_double()) throw std::logic_error("Not a double"s);
        return is_pure_double() ? std::get<double>(*this) : as_int();
    }

    bool Node::is_bool() const {
        return std::holds_alternative<bool>(*this);
    }

    bool Node::as_bool() const {
        if (!is_bool()) throw std::logic_error("Not a bool"s);
        return std::get<bool>(*this);
    }

    bool Node::is_null() const {
        return std::holds_alternative<std::nullptr_t>(*this);
    }

    bool Node::is_array() const {
        return std::holds_alternative<Array>(*this);
    }

    Array& Node::as_array() {
        if (!is_array()) throw std::logic_error("Not an array"s);
        return std::get<Array>(*this);
    }

    const Array& Node::as_array() const {
        if (!is_array()) throw std::logic_error("Not an array"s);
        return std::get<Array>(*this);
    }

    bool Node::is_string() const {
        return std::holds_alternative<std::string>(*this);
    }

    const std::string& Node::as_string() const {
        if (!is_string()) throw std::logic_error("Not a string"s);
        return std::get<std::string>(*this);
    }

    bool Node::is_dict() const {
        return std::holds_alternative<Dict>(*this);
    }

    Dict& Node::as_dict() {
        if (!is_dict()) throw std::logic_error("Not a dict"s);
        return std::get<Dict>(*this);
    }

    const Dict& Node::as_dict() const {
        if (!is_dict()) throw std::logic_error("Not a dict"s);
        return std::get<Dict>(*this);
    }

    const value& Node::get_value() const {
        return *this;
    }

    // class Document definitions --------------------------------------------------

    Document::Document(Node root) : root_(std::move(root)) {}

    const Node& Document::get_root() const {
        return root_;
    }

    namespace {

        Node load_node(std::istream& input);
        Node load_string(std::istream& input);

        std::string load_literal(std::istream& input) {
            std::string s;
            while (std::isalpha(input.peek())) {
                s.push_back(static_cast<char>(input.get()));
            }
            return s;
        }

        Node load_array(std::istream& input) {
            std::vector<Node> result;

            for (char c; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(load_node(input));
            }

            if (!input) throw ParsingError("Array parsing error"s);

            return Node{std::move(result)};
        }

        Node load_dict(std::istream& input) {
            Dict dict;

            for (char c; input >> c && c != '}';) {
                if (c == '"') {
                    std::string key = load_string(input).as_string();
                    if (input >> c && c == ':') {
                        if (dict.find(key) != dict.end()) {
                            throw ParsingError("Duplicate key '"s + key + "' have been found");
                        }
                        dict.emplace(std::move(key), load_node(input));
                    } else {
                        throw ParsingError(": is expected but '"s + c + "' has been found"s);
                    }
                } else if (c != ',') {
                    throw ParsingError(R"(',' is expected but ')"s + c + "' has been found"s);
                }
            }

            if (!input) throw ParsingError("Dictionary parsing error"s);

            return Node{std::move(dict)};
        }

        Node load_string(std::istream& input) {
            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;

            while (true) {
                if (it == end) {
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    ++it;
                    break;
                } else if (ch == '\\') {
                    ++it;
                    if (it == end) {
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    switch (escaped_char) {
                        case 'n':
                            s.push_back('\n');
                            break;
                        case 't':
                            s.push_back('\t');
                            break;
                        case 'r':
                            s.push_back('\r');
                            break;
                        case '"':
                            s.push_back('"');
                            break;
                        case '\\':
                            s.push_back('\\');
                            break;
                        default:
                            throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                } else if (ch == '\n' || ch == '\r') {
                    throw ParsingError("Unexpected end of line"s);
                } else {
                    s.push_back(ch);
                }
                ++it;
            }

            return Node{std::move(s)};
        }

        Node load_bool(std::istream& input) {
            const auto s = load_literal(input);

            if (s == "true"sv) return Node{true};
            else if (s == "false"sv) return Node{false};
            else throw ParsingError("Failed to parse '"s + s + "' as bool"s);
        }

        Node load_null(std::istream& input) {
            if (auto literal = load_literal(input); literal == "null"sv) {
                return Node{nullptr};
            } else {
                throw ParsingError("Failed to parse '"s + literal + "' as null"s);
            }
        }

        Node load_number(std::istream& input) {
            std::string parsed_num;

            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) throw ParsingError("Failed to read number from stream"s);
            };

            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) throw ParsingError("A digit is expected"s);
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }

            if (input.peek() == '0') {
                read_char();
            } else {
                read_digits();
            }

            bool is_int = true;
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    try {
                        return std::stoi(parsed_num);
                    } catch (...) {
                        // In case of failure, for example, with overflow,
                        // the code below will try to convert the string to double
                    }
                }
                return std::stod(parsed_num);
            } catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node load_node(std::istream& input) {
            char c;
            if (!(input >> c)) throw ParsingError("Unexpected EOF"s);
            switch (c) {
                case '[':
                    return load_array(input);
                case '{':
                    return load_dict(input);
                case '"':
                    return load_string(input);
                case 't':
                    [[fallthrough]];
                case 'f':
                    input.putback(c);
                    return load_bool(input);
                case 'n':
                    input.putback(c);
                    return load_null(input);
                default:
                    input.putback(c);
                    return load_number(input);
            }
        }

        struct PrintContext {
            std::ostream& out;
            int indent_step = 4;
            int indent = 0;

            void PrintIndent() const {
                for (int i = 0; i < indent; ++i) {
                    out.put(' ');
                }
            }

            PrintContext Indented() const {
                return {out, indent_step, indent_step + indent};
            }
        };

        void print_node(const Node& node, const PrintContext& ctx);

        template <typename Value>
        void print_value(const Value& value, const PrintContext& ctx) {
            ctx.out << value;
        }

        void PrintString(const std::string& value, std::ostream& out) {
            out.put('"');
            for (const char c : value) {
                switch (c) {
                    case '\r':
                        out << "\\r"sv;
                        break;
                    case '\n':
                        out << "\\n"sv;
                        break;
                    case '"':
                        [[fallthrough]];
                    case '\\':
                        out.put('\\');
                        [[fallthrough]];
                    default:
                        out.put(c);
                        break;
                }
            }
            out.put('"');
        }

        template <>
        void print_value<std::string>(const std::string& value, const PrintContext& ctx) {
            PrintString(value, ctx.out);
        }

        template <>
        void print_value<std::nullptr_t>(const std::nullptr_t&, const PrintContext& ctx) {
            ctx.out << "null"sv;
        }

        template <>
        void print_value<bool>(const bool& value, const PrintContext& ctx) {
            ctx.out << (value ? "true"sv : "false"sv);
        }

        template <>
        void print_value<Array>(const Array& value, const PrintContext& ctx) {
            std::ostream& out = ctx.out;
            out << "[\n"sv;

            bool first = true;
            auto inner_ctx = ctx.Indented();
            for (const Node& node : value) {
                if (first) {
                    first = false;
                } else {
                    out << ",\n"sv;
                }
                inner_ctx.PrintIndent();
                print_node(node, inner_ctx);
            }

            out.put('\n');
            ctx.PrintIndent();
            out.put(']');
        }

        template <>
        void print_value<Dict>(const Dict& value, const PrintContext& ctx) {
            std::ostream& out = ctx.out;
            out << "{\n"sv;

            bool first = true;
            auto inner_ctx = ctx.Indented();
            for (const auto& [key, node] : value) {
                if (first) {
                    first = false;
                } else {
                    out << ",\n"sv;
                }
                inner_ctx.PrintIndent();
                PrintString(key, ctx.out);
                out << ": "sv;
                print_node(node, inner_ctx);
            }

            out.put('\n');
            ctx.PrintIndent();
            out.put('}');
        }

        void print_node(const Node& node, const PrintContext& ctx) {
            std::visit(
                    [&ctx](const auto& value) {
                        print_value(value, ctx);
                    },
                    node.get_value());
        }

    }  // namespace

    Document load(std::istream& input) {
        return Document{load_node(input)};
    }

    Document load(const std::string& input) {
        std::istringstream strm(input);
        return load(strm);
    }

    // C++20 gives us stringstreams constructors that takes rvalue reference to basic_string
    // at the time of this commit, on my M1 ARM Mac only gcc supports this option, clang does not.
    // checked for gcc-13. clang 13-16
    Document load(std::string&& input) {
        std::istringstream strm(std::move(input));
        return load(strm);
    }

    void print(const Document& doc, std::ostream& output) {
        print_node(doc.get_root(), PrintContext{output});
    }

} // namespace json
