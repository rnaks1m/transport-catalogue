#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"

#include <iostream>
#include <string>

using namespace std;

int main() {
    using namespace std;

    transport_catalogue::TransportCatalogue catalogue;
    JsonReader reader(std::cin);
    reader.ParseData(catalogue);
    reader.PrintResult(std::cout);
}