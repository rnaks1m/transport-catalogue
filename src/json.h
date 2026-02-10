#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

    class Node;
    // Сохраните объявления Dict и Array без изменения
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node {
    public:
        using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
        const Value& GetValue() const { return value_; }
        inline Value& GetValue() { return value_; }

        Node();
        Node(nullptr_t);
        Node(Array array);
        Node(Dict array);
        Node(bool value);
        Node(int value);
        Node(double value);
        Node(std::string value);

        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        int AsInt() const;
        bool AsBool() const;
        double AsDouble() const;
        const std::string& AsString() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;
        Array& AsArray();
        Dict& AsMap();

        bool operator==(const Node& other) const;
        bool operator!=(const Node& other) const;

    private:
        Value value_;
    };

    // Контекст вывода, хранит ссылку на поток вывода и текущий отсуп
    struct PrintContext {
        std::ostream& out;
        int indent_step = 4;
        int indent = 0;

        void PrintIndent() const;

        // Возвращает новый контекст вывода с увеличенным смещением
        PrintContext Indented() const;
    };


    struct PrintValue {
        PrintContext ctx;

        template <typename Value>
        void operator()(const Value& value) const {
            ctx.PrintIndent();
            ctx.out << value;
        }
        void operator()(std::nullptr_t) const;
        void operator()(bool value) const;
        void operator()(std::string value) const;
        void operator()(Dict value) const;
        void operator()(Array value) const;
    };

    void PrintNode(const Node& node, std::ostream& out);

    std::ostream& operator<<(std::ostream& out, const Node node);

    //------------------ Document ----------------------

    class Document {
    public:
        explicit Document(Node root);
        explicit Document();
        const Node& GetRoot() const;

        bool operator==(const Document& other) const;
        bool operator!=(const Document& other) const;

    private:
        Node root_;
    };

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

    //------------------- Parsing --------------------------

}  // namespace json