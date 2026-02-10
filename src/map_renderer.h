#pragma once

#include "domain.h"
#include "svg.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <vector>

inline const double EPSILON = 1e-6;

bool IsZero(double value);

class SphereProjector {
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
        double max_width, double max_height, double padding);

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const;

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

template <typename PointInputIt>
SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end,
    double max_width, double max_height, double padding)
    : padding_(padding) //
{
    // Если точки поверхности сферы не заданы, вычислять нечего
    if (points_begin == points_end) {
        return;
    }

    // Находим точки с минимальной и максимальной долготой
    const auto [left_it, right_it] = std::minmax_element(
        points_begin, points_end,
        [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
    min_lon_ = left_it->lng;
    const double max_lon = right_it->lng;

    // Находим точки с минимальной и максимальной широтой
    const auto [bottom_it, top_it] = std::minmax_element(
        points_begin, points_end,
        [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
    const double min_lat = bottom_it->lat;
    max_lat_ = top_it->lat;

    // Вычисляем коэффициент масштабирования вдоль координаты x
    std::optional<double> width_zoom;
    if (!IsZero(max_lon - min_lon_)) {
        width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
    }

    // Вычисляем коэффициент масштабирования вдоль координаты y
    std::optional<double> height_zoom;
    if (!IsZero(max_lat_ - min_lat)) {
        height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
    }

    if (width_zoom && height_zoom) {
        // Коэффициенты масштабирования по ширине и высоте ненулевые,
        // берём минимальный из них
        zoom_coeff_ = std::min(*width_zoom, *height_zoom);
    }
    else if (width_zoom) {
        // Коэффициент масштабирования по ширине ненулевой, используем его
        zoom_coeff_ = *width_zoom;
    }
    else if (height_zoom) {
        // Коэффициент масштабирования по высоте ненулевой, используем его
        zoom_coeff_ = *height_zoom;
    }
}

struct RenderSettings {
    double width = 0; 
    double height = 0;
    double padding = 0;
    double line_width = 0;
    double stop_radius = 0;
    

    int bus_label_font_size = 0;
    double bus_label_offset[2] = { 0, 0 };
    int stop_label_font_size = 0;
    double stop_label_offset[2] = { 0, 0 };

    svg::Color underlayer_color;
    double underlayer_width = 0;
    std::vector<svg::Color> color_palette;
};

struct RouteInfoBegEnd {
    std::string_view begin_route;
    std::string_view end_route;
};

struct RouteInfo {
    std::vector<const Stop*> beg_end_;
    std::vector<const Stop*> stops_;
};

class PaintDataRoutes {
public:
    void AddRoute(std::string_view, RouteInfo info);
    void RenderData(std::ostream& out);
    void SetRenderSetting(const RenderSettings& settings);
private:
    void RenderPolyline(const SphereProjector& proj);
    void RenderText(const SphereProjector& proj);
    void RenderStop(const SphereProjector& proj);
    void RenderStopName(const SphereProjector& proj, const std::map<std::string_view, const Stop*>& stops);

private:
    std::map<std::string_view, RouteInfo> routes_;
    RenderSettings set_;
    svg::Document render_doc_;
};