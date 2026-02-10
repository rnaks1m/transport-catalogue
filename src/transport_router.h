#pragma once

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

#include<memory>

struct RouteItems {
    std::string_view type;
    std::string_view name;
    double time;
    int span_count = 0;
};

struct RouteSettings {
    int bus_wait_time = 0;
    double bus_velocity = 0.0;
};

class TransportRouter {
public:
    TransportRouter(const transport_catalogue::TransportCatalogue& catalogue, RouteSettings settings);
    std::vector<RouteItems> FindRoute(std::string stop_from, std::string stop_to) const;

private:
    void BuildRouter();
    graph::VertexId GetVertexByStop(const Stop* name) const;
    const Stop* GetStopByVertex(graph::VertexId id) const;
    double CalculateTime(double distance) const;
    double CalcDistanceAndGetTime(const Stop* from, const Stop* to, double& distance);
    void AddBuses(const std::unordered_map<std::string_view, const Bus*>& buses);

    const transport_catalogue::TransportCatalogue& catalogue_;
    graph::DirectedWeightedGraph<double> graph_;
    std::unique_ptr<graph::Router<double>> router_;
    std::unordered_set<const Stop*> unique_stops_;
    std::unordered_map<const Stop*, graph::VertexId> stops_vertexes_;
    std::vector<const Stop*> stops_by_vertexes_;
    RouteSettings settings_;
};