#pragma once

#include "json_builder.h"
#include "map_renderer.h"
#include "router.h"
#include "transport_catalogue.h"
#include "transport_router.h"

class JsonReader {
public:
    JsonReader(std::istream& input);

    void ParseData(transport_catalogue::TransportCatalogue& catalogue);
    void PrintResult(std::ostream& out);
    void RenderDataCatalogue(transport_catalogue::TransportCatalogue& catalogue, std::ostream& out);

private:
    void GetResult(transport_catalogue::TransportCatalogue& catalogue);
    void ParseStopsInCatalogue(transport_catalogue::TransportCatalogue& catalogue);
    void ParseStopDistanceInCatalogue(transport_catalogue::TransportCatalogue& catalogue);
    void ParseBusInCatalogue(transport_catalogue::TransportCatalogue& catalogue);
    void ParseDataForPaint(transport_catalogue::TransportCatalogue& catalogue);
    void ParseRenderSettings();
    void GetResultOfBus(transport_catalogue::TransportCatalogue& catalogue, json::Builder& answer, std::string name);
    void GetResultOfStop(transport_catalogue::TransportCatalogue& catalogue, json::Builder& answer, std::string name);
    void GetResultOfMap(transport_catalogue::TransportCatalogue& catalogue, json::Builder& answer);
    RouteSettings ParseRouteSettings();
    void GetResultOfRoute(transport_catalogue::TransportCatalogue& catalogue, json::Builder& answer, std::string stop_from, std::string stop_to);

private:
    json::Node base_request_;
    json::Node stat_request_;
    json::Node render_settings_;
    json::Node route_settings_;
    json::Array result_;
    std::map<std::string, std::pair<std::vector<std::string_view>, RouteInfoBegEnd>> routes_;
    PaintDataRoutes routes_for_paint_;
    TransportRouter* router_;
};