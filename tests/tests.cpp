// @copyright Copyright (c) 2023. Created by Konstantin Belousov.
// All rights reserved.

#include "tests.h"
#include "log-duration.h"

#include "../include/json.h"
#include "../include/json-builder.h"

#include <chrono>

namespace tests {

    using namespace std::string_literals;

    std::string print_node(const json::Node& node);
    json::Document load_json(const std::string& s);
    void must_fail_to_load(const std::string& s);

    template <typename Fn>
    void must_throw_logic_error(Fn fn);

    void json_null_node_constructor() {
        json::Node null_node;
        ASSERT(null_node.is_null())
        ASSERT(!null_node.is_int())
        ASSERT(!null_node.is_double())
        ASSERT(!null_node.is_pure_double())
        ASSERT(!null_node.is_string())
        ASSERT(!null_node.is_array())
        ASSERT(!null_node.is_dict())

        json::Node null_node_2{nullptr};
        ASSERT(null_node_2.is_null())

        ASSERT(print_node(null_node) == "null"s)
        ASSERT(null_node == null_node_2)
        ASSERT(!(null_node != null_node_2))

        const json::Node node = load_json("null"s).get_root();
        ASSERT(node.is_null())
        ASSERT(node == null_node)

        ASSERT(load_json(" \t\r\n\n\r null \t\r\n\n\r "s).get_root() == null_node)
    }

    void json_number_values() {
        const json::Node int_node{42};
        ASSERT(int_node.is_int())
        ASSERT(int_node.as_int() == 42)

        ASSERT(int_node.is_double())

        ASSERT(int_node.as_double() == 42.0)
        ASSERT(!int_node.is_pure_double())
        ASSERT(int_node == json::Node{42})

        ASSERT(int_node != json::Node{42.0})

        const json::Node dbl_node{123.45};
        ASSERT(dbl_node.is_double())
        ASSERT(dbl_node.as_double() == 123.45)
        ASSERT(dbl_node.is_pure_double())
        ASSERT(!dbl_node.is_int())

        ASSERT(print_node(int_node) == "42"s)
        ASSERT(print_node(dbl_node) == "123.45"s)
        ASSERT(print_node(json::Node{-42}) == "-42"s)
        ASSERT(print_node(json::Node{-3.5}) == "-3.5"s)

        ASSERT(load_json("42"s).get_root() == int_node)
        ASSERT(load_json("123.45"s).get_root() == dbl_node)
        ASSERT(load_json("0.25"s).get_root().as_double() == 0.25)
        ASSERT(load_json("3e5"s).get_root().as_double() == 3e5)
        ASSERT(load_json("1.2e-5"s).get_root().as_double() == 1.2e-5)
        ASSERT(load_json("1.2e+5"s).get_root().as_double() == 1.2e5)
        ASSERT(load_json("-123456"s).get_root().as_int() == -123456)
        ASSERT(load_json("0").get_root() == json::Node{0})
        ASSERT(load_json("0.0").get_root() == json::Node{0.0})

        ASSERT(load_json(" \t\r\n\n\r 0.0 \t\r\n\n\r ").get_root() == json::Node{0.0})
    }

    void json_string_values() {
        json::Node str_node{"Hello, \"everybody\""s};
        ASSERT(str_node.is_string())
        ASSERT(str_node.as_string() == "Hello, \"everybody\""s)

        ASSERT(!str_node.is_int())
        ASSERT(!str_node.is_double())

        ASSERT(print_node(str_node) == "\"Hello, \\\"everybody\\\"\""s)
        ASSERT(load_json(print_node(str_node)).get_root() == str_node)

        const std::string escape_chars = R"("\r\n\t\"\\")"s;

        ASSERT(print_node(load_json(escape_chars).get_root()) == "\"\\r\\n\t\\\"\\\\\""s)

        ASSERT(load_json("\t\r\n\n\r \"Hello\" \t\r\n\n\r ").get_root() == json::Node{"Hello"s})
        ASSERT(load_json("\"Привет, мир!\"").get_root() == json::Node{"Привет, мир!"});
    }

    void json_bool_values() {
        json::Node true_node{true};
        ASSERT(true_node.is_bool())
        ASSERT(true_node.as_bool())

        json::Node false_node{false};
        ASSERT(false_node.is_bool())
        ASSERT(!false_node.as_bool())

        ASSERT(print_node(true_node) == "true"s)
        ASSERT(print_node(false_node) == "false"s)

        ASSERT(load_json("true"s).get_root() == true_node)
        ASSERT(load_json("false"s).get_root() == false_node)
        ASSERT(load_json(" \t\r\n\n\r true \r\n"s).get_root() == true_node)
        ASSERT(load_json(" \t\r\n\n\r false \t\r\n\n\r "s).get_root() == false_node)
    }

    void json_array_values() {
        json::Node arr_node{json::Array{1, 1.23, "Hello"s}};
        ASSERT(arr_node.is_array())
        const json::Array& arr = arr_node.as_array();
        ASSERT(arr.size() == 3)
        ASSERT(arr.at(0).as_int() == 1)

        ASSERT(load_json("[1,1.23,\"Hello\"]"s).get_root() == arr_node)
        ASSERT(load_json(print_node(arr_node)).get_root() == arr_node)
        ASSERT(load_json(R"(  [ 1  ,  1.23,  "Hello"   ]   )"s).get_root() == arr_node)

        ASSERT(load_json("[ 1 \r \n ,  \r\n\t 1.23, \n \n  \t\t  \"Hello\" \t \n  ] \n  "s).get_root() == arr_node)
    }

    void json_dictionary_values() {
        json::Node dict_node{json::Dict{{"key1"s, "value1"s}, {"key2"s, 42}}};
        ASSERT(dict_node.is_dict())
        const json::Dict& dict = dict_node.as_dict();
        ASSERT(dict.size() == 2)
        ASSERT(dict.at("key1"s).as_string() == "value1"s)
        ASSERT(dict.at("key2"s).as_int() == 42)

        ASSERT(load_json("{ \"key1\": \"value1\", \"key2\": 42 }"s).get_root() == dict_node)
        ASSERT(load_json(print_node(dict_node)).get_root() == dict_node)

        ASSERT(load_json("\t\r\n\n\r { \t\r\n\n\r \"key1\" \t\r\n\n\r: \t\r\n\n\r \"value1\" \t\r\n\n\r , \t\r\n\n\r \"key2\" \t\r\n\n\r : \t\r\n\n\r 42 \t\r\n\n\r } \t\r\n\n\r"s).get_root() == dict_node)
    }

    void json_error_handling() {
        must_fail_to_load("["s);
        must_fail_to_load("]"s);

        must_fail_to_load("{"s);
        must_fail_to_load("}"s);

        must_fail_to_load("\"hello"s);

        must_fail_to_load("tru"s);
        must_fail_to_load("fals"s);
        must_fail_to_load("nul"s);

        json::Node dbl_node{3.5};
        must_throw_logic_error([&dbl_node] {
            static_cast<void>(dbl_node.as_int());
        });
        must_throw_logic_error([&dbl_node] {
            static_cast<void>(dbl_node.as_string());
        });
        must_throw_logic_error([&dbl_node] {
            static_cast<void>(dbl_node.as_array());
        });

        json::Node array_node{json::Array{}};
        must_throw_logic_error([&array_node] {
            static_cast<void>(array_node.as_dict());
        });
        must_throw_logic_error([&array_node] {
            static_cast<void>(array_node.as_double());
        });
        must_throw_logic_error([&array_node] {
            static_cast<void>(array_node.as_bool());
        });
    }

    void json_builder() {
        std::stringstream ss;
        auto doc1 = json::Document{
                json::Builder{}
                        .start_dict()
                            .key("key1"s).value(123)
                            .key("key2"s).value("value2"s)
                            .key("key3"s).start_array()
                                                .value(456)
                                                .start_dict()
                                                .end_dict()
                                                .start_dict()
                                                        .key(""s).value(nullptr)
                                                .end_dict()
                                                .value(""s)
                                         .end_array()
                        .end_dict()
                        .build()
        };

        json::print(doc1, ss);
        json::Document doc2 = json::load(ss);

        ASSERT(doc1 == doc2);
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
            json::print(json::Document{arr}, strm);
        }
        {
            LOG_DURATION("Parsing from stream"sv);
            const auto doc = json::load(strm);
            ASSERT(doc.get_root() == arr)
        }

    }

    void run_test() {
        TestRunner tr;

        RUN_TEST(tr, json_null_node_constructor);
        RUN_TEST(tr, json_number_values);
        RUN_TEST(tr, json_string_values);
        RUN_TEST(tr, json_bool_values);
        RUN_TEST(tr, json_array_values);
        RUN_TEST(tr, json_dictionary_values);
        RUN_TEST(tr, json_error_handling);
        RUN_TEST(tr, json_builder);

        benchmark();
    }

    std::string print_node(const json::Node& node) {
        std::ostringstream out;
        print(json::Document{node}, out);
        return out.str();
    }

    json::Document load_json(const std::string& s) {
        std::istringstream strm(s);
        return json::load(strm);
    }

    void must_fail_to_load(const std::string& s) {
        using namespace std::string_view_literals;

        try {
            load_json(s);
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

} // namespace
