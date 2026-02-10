#include "json_reader.h"

#include <algorithm>
#include <sstream>

JsonReader::JsonReader(std::istream& input) {
    json::Document input_data = json::Load(input);
    base_request_ = input_data.GetRoot().AsMap().at("base_requests");
    stat_request_ = input_data.GetRoot().AsMap().at("stat_requests");
    render_settings_ = input_data.GetRoot().AsMap().at("render_settings");
    route_settings_ = input_data.GetRoot().AsMap().at("routing_settings");
}

void JsonReader::ParseStopsInCatalogue(transport_catalogue::TransportCatalogue& catalogue) {

    for (const auto& request : base_request_.AsArray()) {
        std::string name;
        geo::Coordinates coordinates;

        for (const auto& [request_name, value] : request.AsMap()) {
            if (request_name == "type") {
                if (value.AsString() != "Stop") {
                    name.clear();
                    break;
                }
            }
            else if (request_name == "name") {
                name = value.AsString();
            }
            else if (request_name == "latitude") {
                coordinates.lat = value.AsDouble();
            }
            else if (request_name == "longitude") {
                coordinates.lng = value.AsDouble();
            }
        }
        if (!name.empty()) {
            catalogue.AddStop(name, coordinates);
        }
    }
}

void JsonReader::ParseStopDistanceInCatalogue(transport_catalogue::TransportCatalogue& catalogue) {

    for (const auto& request : base_request_.AsArray()) {
        std::string name;
        std::map<std::string_view, int> road_distances;

        for (const auto& [request_name, value] : request.AsMap()) {
            if (request_name == "type") {
                if (value.AsString() != "Stop") {
                    name.clear();
                    break;
                }
            }
            else if (request_name == "name") {
                name = value.AsString();
            }
            else if (request_name == "road_distances") {
                for (const auto& [name, distance] : value.AsMap()) {
                    road_distances.insert({ name, distance.AsInt() });
                }
            }
        }
        if (!name.empty()) {
            auto stop_from = catalogue.FindStop(name);
            for (const auto& [stop, distance] : road_distances) {
                auto stop_to = catalogue.FindStop(stop);
                catalogue.SetDistance(stop_from, stop_to, distance);
            }
        }
    }
}

void JsonReader::ParseBusInCatalogue(transport_catalogue::TransportCatalogue& catalogue) {

    for (const auto& request : base_request_.AsArray()) {
        std::string name;
        bool is_roundtrip = false;
        std::vector<std::string_view> stops;

        for (const auto& [request_name, value] : request.AsMap()) {
            if (request_name == "type") {
                if (value.AsString() != "Bus") {
                    name.clear();
                    break;
                }
            }
            else if (request_name == "name") {
                name = value.AsString();
            }
            else if (request_name == "is_roundtrip") {
                is_roundtrip = value.AsBool();
            }
            else if (request_name == "stops") {
                for (const auto& stop : value.AsArray()) {
                    stops.push_back(stop.AsString());
                }
            }
        }
        if (!name.empty()) {

            RouteInfoBegEnd info;
            info.begin_route = *stops.begin();
            info.end_route = stops.back();

            if (*stops.begin() == stops.back() && is_roundtrip) {
                catalogue.AddBus(name, stops, is_roundtrip);
                routes_[name] = { stops, info };
                continue;
            }

            std::vector<std::string_view> round_route(stops.begin(), stops.end());

            if (!is_roundtrip) {
                for (auto it = ++stops.rbegin(); it != stops.rend(); ++it) {
                    round_route.push_back(*it);
                }
                catalogue.AddBus(name, round_route, is_roundtrip);
                routes_[name] = { round_route, info };
            }
            else {
                auto it = stops.begin();
                round_route.push_back(*it);
                catalogue.AddBus(name, round_route, is_roundtrip);
                routes_[name] = { round_route, info };
            }
        }
    }
}

RouteSettings JsonReader::ParseRouteSettings() {
    RouteSettings settings;
    for (const auto& [setting_name, value] : route_settings_.AsMap()) {
        if (setting_name == "bus_wait_time") {
            settings.bus_wait_time = value.AsInt();
        }
        else if (setting_name == "bus_velocity") {
            settings.bus_velocity = value.AsDouble();
        }
    }
    return settings;
}

void JsonReader::GetResultOfBus(transport_catalogue::TransportCatalogue& catalogue, json::Builder& answer, std::string name) {
    const auto& result = catalogue.GetBusInfo(name);

    if (result.stops == 0) {
        std::string str = "not found";
        answer.Key("error_message").Value(str).EndDict();
        result_.push_back(answer.Build());
        return;
    }

    answer.Key("stop_count").Value(result.stops);
    answer.Key("unique_stop_count").Value(result.unique_stop);
    answer.Key("route_length").Value(result.route_distance);
    answer.Key("curvature").Value(result.route_distance / result.geo_distance);
    answer.EndDict();

    result_.push_back(answer.Build());
}

void JsonReader::GetResultOfStop(transport_catalogue::TransportCatalogue& catalogue, json::Builder& answer, std::string name) {
    const auto ptr = catalogue.FindStop(name);

    if (ptr == nullptr) {
        std::string str = "not found";
        answer.Key("error_message").Value(str).EndDict();
        result_.push_back(answer.Build());
        return;
    }

    const auto& result = catalogue.GetStopInfo(name);
    std::vector<std::string_view> vec_result(result.begin(), result.end());
    std::sort(vec_result.begin(), vec_result.end());

    answer.Key("buses").StartArray();

    for (const auto& bus : vec_result) {
        answer.Value(std::string(bus));
    }
    answer.EndArray().EndDict();
    result_.push_back(answer.Build());
}

void JsonReader::GetResultOfMap(transport_catalogue::TransportCatalogue& catalogue, json::Builder& answer) {
    std::ostringstream output_map;
    RenderDataCatalogue(catalogue, output_map);
    answer.Key("map").Value(output_map.str()).EndDict();
    result_.push_back(answer.Build());
}

void JsonReader::GetResultOfRoute(transport_catalogue::TransportCatalogue& catalogue, json::Builder& answer, std::string stop_from, std::string stop_to) {
    if (catalogue.GetStopInfo(stop_from).size() == 0 || catalogue.GetStopInfo(stop_to).size() == 0) {
        answer.Key("error_message").Value("not found").EndDict();
        result_.push_back(answer.Build());
        return;
    }

    static TransportRouter router(catalogue, ParseRouteSettings());
    router_ = &router;
    auto items = router_->FindRoute(stop_from, stop_to);
    double route_time = 0.0;

    if (items.size() == 1 && items[0].type == "error_message") {
        answer.Key("error_message").Value("not found").EndDict();
        result_.push_back(answer.Build());
        return;
    }

    answer.Key("items").StartArray();
    for (const auto& item : items) {
        route_time += item.time;
        answer.StartDict().Key("type").Value(item.type.data()).Key("time").Value(item.time);
        if (item.type == "Bus") {
            answer.Key("bus").Value(item.name.data()).Key("span_count").Value(item.span_count);
        }
        else if (item.type == "Wait") {
            answer.Key("stop_name").Value(item.name.data());
        }
        answer.EndDict();
    }
    answer.EndArray().Key("total_time").Value(route_time).EndDict();
    result_.push_back(answer.Build());
}

void JsonReader::GetResult(transport_catalogue::TransportCatalogue& catalogue) {
    for (const auto& request : stat_request_.AsArray()) {
        std::string name, type, stop_from, stop_to;
        int id = 0;
        json::Builder answer;

        for (const auto& [request_name, value] : request.AsMap()) {
            if (request_name == "type") {
                type = value.AsString();
            }
            else if (request_name == "name") {
                name = value.AsString();
            }
            else if (request_name == "id") {
                id = value.AsInt();
            }
            else if (request_name == "from") {
                stop_from = value.AsString();
            }
            else if (request_name == "to") {
                stop_to = value.AsString();
            }
        }

        answer.StartDict().Key("request_id").Value(id);

        if (type == "Bus") {
            GetResultOfBus(catalogue, answer, name);
        }
        else if (type == "Stop") {
            GetResultOfStop(catalogue, answer, name);
        }
        else if (type == "Map") {
            GetResultOfMap(catalogue, answer);
        }
        else if (type == "Route") {
            GetResultOfRoute(catalogue, answer, stop_from, stop_to);
        }
    }
}

void JsonReader::ParseData(transport_catalogue::TransportCatalogue& catalogue) {

    // добавление данных в справочник
    ParseStopsInCatalogue(catalogue);
    ParseStopDistanceInCatalogue(catalogue);
    ParseBusInCatalogue(catalogue);
    ParseRenderSettings();
    GetResult(catalogue);
}

void JsonReader::PrintResult(std::ostream& out) {
    json::Document doc(result_);
    if (!doc.GetRoot().AsArray().empty()) {
        json::Print(doc, out);
    }  
}

void JsonReader::ParseDataForPaint(transport_catalogue::TransportCatalogue& catalogue){
    std::vector<const Stop*> routes_points;
    for (const auto& [name, route] : routes_) {
        RouteInfo info;

        for (const auto& stop : route.first) {
            auto st = catalogue.FindStop(stop);
            routes_points.push_back(st);
            if (st->name == route.second.begin_route && info.beg_end_.empty()) {
                info.beg_end_.push_back(st);
            }
            if (st->name == route.second.end_route && st->name != route.second.begin_route && info.beg_end_.size() == 1) {
                info.beg_end_.push_back(st);
            }
        }
        info.stops_ = routes_points;
        routes_for_paint_.AddRoute(name, info);
        routes_points.clear();
    }
}

void JsonReader::RenderDataCatalogue(transport_catalogue::TransportCatalogue& catalogue, std::ostream& out) {
    ParseDataForPaint(catalogue);
    routes_for_paint_.RenderData(out);
}

void JsonReader::ParseRenderSettings() {
    RenderSettings settings;

    for (const auto& [name, set] : render_settings_.AsMap()) {
        if (name == "width") {
            settings.width = set.AsDouble();
        }
        else if (name == "height") {
            settings.height = set.AsDouble();
        }
        else if (name == "padding") {
            settings.padding = set.AsDouble();
        }
        else if (name == "line_width") {
            settings.line_width = set.AsDouble();
        }
        else if (name == "stop_radius") {
            settings.stop_radius = set.AsDouble();
        }
        else if (name == "bus_label_font_size") {
            settings.bus_label_font_size = set.AsInt();
        }
        else if (name == "bus_label_offset") {
            const auto& array = set.AsArray();
            int i(0);
            for (const auto& settinng : array) {
                settings.bus_label_offset[i] = settinng.AsDouble();
                ++i;
            }
        }
        else if (name == "stop_label_font_size") {
            settings.stop_label_font_size = set.AsInt();
        }
        else if (name == "stop_label_offset") {
            const auto& array = set.AsArray();
            int i(0);
            for (const auto& settinng : array) {
                settings.stop_label_offset[i] = settinng.AsDouble();
                ++i;
            }
        }
        else if (name == "underlayer_color") {
            if (set.IsString()) {
                settings.underlayer_color = set.AsString();
            }
            else if (set.IsArray()) {
                const auto& array = set.AsArray();
                if (array.size() == 3) {
                    svg::Rgb color(array[0].AsInt(), array[1].AsInt(), array[2].AsInt());
                    settings.underlayer_color = color;
                }
                else {
                    svg::Rgba color(array[0].AsInt(), array[1].AsInt(), array[2].AsInt(), array[3].AsDouble());
                    settings.underlayer_color = color;
                }
            }
        }
        else if (name == "underlayer_width") {
            settings.underlayer_width = set.AsDouble();
        }
        else if (name == "color_palette") {
            for (const auto& setting : set.AsArray()) {
                if (setting.IsString()) {
                    settings.color_palette.push_back(setting.AsString());
                }
                else if (setting.IsArray()) {
                    const auto& array = setting.AsArray();
                    if (array.size() == 3) {
                        svg::Rgb color(array[0].AsInt(), array[1].AsInt(), array[2].AsInt());
                        settings.color_palette.push_back(color);
                    }
                    else {
                        svg::Rgba color(array[0].AsInt(), array[1].AsInt(), array[2].AsInt(), array[3].AsDouble());
                        settings.color_palette.push_back(color);
                    }
                }
            }
        }
    }

    routes_for_paint_.SetRenderSetting(settings);
}