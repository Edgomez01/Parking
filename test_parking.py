# test_parking.py
import parking

pm = parking.ParkingManager()
print(f"Total: {pm.getTotalSpots()}")

# Agregar vehículo
pm.addVehicle(0, "ABC123", "2024-11-25 10:30:00")
print(f"Plaza 1 ocupada: {pm.isSpotOccupied(0)}")
print(f"Placa en plaza 1: {pm.getPlate(0)}")