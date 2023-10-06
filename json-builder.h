// @copyright Copyright (c) 2023. Created by Konstantin Belousov.
// All rights reserved.

#pragma once

#include <iostream>
#include <vector>
#include <deque>
#include "json.h"

namespace json {

    class Builder {
     private:
        class DictItemContext;
        class ArrayItemContext;
        class KeyItemContext;
        class ValueAfterArrayContext;
        class ValueAfterKeyContext;
        class ItemContext;

        template<typename T>
        void start_data(T obj);
        void end_data();
        Node root_ = nullptr;
        std::vector<Node*> nodes_stack_;
        std::deque<Node> nodes_;

     public:
        Builder& key(std::string key);
        Builder& value(Node::value value);
        DictItemContext start_dict();
        ArrayItemContext start_array();
        Builder& end_dict();
        Builder& end_array();
        json::Node build();
    };

    class Builder::ItemContext {
     public:
        ItemContext(Builder& builder) : builder_(builder) {};
        ValueAfterArrayContext value(Node::value value);
        DictItemContext start_dict();
        ArrayItemContext start_array();
        Builder& end_dict();
        Builder& end_array();
     protected:
        Builder& builder_;
    };

    class Builder::ValueAfterKeyContext : public ItemContext {
     public:
        KeyItemContext key(std::string key);
        ValueAfterArrayContext value(Node::value value) = delete;
        Builder& end_array() = delete;
        DictItemContext start_dict() = delete;
        ArrayItemContext start_array() = delete;
    };

    class Builder::ValueAfterArrayContext : public ItemContext {
     public:
        Builder& end_dict() = delete;
    };

    class Builder::KeyItemContext : public ItemContext {
     public:
        ValueAfterKeyContext value(Node::value value);
        Builder& end_dict() = delete;
        Builder& end_array() = delete;
    };

    class Builder::DictItemContext : public ItemContext {
     public:
        KeyItemContext key(std::string key);
        ValueAfterArrayContext value(Node::value value) = delete;
        Builder& end_array() = delete;
        DictItemContext start_dict() = delete;
        ArrayItemContext start_array() = delete;
    };

    class Builder::ArrayItemContext : public ItemContext {
     public:
        Builder& end_dict() = delete;
    };

    template<typename T>
    void Builder::start_data(T obj) {
        std::string str;
        if constexpr (std::is_same_v<T, Array>) {
            str = "Array";
        } else {
            str = "Dict";
        }

        if (root_ != nullptr) throw std::logic_error("calling Start" + str + "-method for ready object");

        if (nodes_stack_.empty() || nodes_stack_.back()->is_array() || nodes_stack_.back()->is_string()) {
            nodes_.emplace_back(obj);
            nodes_stack_.push_back(&nodes_.back());
        } else {
            throw std::logic_error("calling Start" + str + "-method in wrong place");
        }
    }

}  // namespace json
