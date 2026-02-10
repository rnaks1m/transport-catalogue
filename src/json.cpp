#include "json.h"

using namespace std;

namespace json {

    //------------------------- Parsing ------------------------

    namespace {

        using Number = std::variant<int, double>;

        Node LoadNode(istream& input);

        Node LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            }
            else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
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
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return Node(move(std::stoi(parsed_num)));
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return Node(move(std::stod(parsed_num)));

            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        // Считывает содержимое строкового литерала JSON-документа
        // Функцию следует использовать после считывания открывающего символа ":
        Node LoadString(istream& input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
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
                        // Встретили неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }
            return Node(move(s));
        }

        Node LoadBoolAndNull(std::istream& input) {
            string str;
            for (char c; input >> c;) {
                if (c < 'a' || c > 'z') {
                    input.putback(c);
                    break;
                }
                str.push_back(c);
            }

            if (str == "null") {
                return Node(nullptr);
            }
            else if (str == "true") {
                return Node(true);
            }
            else if (str == "false") {
                return Node(false);
            }
            else {
                throw ParsingError("invalid data");
            }
        }

        Node LoadValue(std::istream& input) {
            for (char c; input >> c;) {

                if (c == ',' || c == ']' || c == '}') {
                    input.putback(c);
                    break;
                }

                if ((c >= '0' && c <= '9') || c == '-') {
                    input.putback(c);
                    return LoadNumber(input);
                }
                else if (c == '"') {
                    return LoadString(input);
                }
                else if (c >= 'a' && c <= 'z') {
                    input.putback(c);
                    return LoadBoolAndNull(input);
                }
                else {
                    throw ParsingError("invalid data");
                }
            }

            return Node();
        }

        Node LoadArray(istream& input) {
            Array result;

            for (char c; input >> c;) {
                if (c == ']') {
                    input.putback(c);
                    break;
                }

                if (c != ',') {
                    input.putback(c);
                }

                result.push_back(LoadNode(input));
            }

            if (input.peek() != ']') {
                throw ParsingError("Expected closing ']' for array");
            }
            input.get();

            return Node(move(result));
        }

        Node LoadDict(istream& input) {
            Dict result;

            for (char c; input >> c;) {

                if (c == '}') {
                    input.putback(c);
                    break;
                }

                if (c == ',') {
                    input >> c;
                }

                string key = LoadString(input).AsString();
                input >> c;
                result.insert({ move(key), LoadNode(input) });
            }

            if (input.peek() != '}') {
                throw ParsingError("Expected closing '}' for array");
            }
            input.get();

            return Node(move(result));
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;

            if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else if (c == '"') {
                return LoadString(input);
            }
            else {
                if (c == ']' || c == '}') {
                    throw ParsingError("invalid data");
                }

                input.putback(c);
                return LoadValue(input);
            }
        }

    }  // namespace

    //--------------- Node construct --------------

    Node::Node()
        : value_(nullptr) {
    }

    Node::Node(nullptr_t)
        : value_(nullptr) {
    }

    Node::Node(Array array)
        : value_(move(array)) {
    }

    Node::Node(Dict map)
        : value_(move(map)) {
    }

    Node::Node(bool value)
        : value_(value) {
    }

    Node::Node(int value)
        : value_(value) {
    }

    Node::Node(double value)
        : value_(value) {
    }

    Node::Node(string value)
        : value_(move(value)) {
    }

    //---------------- Node is value ----------------------

    bool Node::IsNull() const {
        return std::holds_alternative<std::nullptr_t>(value_);
    }

    bool Node::IsArray() const {
        return std::holds_alternative<Array>(value_);
    }

    bool Node::IsMap() const {
        return std::holds_alternative<Dict>(value_);
    }

    bool Node::IsBool() const {
        return std::holds_alternative<bool>(value_);
    }

    bool Node::IsInt() const {
        return std::holds_alternative<int>(value_);
    }

    bool Node::IsDouble() const {
        return std::holds_alternative<double>(value_) || std::holds_alternative<int>(value_);
    }

    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(value_);
    }

    bool Node::IsString() const {
        return std::holds_alternative<std::string>(value_);
    }

    //---------------------- Node as value --------------------

    Array& Node::AsArray() {
        if (!IsArray()) {
            throw std::logic_error("logic error");
        }
        return std::get<Array>(value_);
    }

    Dict& Node::AsMap() {
        if (!IsMap()) {
            throw std::logic_error("logic error");
        }
        return std::get<Dict>(value_);
    }

    const Array& Node::AsArray() const {
        if (!IsArray()) {
            throw std::logic_error("logic error");
        }
        return std::get<Array>(value_);
    }

    const Dict& Node::AsMap() const {
        if (!IsMap()) {
            throw std::logic_error("logic error");
        }
        return std::get<Dict>(value_);
    }

    int Node::AsInt() const {
        if (!IsInt()) {
            throw std::logic_error("logic error");
        }
        return std::get<int>(value_);
    }

    bool Node::AsBool() const {
        if (!IsBool()) {
            throw std::logic_error("logic error");
        }
        return std::get<bool>(value_);
    }

    double Node::AsDouble() const {
        if (!IsDouble() && !IsInt()) {
            throw std::logic_error("logic error");
        }
        if (IsInt()) {
            return std::get<int>(value_);
        }
        return std::get<double>(value_);
    }

    const std::string& Node::AsString() const {
        if (!IsString()) {
            throw std::logic_error("logic error");
        }
        return std::get<std::string>(value_);
    }

    //---------------- Document ------------------------

    Document::Document(Node root)
        : root_(move(root)) {
    }

    Document::Document() : root_(Node()) {}

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    //----------------- Print Context -----------------------

    void PrintContext::PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    // Возвращает новый контекст вывода с увеличенным смещением
    PrintContext PrintContext::Indented() const {
        return { out, indent_step, indent_step + indent };
    }

    //------------------ PrintValue --------------------------

    void PrintValue::operator()(std::nullptr_t) const {
        ctx.PrintIndent();
        ctx.out << "null"sv;
    }

    void PrintValue::operator()(bool value) const {
        ctx.PrintIndent();
        if (value) {
            ctx.out << "true"sv;
        }
        else {
            ctx.out << "false"sv;
        }
    }

    void PrintValue::operator()(std::string value) const {
        ctx.PrintIndent();
        ctx.out << "\""sv;
        for (auto symbol : value) {
            if (symbol == '\\') {
                ctx.out << '\\' << '\\';
            }
            else if (symbol == '\"') {
                ctx.out << '\\' << '\"';
            }
            else if (symbol == '\r') {
                ctx.out << '\\' << 'r';
            }
            else if (symbol == '\n') {
                ctx.out << '\\' << 'n';
            }
            else if (symbol == '\t') {
                ctx.out << '\\' << 't';
            }
            else {
                ctx.out << symbol;
            }
        }
        ctx.out << "\""sv;
    }

    void PrintValue::operator()(Dict value) const {
        size_t i(0);
        PrintContext new_out(ctx.out, 0, 0);

        ctx.PrintIndent();
        ctx.out << "{\n"sv;
        for (const auto& [key, val] : value) {
            ++i;
            ctx.Indented().PrintIndent();
            ctx.out << "\""sv << key << "\""sv << ": ";
            if (!val.IsArray() && !val.IsMap()) {
                std::visit(PrintValue{ new_out }, val.GetValue());
            }
            else {
                std::visit(PrintValue{ ctx.Indented() }, val.GetValue());
            }

            if (i < value.size()) {
                ctx.out << ","sv;
            }
            ctx.out << "\n"sv;
        }
        ctx.PrintIndent();
        ctx.out << "}"sv;
    }

    void PrintValue::operator()(Array value) const {
        ctx.out << "[\n"sv;

        size_t i(0);
        for (const auto& val : value) {
            ++i;
            std::visit(PrintValue{ ctx.Indented() }, val.GetValue());

            if (i < value.size()) {
                ctx.out << ","sv;
            }
            ctx.out << "\n"sv;
        }
        ctx.PrintIndent();
        ctx.out << "]"sv;
    }
    //----------------- Print Node --------------------------

    void PrintNode(const Node& node, std::ostream& out) {
        PrintContext ctx{ out };
        std::visit(PrintValue{ ctx }, node.GetValue());
    }

    ostream& operator<<(ostream& out, const Node node) {
        PrintNode(node, out);
        return out;
    }

    void Print(const Document& doc, std::ostream& output) {
        PrintNode(doc.GetRoot(), output);
    }

    //------------------- Operator != == -----------------------------------

    bool Node::operator==(const Node& other) const {
        return value_ == other.value_;
    }

    bool Node::operator!=(const Node& other) const {
        return !(*this == other);
    }

    bool Document::operator==(const Document& other) const {
        return root_ == other.root_;
    }

    bool Document::operator!=(const Document& other) const {
        return !(*this == other);
    }

}  // namespace json