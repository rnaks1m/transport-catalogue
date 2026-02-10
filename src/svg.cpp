#include "svg.h"

namespace svg {

    using namespace std::literals;

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- Polyline ----------------

    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        bool check = false;
        out << "<polyline points=\""sv;
        for (auto point : points_) {
            if (check == true) {
                out << " "sv;
            }
            out << point.x << ","sv << point.y;
            check = true;
        }
        out << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- Text --------------------

    Text& Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(std::string data) {
        data_ = data;
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text"sv;
        RenderAttrs(out);
        out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv
            << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv
            << "font-size=\""sv << size_ << "\""sv;
        if (!font_family_.empty()) {
            out << " font-family=\""sv << font_family_ << "\""sv;
        }
        if (!font_weight_.empty()) {
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        }
        out << ">"sv << data_ << "</text>"sv;
    }

    // ---------- Document ----------------

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.emplace_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

        RenderContext context(out, 2, 2);

        for (const auto& object : objects_) {
            object->Render(context);
        }

        out << "</svg>"sv;
    }

    // ---------- Enum --------------------

    std::ostream& operator<<(std::ostream& out, const StrokeLineCap slp) {
        if (slp == StrokeLineCap::BUTT) {
            out << "butt";
        }
        else if ((slp == StrokeLineCap::ROUND)) {
            out << "round";
        }
        else if ((slp == StrokeLineCap::SQUARE)) {
            out << "square";
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin slj) {
        if (slj == StrokeLineJoin::ARCS) {
            out << "arcs";
        }
        else if ((slj == StrokeLineJoin::BEVEL)) {
            out << "bevel";
        }
        else if ((slj == StrokeLineJoin::MITER)) {
            out << "miter";
        }
        else if ((slj == StrokeLineJoin::MITER_CLIP)) {
            out << "miter-clip";
        }
        else if ((slj == StrokeLineJoin::ROUND)) {
            out << "round";
        }
        return out;
    }

    // ---------- Color -------------------

    void OstreamColor::operator()(std::monostate) const {
        out << "none"sv;
    }

    void OstreamColor::operator()(std::string color) const {
        out << color;
    }

    void OstreamColor::operator()(svg::Rgb color) const {
        out << "rgb("sv << static_cast<int>(color.red)
            << ","sv << static_cast<int>(color.green) << ","sv
            << static_cast<int>(color.blue) << ")"sv;
    }

    void OstreamColor::operator()(svg::Rgba color) const {
        out << "rgba("sv << static_cast<int>(color.red) << ","sv
            << static_cast<int>(color.green) << ","sv
            << static_cast<int>(color.blue) << ","sv
            << color.opacity << ")"sv;
    }

    std::ostream& operator<<(std::ostream& out, const Color color) {
        std::visit(OstreamColor{ out }, color);
        return out;
    }

}  // namespace svg