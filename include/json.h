#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node final : private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> {
    public:
        using variant::variant;
        using value = variant;

        Node(value&& v) : variant(std::move(v)) {};

        [[nodiscard]] bool is_int() const;
        [[nodiscard]] bool is_pure_double() const;
        [[nodiscard]] bool is_double() const;
        [[nodiscard]] bool is_bool() const;
        [[nodiscard]] bool is_null() const;
        [[nodiscard]] bool is_array() const;
        [[nodiscard]] bool is_string() const;
        [[nodiscard]] bool is_dict() const;

        [[nodiscard]] int as_int() const;
        [[nodiscard]] double as_double() const;
        [[nodiscard]] bool as_bool() const;

        [[nodiscard]] const std::string& as_string() const;

        [[nodiscard]] Array& as_array();
        [[nodiscard]] const Array& as_array() const;
        [[nodiscard]] Dict& as_dict();
        [[nodiscard]] const Dict& as_dict() const;

        [[nodiscard]] const value& get_value() const;
    };

    inline bool operator==(const Node& lhs, const Node& rhs) {
        return lhs.get_value() == rhs.get_value();
    }

    inline bool operator!=(const Node& lhs, const Node& rhs) {
        return !(lhs == rhs);
    }

    class Document {
    public:
        explicit Document(Node root);
        const Node& get_root() const;

    private:
        Node root_;
    };

    inline bool operator==(const Document& lhs, const Document& rhs) {
        return lhs.get_root() == rhs.get_root();
    }

    inline bool operator!=(const Document& lhs, const Document& rhs) {
        return !(lhs == rhs);
    }

    Document load(std::istream& input);

    void print(const Document& doc, std::ostream& output);

} // namespace