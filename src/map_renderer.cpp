#include "map_renderer.h"

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
    return {
        (coords.lng - min_lon_) * zoom_coeff_ + padding_,
        (max_lat_ - coords.lat) * zoom_coeff_ + padding_
    };
}

void PaintDataRoutes::AddRoute(std::string_view name, RouteInfo info) {
    routes_.insert({ name, info });
}

void PaintDataRoutes::RenderData(std::ostream& out) {
    std::vector<geo::Coordinates> routes;

    for (const auto& [name, info] : routes_) {
        for (const auto& stop : info.stops_) {
            routes.push_back(stop->coordinates);
        }
    }
    const SphereProjector proj{ routes.begin(), routes.end(), set_.width, set_.height, set_.padding };

    RenderPolyline(proj);
    RenderText(proj);
    RenderStop(proj);

    render_doc_.Render(out);
}

void PaintDataRoutes::SetRenderSetting(const RenderSettings& settings) {
    set_ = settings;
}

void PaintDataRoutes::RenderPolyline(const SphereProjector& proj) {
    int set_color = -1;

    for (const auto& [name, route] : routes_) {

        if (!route.stops_.empty()) {
            ++set_color;
        }
        if (static_cast<size_t>(set_color) == set_.color_palette.size()) {
            set_color = 0;
        }

        svg::Polyline line;

        for (const auto& stop : route.stops_) {
            auto stop_coord = stop->coordinates;
            auto coord = proj(stop_coord);
            line.AddPoint(coord);
        }
        line.SetStrokeWidth(set_.line_width).
            SetFillColor("none").
            SetStrokeColor(set_.color_palette.at(set_color)).
            SetStrokeLineCap(svg::StrokeLineCap::ROUND).
            SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        render_doc_.AddPtr(std::make_unique<svg::Polyline>(line));
    }
}

void PaintDataRoutes::RenderText(const SphereProjector& proj) {
    int set_color = -1;

    for (const auto& [name, route] : routes_) {

        if (!route.stops_.empty()) {
            ++set_color;
        }
        if (static_cast<size_t>(set_color) == set_.color_palette.size()) {
            set_color = 0;
        }

        for (const auto& stop : route.beg_end_) {
            svg::Text text_in, text_back;
            auto stop_coord = stop->coordinates;
            auto coord = proj(stop_coord);

            text_back.SetPosition(coord).
                SetOffset({ set_.bus_label_offset[0], set_.bus_label_offset[1] }).
                SetFontSize(set_.bus_label_font_size).
                SetFontFamily("Verdana").
                SetFontWeight("bold").
                SetData(std::string(name)).
                SetFillColor(set_.underlayer_color).
                SetStrokeColor(set_.underlayer_color).
                SetStrokeWidth(set_.underlayer_width).
                SetStrokeLineCap(svg::StrokeLineCap::ROUND).
                SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            render_doc_.AddPtr(std::make_unique<svg::Text>(text_back));

            text_in.SetPosition(coord).
                SetOffset({ set_.bus_label_offset[0], set_.bus_label_offset[1] }).
                SetFontSize(set_.bus_label_font_size).
                SetFontFamily("Verdana").
                SetFontWeight("bold").
                SetData(std::string(name)).
                SetFillColor(set_.color_palette.at(set_color));
            render_doc_.AddPtr(std::make_unique<svg::Text>(text_in));
        }
    }
}

void PaintDataRoutes::RenderStop(const SphereProjector& proj) {
    std::map<std::string_view, const Stop*> stops;

    for (const auto& [name, route] : routes_) {
        for (const auto& stop : route.stops_)
        {
            stops.insert({ stop->name, stop });
        }
    }

    for (const auto& [name, stop] : stops) {
        svg::Circle circle;
        auto stop_coord = stop->coordinates;
        auto coord = proj(stop_coord);

        circle.SetCenter(coord).
            SetRadius(set_.stop_radius).
            SetFillColor("white");

        render_doc_.AddPtr(std::make_unique<svg::Circle>(circle));
    }

    RenderStopName(proj, stops);
}

void PaintDataRoutes::RenderStopName(const SphereProjector& proj, const std::map<std::string_view, const Stop*>& stops) {

    for (const auto& [name, stop] : stops) {
        svg::Text text_in, text_back;
        auto stop_coord = stop->coordinates;
        auto coord = proj(stop_coord);

        text_back.SetPosition(coord).
            SetOffset({ set_.stop_label_offset[0], set_.stop_label_offset[1] }).
            SetFontSize(set_.stop_label_font_size).
            SetFontFamily("Verdana").
            SetData(std::string(name)).
            SetFillColor("black").
            SetFillColor(set_.underlayer_color).
            SetStrokeColor(set_.underlayer_color).
            SetStrokeWidth(set_.underlayer_width).
            SetStrokeLineCap(svg::StrokeLineCap::ROUND).
            SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        render_doc_.AddPtr(std::make_unique<svg::Text>(text_back));

        text_in.SetPosition(coord).
            SetOffset({ set_.stop_label_offset[0], set_.stop_label_offset[1] }).
            SetFontSize(set_.stop_label_font_size).
            SetFontFamily("Verdana").
            SetData(std::string(name)).
            SetFillColor("black");

        render_doc_.AddPtr(std::make_unique<svg::Text>(text_in));
    }
}
