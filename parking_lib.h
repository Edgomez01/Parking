#ifndef PARKING_LIB_H
#define PARKING_LIB_H

struct VehicleInfo {
    char plate[10];
    char timestamp[30];
    bool occupied;
};

class ParkingManager {
private:
    VehicleInfo spots[40];
    
public:
    ParkingManager();
    int getTotalSpots() const;
    bool isSpotOccupied(int spotIndex) const;
    const char* getPlate(int spotIndex) const;
    bool addVehicle(int spotIndex, const char* plate, const char* timestamp);
    int removeVehicle(const char* plate);
    int findPlate(const char* plate) const;
    int getOccupiedCount() const;
    int getFreeCount() const;
};

#endif