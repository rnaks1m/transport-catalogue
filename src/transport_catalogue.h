#pragma once

#include "domain.h"
#include "geo.h"
#include "graph.h"

#include <deque>
#include <string>
#include <string_view>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace transport_catalogue {

	struct BusInfo {
		double route_distance = 0;
		double geo_distance = 0;
		int stops = 0;
		int unique_stop = 0;
	};

	struct Hasher {
		size_t operator() (const std::pair<const Stop*, const Stop*> elem) const {
			size_t h_x = hasher_(elem.first);
			size_t h_y = hasher_(elem.second);
			return h_x + h_y * 37;
		}

	private:
		std::hash<const Stop*> hasher_;
	};

	class TransportCatalogue {
		// Реализуйте класс самостоятельно
	public:
		void AddStop(const std::string& name, geo::Coordinates coordinates);
		void AddBus(const std::string& name, const std::vector<std::string_view>& data, bool is_roundtrip);
		const Stop* FindStop(const std::string_view name) const;
		const Bus* FindBus(const std::string_view name) const;
		BusInfo GetBusInfo(const std::string_view name) const;
		const std::unordered_set<std::string_view>& GetStopInfo(const std::string_view name) const;
		void SetDistance(const Stop* stop_from, const Stop* stop_to, int distance);
		int GetDistance(const Stop* stop_from, const Stop* stop_to) const;
		const std::unordered_map<std::string_view, const Stop*>& GetStops() const;
		const std::unordered_map<std::string_view, const Bus*>& GetBuses() const;

	private:
		std::deque<Stop> stops_;
		std::deque<Bus> buses_;
		std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
		std::unordered_map<std::string_view, const Bus*> busname_to_bus_;
		std::unordered_map<const Stop*, std::unordered_set<std::string_view>> buses_in_stops_;
		std::unordered_map<std::pair<const Stop*, const Stop*>, int, Hasher> distances_;
		std::unordered_map<std::string_view, graph::VertexId> stops_vertex_wait_;
		std::unordered_map<std::string_view, graph::VertexId> stops_vertex_bus_;
	};
}