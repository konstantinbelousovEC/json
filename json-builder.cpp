// @copyright Copyright (c) 2023. Created by Konstantin Belousov.
// All rights reserved.

#include "json-builder.h"

namespace json {

    Builder& Builder::key(std::string key) {
        if (root_ != nullptr) throw std::logic_error("calling key method for ready object");

        if (nodes_stack_.back()->is_dict()) {
            Node::value str{std::move(key)};
            nodes_.emplace_back(std::move(str));
            nodes_stack_.push_back(&nodes_.back());
        } else {
            throw std::logic_error("calling key method in wrong place");
        }

        return *this;
    }

    Builder& Builder::value(Node::value value) {
        if (root_ != nullptr) throw std::logic_error("calling value method for ready object");

        if (root_ == nullptr && nodes_stack_.empty()) {
            root_ = std::move(value);
        } else if (nodes_stack_.back()->is_array()) {
            nodes_stack_.back()->as_array().emplace_back(std::move(value));
        } else if (nodes_stack_.back()->is_string()) {
            Node& node_ref =*nodes_stack_.back();
            nodes_stack_.pop_back();
            nodes_stack_.back()->as_dict().insert({node_ref.as_string(), std::move(value)});
        } else {
            throw std::logic_error("calling value method in wrong place");
        }

        return *this;
    }

    Builder::DictItemContext Builder::start_dict() {
        start_data(Dict{});
        return {*this};
    }

    Builder::ArrayItemContext Builder::start_array() {
        start_data(Array{});
        return {*this};
    }

    Builder& Builder::end_dict() {
        if (nodes_stack_.empty()) throw std::logic_error("calling end_dict method for ready or empty object");

        if (nodes_stack_.back()->is_dict()) {
            end_data();
            return *this;
        } else {
            throw std::logic_error("calling end_dict method in wrong place");
        }
    }

    Builder& Builder::end_array() {
        if (nodes_stack_.empty()) throw std::logic_error("calling end_array method for ready or empty object");

        if (nodes_stack_.back()->is_array()) {
            end_data();
            return *this;
        } else {
            throw std::logic_error("calling end_array method in wrong place");
        }
    }

    json::Node Builder::build() {
        if (nodes_stack_.empty() && root_ != nullptr) {
            return root_;
        } else {
            throw std::logic_error("calling build when object is not ready");
        }
    }

    void Builder::end_data() {
        Node& node_ref = *nodes_stack_.back();
        nodes_stack_.pop_back();

        if (nodes_stack_.empty()) {
            root_ = std::move(node_ref);
        } else if (nodes_stack_.back()->is_array()) {
            nodes_stack_.back()->as_array().emplace_back(std::move(node_ref));
        } else if (nodes_stack_.back()->is_string()) {
            Node& str_node_ref = *nodes_stack_.back();
            nodes_stack_.pop_back();
            nodes_stack_.back()->as_dict().insert({str_node_ref.as_string(), std::move(node_ref)});
        }
    }

    Builder& Builder::ItemContext::end_dict() {
        builder_.end_dict();
        return builder_;
    }

    Builder& Builder::ItemContext::end_array() {
        builder_.end_array();
        return builder_;
    }

    Builder::DictItemContext Builder::ItemContext::start_dict() {
        builder_.start_dict();
        return {*this};
    }

    Builder::ArrayItemContext Builder::ItemContext::start_array() {
        builder_.start_array();
        return {*this};
    }

    Builder::ValueAfterArrayContext Builder::ItemContext::value(Node::value value) {
        builder_.value(std::move(value));
        return {*this};
    }

    Builder::ValueAfterKeyContext Builder::KeyItemContext::value(Node::value value) {
        builder_.value(std::move(value));
        return {*this};
    }

    Builder::KeyItemContext Builder::ValueAfterKeyContext::key(std::string key) {
        builder_.key(std::move(key));
        return {*this};
    }

    Builder::KeyItemContext Builder::DictItemContext::key(std::string key) {
        builder_.key(std::move(key));
        return {*this};
    }

}  // namespace json
