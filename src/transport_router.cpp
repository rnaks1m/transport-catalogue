#include "transport_router.h"

TransportRouter::TransportRouter(const transport_catalogue::TransportCatalogue& catalogue, RouteSettings settings) 
    : catalogue_(catalogue), router_(nullptr), settings_(settings) { BuildRouter(); }

double TransportRouter::CalcDistanceAndGetTime(const Stop* from, const Stop* to, double& distance) {
    distance += static_cast<double>(catalogue_.GetDistance(from, to));
    return CalculateTime(distance);
}

void TransportRouter::AddBuses(const std::unordered_map<std::string_view, const Bus*>& buses) {
    for (const auto& [bus_name, bus] : buses) {
        const auto& stops = bus->route;
        int size = 0;

        if (bus->is_roundtrip) {
            size = stops.size();
        }
        else if (stops.size() % 2 == 0) {
            size = stops.size() / 2;
        }
        else {
            size = stops.size() / 2 + 1;
        }

        for (int i(0); i < size; ++i) {
            auto stop_from_id = GetVertexByStop(stops[i]);
            double distance_from_to = 0.0, distance_to_from = 0.0;

            for (int j = i + 1; j < size; ++j) {
                auto stop_to_id = GetVertexByStop(stops[j]);

                double time_from_to = CalcDistanceAndGetTime(stops[j - 1], stops[j], distance_from_to);
                graph_.AddEdge({ stop_from_id + 1, stop_to_id, time_from_to, bus->name, j - i });

                if (!bus->is_roundtrip) {
                    double time_to_from = CalcDistanceAndGetTime(stops[j], stops[j - 1], distance_to_from);
                    graph_.AddEdge({ stop_to_id + 1, stop_from_id, time_to_from, bus->name, std::abs(i - j) });
                }
            }

            if (bus->is_roundtrip) {
                double total_time = CalculateTime(distance_from_to);
                graph_.AddEdge({ stop_from_id + 1, stop_from_id, total_time, bus->name, static_cast<int>(size) });
            }
        }
    }
}

void TransportRouter::BuildRouter() {
    graph::VertexId vertex = 0;
    const auto& buses = catalogue_.GetBuses();

    for (const auto& [bus_name, bus] : buses) {
        for (const auto& stop : bus->route) {
            unique_stops_.emplace(stop);
        }
    }

    graph_ = graph::DirectedWeightedGraph<double>(unique_stops_.size() * 2);

    for (const auto& stop : unique_stops_) {
        stops_vertexes_.insert({ stop, vertex });
        stops_by_vertexes_.push_back(stop);
        vertex += 2;
        graph::VertexId id_stop = GetVertexByStop(stop);
        graph_.AddEdge({ id_stop, id_stop + 1, static_cast<double>(settings_.bus_wait_time), "", 0 });
    }

    AddBuses(buses);

    router_ = std::make_unique<graph::Router<double>>(graph_);
}

std::vector<RouteItems> TransportRouter::FindRoute(std::string stop_from, std::string stop_to) const {
    auto stop_from_id = GetVertexByStop(catalogue_.FindStop(stop_from));
    auto stop_to_id = GetVertexByStop(catalogue_.FindStop(stop_to));
    std::vector<RouteItems> items;
    auto result_route = router_->BuildRoute(stop_from_id, stop_to_id);

    if (!result_route.has_value()) {
        items.push_back({ "error_message", "", 0, 0 });
        return items;
    }

    auto result = result_route.value();
    
    for (int i = 0; i < result.edges.size(); ++i) {
        const auto& edge = graph_.GetEdge(result_route->edges[i]);

        if (edge.from % 2 == 0) {
            auto stop = GetStopByVertex(edge.from);
            items.push_back({ "Wait", stop->name, edge.weight });
        }

        auto bus = catalogue_.FindBus(edge.name);

        if (bus != nullptr) {
            items.push_back({ "Bus", bus->name, edge.weight, edge.span_count });
        }
    }
    return items;
}

graph::VertexId TransportRouter::GetVertexByStop(const Stop* name) const {
    auto it = stops_vertexes_.find(name);
    if (it != stops_vertexes_.end()) {
        return it->second;
    }
    else {
        throw std::out_of_range("Out of range unordered map stops!");
    }
}

const Stop* TransportRouter::GetStopByVertex(graph::VertexId id) const {
    auto index = static_cast<size_t>(id) / 2;
    if (index < stops_by_vertexes_.size()) {
        return stops_by_vertexes_.at(index);
    }
    else {
        throw std::out_of_range("Out of range vector stops!");
    }
}

double TransportRouter::CalculateTime(double distance) const {
    return distance / settings_.bus_velocity / 1000.0 * 60.0;
}