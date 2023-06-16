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
        using Value = variant;

        Node(Value&& v) : variant(std::move(v)) {};

        [[nodiscard]] bool IsInt() const;
        [[nodiscard]] bool IsPureDouble() const;
        [[nodiscard]] bool IsDouble() const;
        [[nodiscard]] bool IsBool() const;
        [[nodiscard]] bool IsNull() const;
        [[nodiscard]] bool IsArray() const;
        [[nodiscard]] bool IsString() const;
        [[nodiscard]] bool IsDict() const;

        [[nodiscard]] int AsInt() const;
        [[nodiscard]] double AsDouble() const;
        [[nodiscard]] bool AsBool() const;

        [[nodiscard]] const std::string& AsString() const;

        [[nodiscard]] Array& AsArray();
        [[nodiscard]] const Array& AsArray() const;
        [[nodiscard]] Dict& AsDict();
        [[nodiscard]] const Dict& AsDict() const;

        [[nodiscard]] const Value& GetValue() const;
    };

    inline bool operator==(const Node& lhs, const Node& rhs) {
        return lhs.GetValue() == rhs.GetValue();
    }

    inline bool operator!=(const Node& lhs, const Node& rhs) {
        return !(lhs == rhs);
    }

    class Document {
    public:
        explicit Document(Node root);
        const Node& GetRoot() const;

    private:
        Node root_;
    };

    inline bool operator==(const Document& lhs, const Document& rhs) {
        return lhs.GetRoot() == rhs.GetRoot();
    }

    inline bool operator!=(const Document& lhs, const Document& rhs) {
        return !(lhs == rhs);
    }

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

}