#include "parking_lib.h"
#include <cstring>

ParkingManager::ParkingManager() {
    for (int i = 0; i < 40; ++i) {
        spots[i].occupied = false;
        spots[i].plate[0] = '\0';
        spots[i].timestamp[0] = '\0';
    }
}

int ParkingManager::getTotalSpots() const {
    return 40;
}

bool ParkingManager::isSpotOccupied(int spotIndex) const {
    if (spotIndex < 0 || spotIndex >= 40) return false;
    return spots[spotIndex].occupied;
}

const char* ParkingManager::getPlate(int spotIndex) const {
    if (spotIndex < 0 || spotIndex >= 40 || !spots[spotIndex].occupied) return "";
    return spots[spotIndex].plate;
}

bool ParkingManager::addVehicle(int spotIndex, const char* plate, const char* timestamp) {
    if (spotIndex < 0 || spotIndex >= 40 || spots[spotIndex].occupied) return false;
    
    strncpy(spots[spotIndex].plate, plate, 9);
    spots[spotIndex].plate[9] = '\0';
    strncpy(spots[spotIndex].timestamp, timestamp, 29);
    spots[spotIndex].timestamp[29] = '\0';
    spots[spotIndex].occupied = true;
    return true;
}

int ParkingManager::removeVehicle(const char* plate) {
    int spotIndex = findPlate(plate);
    if (spotIndex == -1) return -1;
    
    spots[spotIndex].occupied = false;
    spots[spotIndex].plate[0] = '\0';
    spots[spotIndex].timestamp[0] = '\0';
    return spotIndex;
}

int ParkingManager::findPlate(const char* plate) const {
    for (int i = 0; i < 40; ++i) {
        if (spots[i].occupied && strcmp(spots[i].plate, plate) == 0) return i;
    }
    return -1;
}

int ParkingManager::getOccupiedCount() const {
    int count = 0;
    for (int i = 0; i < 40; ++i) {
        if (spots[i].occupied) count++;
    }
    return count;
}

int ParkingManager::getFreeCount() const {
    return 40 - getOccupiedCount();
}