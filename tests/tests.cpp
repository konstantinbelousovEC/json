#include "tests.h"
#include "../include/json.h"
#include "log-duration.h"

#include <chrono>

namespace tests {

    using namespace std::string_literals;

    std::string PrintNode(const json::Node& node) {
        std::ostringstream out;
        Print(json::Document{node}, out);
        return out.str();
    }

    json::Document LoadJSON(const std::string& s) {
        std::istringstream strm(s);
        return json::Load(strm);
    }

    void must_fail_to_load(const std::string& s) {
        using namespace std::string_view_literals;

        try {
            LoadJSON(s);
            std::cerr << "ParsingError exception is expected on '"sv << s << "'"sv << std::endl;
            ASSERT(false)
        } catch (const json::ParsingError&) {
            // ok
        } catch (const std::exception& e) {
            std::cerr << "exception thrown: "sv << e.what() << std::endl;
            ASSERT(false)
        } catch (...) {
            std::cerr << "Unexpected error"sv << std::endl;
            ASSERT(false)
        }
    }

    template <typename Fn>
    void must_throw_logic_error(Fn fn) {
        using namespace std::string_view_literals;

        try {
            fn();
            std::cerr << "logic_error is expected"sv << std::endl;
            ASSERT(false)
        } catch (const std::logic_error&) {
            // ok
        } catch (const std::exception& e) {
            std::cerr << "exception thrown: "sv << e.what() << std::endl;
            ASSERT(false)
        } catch (...) {
            std::cerr << "Unexpected error"sv << std::endl;
            ASSERT(false)
        }
    }

    void json_null_node_constructor() {
        json::Node null_node;
        ASSERT(null_node.IsNull())
        ASSERT(!null_node.IsInt())
        ASSERT(!null_node.IsDouble())
        ASSERT(!null_node.IsPureDouble())
        ASSERT(!null_node.IsString())
        ASSERT(!null_node.IsArray())
        ASSERT(!null_node.IsDict())

        json::Node null_node_2{nullptr};
        ASSERT(null_node_2.IsNull())

        ASSERT(PrintNode(null_node) == "null"s)
        ASSERT(null_node == null_node_2)
        ASSERT(!(null_node != null_node_2))

        const json::Node node = LoadJSON("null"s).GetRoot();
        ASSERT(node.IsNull())
        ASSERT(node == null_node)

        // Пробелы, табуляции и символы перевода строки между токенами JSON файла игнорируются
        ASSERT(LoadJSON(" \t\r\n\n\r null \t\r\n\n\r "s).GetRoot() == null_node)
    }

    void json_number_values() {
        const json::Node int_node{42};
        ASSERT(int_node.IsInt())
        ASSERT(int_node.AsInt() == 42)

        // целые числа являются подмножеством чисел с плавающей запятой
        ASSERT(int_node.IsDouble())

        // Когда узел хранит int, можно получить соответствующее ему double-значение
        ASSERT(int_node.AsDouble() == 42.0)
        ASSERT(!int_node.IsPureDouble())
        ASSERT(int_node == json::Node{42})

        // int и double - разные типы, поэтому не равны, даже когда хранят равные семантические значения
        ASSERT(int_node != json::Node{42.0})

        const json::Node dbl_node{123.45};
        ASSERT(dbl_node.IsDouble())
        ASSERT(dbl_node.AsDouble() == 123.45)
        ASSERT(dbl_node.IsPureDouble())  // Значение содержит число с плавающей запятой
        ASSERT(!dbl_node.IsInt())

        ASSERT(PrintNode(int_node) == "42"s)
        ASSERT(PrintNode(dbl_node) == "123.45"s)
        ASSERT(PrintNode(json::Node{-42}) == "-42"s)
        ASSERT(PrintNode(json::Node{-3.5}) == "-3.5"s)

        ASSERT(LoadJSON("42"s).GetRoot() == int_node)
        ASSERT(LoadJSON("123.45"s).GetRoot() == dbl_node)
        ASSERT(LoadJSON("0.25"s).GetRoot().AsDouble() == 0.25)
        ASSERT(LoadJSON("3e5"s).GetRoot().AsDouble() == 3e5)
        ASSERT(LoadJSON("1.2e-5"s).GetRoot().AsDouble() == 1.2e-5)
        ASSERT(LoadJSON("1.2e+5"s).GetRoot().AsDouble() == 1.2e5)
        ASSERT(LoadJSON("-123456"s).GetRoot().AsInt() == -123456)
        ASSERT(LoadJSON("0").GetRoot() == json::Node{0})
        ASSERT(LoadJSON("0.0").GetRoot() == json::Node{0.0})

        // Пробелы, табуляции и символы перевода строки между токенами JSON файла игнорируются
        ASSERT(LoadJSON(" \t\r\n\n\r 0.0 \t\r\n\n\r ").GetRoot() == json::Node{0.0})
    }

    void json_string_values() {
        json::Node str_node{"Hello, \"everybody\""s};
        ASSERT(str_node.IsString())
        ASSERT(str_node.AsString() == "Hello, \"everybody\""s)

        ASSERT(!str_node.IsInt())
        ASSERT(!str_node.IsDouble())

        ASSERT(PrintNode(str_node) == "\"Hello, \\\"everybody\\\"\""s)
        ASSERT(LoadJSON(PrintNode(str_node)).GetRoot() == str_node)

        const std::string escape_chars
                = R"("\r\n\t\"\\")"s;  // При чтении строкового литерала последовательности \r,\n,\t,\\,\"
        // преобразовываться в соответствующие символы.
        // При выводе эти символы должны экранироваться, кроме \t.
        ASSERT(PrintNode(LoadJSON(escape_chars).GetRoot()) == "\"\\r\\n\t\\\"\\\\\""s)

        // Пробелы, табуляции и символы перевода строки между токенами JSON файла игнорируются
        ASSERT(LoadJSON("\t\r\n\n\r \"Hello\" \t\r\n\n\r ").GetRoot() == json::Node{"Hello"s})
    }

    void json_bool_values() {
        json::Node true_node{true};
        ASSERT(true_node.IsBool())
        ASSERT(true_node.AsBool())

        json::Node false_node{false};
        ASSERT(false_node.IsBool())
        ASSERT(!false_node.AsBool())

        ASSERT(PrintNode(true_node) == "true"s)
        ASSERT(PrintNode(false_node) == "false"s)

        ASSERT(LoadJSON("true"s).GetRoot() == true_node)
        ASSERT(LoadJSON("false"s).GetRoot() == false_node)
        ASSERT(LoadJSON(" \t\r\n\n\r true \r\n"s).GetRoot() == true_node)
        ASSERT(LoadJSON(" \t\r\n\n\r false \t\r\n\n\r "s).GetRoot() == false_node)
    }

    void json_array_values() {
        json::Node arr_node{json::Array{1, 1.23, "Hello"s}};
        ASSERT(arr_node.IsArray())
        const json::Array& arr = arr_node.AsArray();
        ASSERT(arr.size() == 3)
        ASSERT(arr.at(0).AsInt() == 1)

        ASSERT(LoadJSON("[1,1.23,\"Hello\"]"s).GetRoot() == arr_node)
        ASSERT(LoadJSON(PrintNode(arr_node)).GetRoot() == arr_node)
        ASSERT(LoadJSON(R"(  [ 1  ,  1.23,  "Hello"   ]   )"s).GetRoot() == arr_node)

        // Пробелы, табуляции и символы перевода строки между токенами JSON файла игнорируются
        ASSERT(LoadJSON("[ 1 \r \n ,  \r\n\t 1.23, \n \n  \t\t  \"Hello\" \t \n  ] \n  "s).GetRoot()
               == arr_node)
    }

    void json_dictionary_values() {
        json::Node dict_node{json::Dict{{"key1"s, "value1"s}, {"key2"s, 42}}};
        ASSERT(dict_node.IsDict())
        const json::Dict& dict = dict_node.AsDict();
        ASSERT(dict.size() == 2)
        ASSERT(dict.at("key1"s).AsString() == "value1"s)
        ASSERT(dict.at("key2"s).AsInt() == 42)

        ASSERT(LoadJSON("{ \"key1\": \"value1\", \"key2\": 42 }"s).GetRoot() == dict_node)
        ASSERT(LoadJSON(PrintNode(dict_node)).GetRoot() == dict_node)

        // Пробелы, табуляции и символы перевода строки между токенами JSON файла игнорируются
        ASSERT(
                LoadJSON(
                        "\t\r\n\n\r { \t\r\n\n\r \"key1\" \t\r\n\n\r: \t\r\n\n\r \"value1\" \t\r\n\n\r , \t\r\n\n\r \"key2\" \t\r\n\n\r : \t\r\n\n\r 42 \t\r\n\n\r } \t\r\n\n\r"s)
                        .GetRoot()
                == dict_node)
    }

    void json_error_handling() {
        must_fail_to_load("["s);
        must_fail_to_load("]"s);

        must_fail_to_load("{"s);
        must_fail_to_load("}"s);

        must_fail_to_load("\"hello"s);  // незакрытая кавычка

        must_fail_to_load("tru"s);
        must_fail_to_load("fals"s);
        must_fail_to_load("nul"s);

        json::Node dbl_node{3.5};
        must_throw_logic_error([&dbl_node] {
            dbl_node.AsInt();
        });
        must_throw_logic_error([&dbl_node] {
            dbl_node.AsString();
        });
        must_throw_logic_error([&dbl_node] {
            dbl_node.AsArray();
        });

        json::Node array_node{json::Array{}};
        must_throw_logic_error([&array_node] {
            array_node.AsDict();
        });
        must_throw_logic_error([&array_node] {
            array_node.AsDouble();
        });
        must_throw_logic_error([&array_node] {
            array_node.AsBool();
        });
    }

    void benchmark() {
        using namespace std::string_view_literals;
        constexpr int SIZE = 5'000;

        json::Array arr;
        arr.reserve(SIZE);

        for (int i = 0; i < SIZE; ++i) {
            arr.emplace_back(json::Dict{
                    {"int"s, 42},
                    {"double"s, 42.1},
                    {"null"s, nullptr},
                    {"string"s, "hello"s},
                    {"array"s, json::Array{1, 2, 3}},
                    {"bool"s, true},
                    {"map"s, json::Dict{{"key"s, "value"s}}},
            });
        }

        std::stringstream strm;

        {
            LOG_DURATION("Writing to stream"sv);
            json::Print(json::Document{arr}, strm);
        }
        {
            LOG_DURATION("Parsing from stream"sv);
            const auto doc = json::Load(strm);
            ASSERT(doc.GetRoot() == arr)
        }

    }

    void RunTest() {
        TestRunner tr;

        RUN_TEST(tr, json_null_node_constructor);
        RUN_TEST(tr, json_number_values);
        RUN_TEST(tr, json_string_values);
        RUN_TEST(tr, json_bool_values);
        RUN_TEST(tr, json_array_values);
        RUN_TEST(tr, json_dictionary_values);
        RUN_TEST(tr, json_error_handling);

        benchmark();
    }

}