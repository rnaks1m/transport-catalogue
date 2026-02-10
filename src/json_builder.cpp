#include "json_builder.h"

using namespace std::literals;

namespace json {

    Builder::Builder() : root_(nullptr), check_key_value_(false) {}

    Builder::DictValueContext Builder::Key(std::string key) {
        if (nodes_stack_.empty() || !nodes_stack_.back()->IsMap() || !key_value_.empty()) {
            throw std::logic_error("logic error");
        }
        key_value_ = std::move(key);
        check_key_value_ = true;
        return DictValueContext(BaseContext(*this));
    }

    Builder::BaseContext Builder::Value(Node::Value value) {
        if (nodes_stack_.empty() && !root_.IsNull()) {
            throw std::logic_error("logic error");
        }

        if (nodes_stack_.empty()) {
            root_ = std::visit([](auto&& arg) { return Node(arg); }, value);
            return *this;
        }

        if (check_key_value_) {
            if (!nodes_stack_.back()->IsMap()) {
                throw std::logic_error("is not map");
            }
            Dict& dict = nodes_stack_.back()->AsMap();
            dict.emplace(std::move(key_value_), std::visit([](auto&& arg) { return Node(arg); }, value));
            check_key_value_ = false;
        }
        else {
            if (!nodes_stack_.back()->IsArray()) {
                throw std::logic_error("is not array");
            }
            Array& array = nodes_stack_.back()->AsArray();
            array.emplace_back(std::visit([](auto&& arg) { return Node(arg); }, value));
        }

        return *this;
    }

    Builder::DictItemContext Builder::StartDict() {
        if (nodes_stack_.empty() && !root_.IsNull()) {
            throw std::logic_error("logic error");
        }

        AddNode(Dict{});
        return DictItemContext(BaseContext(*this));
    }

    Builder::ArrayItemContext Builder::StartArray() {
        if (nodes_stack_.empty() && !root_.IsNull()) {
            throw std::logic_error("logic error");
        }

        AddNode(Array{});
        return ArrayItemContext(BaseContext(*this));
    }

    Builder::BaseContext Builder::EndDict() {
        if (nodes_stack_.empty() || !nodes_stack_.back()->IsMap()) {
            throw std::logic_error("logic error");
        }
        nodes_stack_.pop_back();
        return *this;
    }

    Builder::BaseContext Builder::EndArray() {
        if (nodes_stack_.empty() || !nodes_stack_.back()->IsArray()) {
            throw std::logic_error("logic error");
        }
        nodes_stack_.pop_back();
        return *this;
    }

    Node Builder::Build() {
        if (!nodes_stack_.empty() || root_.IsNull()) {
            throw std::logic_error("structure is unfinished");
        }
        return std::move(root_);
    }

    void Builder::AddNode(Node node) {
        if (!nodes_stack_.empty() && nodes_stack_.back()->IsMap() && !check_key_value_) {
            throw std::logic_error("dict logic error");
        }

        if (nodes_stack_.empty()) {
            root_ = std::move(node);
            nodes_stack_.push_back(&root_);
        }
        else if (check_key_value_) {
            if (nodes_stack_.empty() || !nodes_stack_.back()->IsMap()) {
                throw std::logic_error("Map error add value");
            }
            Dict& val = nodes_stack_.back()->AsMap();
            auto [it, check] = val.emplace(std::move(key_value_), std::move(node));
            nodes_stack_.push_back(&it->second);
            check_key_value_ = false;
        }
        else {
            if (nodes_stack_.empty() || !nodes_stack_.back()->IsArray()) {
                throw std::logic_error("Array error add value");
            }
            Array& val = nodes_stack_.back()->AsArray();
            val.push_back(std::move(node));
            nodes_stack_.push_back(&val.back());
        }
    }
}