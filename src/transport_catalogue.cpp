#include "transport_catalogue.h"

#include <cmath>
#include <exception>
#include <iomanip>
#include <iostream>
#include <functional>
#include <sstream>

using namespace transport_catalogue;

void TransportCatalogue::AddStop(const std::string& name, geo::Coordinates coordinates) {
    stops_.push_back({ name, coordinates });
    stopname_to_stop_.insert({ stops_.back().name, &stops_.back() });
    graph::VertexId id = stops_.size() * 2;
    stops_vertex_wait_.insert({ stops_.back().name, id - 1 });
    stops_vertex_bus_.insert({ stops_.back().name, id });
}

void TransportCatalogue::AddBus(const std::string& name, const std::vector<std::string_view>& data, bool is_roundtrip) {
    std::vector<const Stop*> route;
    for (auto stop : data) {
        auto ptr = FindStop(std::string(stop));
        route.push_back(ptr);
    }

    buses_.push_back({ name, route, is_roundtrip });
    auto& bus = buses_.back();
    busname_to_bus_.insert({ bus.name, &bus });

    for (auto stop : bus.route) {
        buses_in_stops_[stop].insert(bus.name);
    }
}

const Stop* TransportCatalogue::FindStop(const std::string_view name) const {
    auto it = stopname_to_stop_.find(name);

    if (it != stopname_to_stop_.end()) {
        return it->second;
    }
    return nullptr;
}

const Bus* TransportCatalogue::FindBus(const std::string_view name) const {
    auto it = busname_to_bus_.find(name);

    if (it == busname_to_bus_.end()) {
        return nullptr;
    }

    return it->second;
}

BusInfo TransportCatalogue::GetBusInfo(const std::string_view name) const {

    BusInfo bus_info;
    std::unordered_set<const Stop*> unique_stop;
    auto bus = FindBus(name);

    if (bus != nullptr) {
        bus_info.stops = bus->route.size();
    }
    else {
        return bus_info;
    }

    for (auto it = bus->route.begin(); it != bus->route.end(); ++it) {
        auto stop = *it;

        if (it != bus->route.begin()) {
            auto bef_it = it;
            --bef_it;
            auto bef_stop = *bef_it;
            bus_info.geo_distance += ComputeDistance(bef_stop->coordinates, stop->coordinates);
            bus_info.route_distance += GetDistance(bef_stop, stop);
        }

        unique_stop.insert(stop);
    }
    bus_info.unique_stop = unique_stop.size();

    return bus_info;
}

const std::unordered_set<std::string_view>& TransportCatalogue::GetStopInfo(const std::string_view name) const {
    static std::unordered_set<std::string_view> names;
    const auto ptr = FindStop(name);
    auto it_ptr = buses_in_stops_.find(ptr);

    if (it_ptr != buses_in_stops_.end()) {
        return it_ptr->second;
    }
    else {
        return names;
    }
}

void TransportCatalogue::SetDistance(const Stop* stop_from, const Stop* stop_to, int distance) {

    distances_[{stop_from, stop_to}] = distance;

    if (distances_[{stop_to, stop_from}] == 0) {
        distances_[{stop_to, stop_from}] = distance;
    }
}

int TransportCatalogue::GetDistance(const Stop* stop_from, const Stop* stop_to) const {
    auto it = distances_.find({ stop_from ,stop_to });
    if (it == distances_.end()) {
        return geo::ComputeDistance(stop_from->coordinates, stop_to->coordinates);
    }
    return it->second;
}

const std::unordered_map<std::string_view, const Stop*>& TransportCatalogue::GetStops() const {
    return stopname_to_stop_;
}

const std::unordered_map<std::string_view, const Bus*>& TransportCatalogue::GetBuses() const {
    return busname_to_bus_;
}