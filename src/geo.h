#pragma once

namespace geo {

    struct Coordinates {
        double lat = 0; // Широта
        double lng = 0; // Долгота

        bool operator==(const Coordinates& other) const;
        bool operator!=(const Coordinates& other) const;
    };

    double ComputeDistance(Coordinates from, Coordinates to);

}  // namespace geo