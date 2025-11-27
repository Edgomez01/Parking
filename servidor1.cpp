#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string.h>
#include <cctype>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define NUM_SPOTS 40

using namespace std;

/**
 * 
 * @param spots Recibimos el puntero a las "plazas de parqueo
 * @param numSpots  recibimos la cantidad de plazas
 */

void printParkingStatus(char** spots, int numSpots)
{
	cout << "\n---[ ESTADO DEL PARKING ]---\n";
	for (int i=0; i<numSpots;i++)
	{
		cout << " Plaza " << (i + 1) << ": ";
		if (spots[i] == nullptr)
		{
			cout << "[ VACIO ]";
		}else
		{
			cout << "[ " << spots[i] << " ]";
		}
		cout << endl;
	}
	cout << "----------------------------------\n\n";
}

/**
 * 
 * @param plate Recibimos la placa
 * @return validamos si la palca es correcta, si es de 6 digitos y sigue el formato AAA000
 */

bool isValidPlate(const char* plate)
{
	if (strlen(plate) != 6)
	{
		cout << "La placa ingresada debe ser de 6 digitos.\n";
		return false;
	}
	for (int i=0; i<3; ++i)
	{
		if (!isalpha(plate[i]))
		{
			return false;
		}
	}
	for (int i=3; i<6;++i)
	{
		if (!isdigit(plate[i]))
		{
			return false;
		}
	}
	return true;
}

int findPlate(const char* plate, char** spots, int numSpots)
{
	for (int i =0; i < numSpots; ++i)
	{
		if (spots[i] != nullptr && strcmp(spots[i], plate)==0)
		{
			return i;
		}
	}
	return -1;
}

int main()
{
	WSADATA wsaData;
	SOCKET servidor_fd, nuevo_socket;
	struct sockaddr_in direccion;
	int addrlen = sizeof(direccion);

	// --- GESTION DEL PARKING
	// Cada espacio del arreglo será una plaza

	char** parkingSpots = new char* [NUM_SPOTS];
	for (int i = 0; i < NUM_SPOTS; ++i)
	{
		parkingSpots[i] = nullptr;
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cerr << "Falló la inicialización del winsock. Código de error: " << WSAGetLastError() << endl;
		return 1;
	}

	servidor_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (servidor_fd == INVALID_SOCKET)
	{
		cerr << "Fallo en el socket. Código de error: " << WSAGetLastError() << endl;
		WSACleanup();
		return 1;
	}

	direccion.sin_family = AF_INET;
	direccion.sin_addr.s_addr = INADDR_ANY;
	direccion.sin_port = htons(PORT);

	if (bind(servidor_fd, (struct sockaddr*)&direccion, sizeof(direccion)) == SOCKET_ERROR)
	{
		cerr << "Fallo en bind. Código de error: " << WSAGetLastError() << endl;
		closesocket(servidor_fd);
		WSACleanup();
		return 1;
	}

	if (listen(servidor_fd, 3) == SOCKET_ERROR)
	{
		cerr << "Fallo el listen. Código de error: " << WSAGetLastError() << endl;
		closesocket(servidor_fd);
		WSACleanup();
		return 1;
	}

	cout << "Servidor del parqueadero iniciado. Esperando conexión en el puerto " << PORT << "...\n";

	while (true)
	{
		nuevo_socket = accept(servidor_fd, (struct sockaddr*)&direccion, &addrlen);
		if (nuevo_socket == INVALID_SOCKET)
		{
			cerr << "Fallo el accept. Códigop de error: " << WSAGetLastError() << endl;
			continue;
		}

		cout << "\n¡Cliente conectado! Enviando estado inicial.\n";
		printParkingStatus(parkingSpots, NUM_SPOTS);

		char buffer[1024] = {0};
		int valread;

		while ((valread = recv(nuevo_socket, buffer, 1024, 0)) > 0)
		{
			if (valread < 1024)
			{
				buffer[valread] = '\0';
			}else
			{
				buffer[1023] = '\0';
			}
			cout << "Cliente envia: \"" << buffer << "\"" << endl;

			char* separator = strchr(buffer, ':');
			const char* responseMessage = "mensaje no procesado";

			if (separator == nullptr)
			{
				responseMessage = "ERROR: Formato invalido. Use PUESTO:PLACA";
			}else
			{
				*separator = '\0';
				char* plate = separator + 1;

				int spotIndex = atoi(buffer) - 1;

				if (!isValidPlate(plate))
				{
					responseMessage = "ERROR: Placa invalida. Formato: AAA000";
				}
				else if (spotIndex < 0 || spotIndex >= NUM_SPOTS)
				{
					responseMessage = "ERROR: Puesto invalido. Use 1, 2, 3 ... 40\n";
				}
				else
				{
					int existingSpot = findPlate(plate, parkingSpots, NUM_SPOTS);

					if (existingSpot != -1)
					{
						cout << "-> Procesando SALIDA para " << plate << " de la plaza " << (existingSpot + 1) << endl;

						delete[] parkingSpots[existingSpot];
						parkingSpots[existingSpot] = nullptr;

						responseMessage = "OK: Vehiculo salio. Plaza liberada";
					}
					else
					{
						if (parkingSpots[spotIndex] == nullptr)
						{
							cout << "-> Procesando ENTRADA para " << plate << " en la plaza " << (spotIndex + 1) << endl;
							parkingSpots[spotIndex] = new char[strlen(plate) + 1];
							size_t plate_len = strlen(plate);
							strcpy_s(parkingSpots[spotIndex], plate_len + 1, plate);

							responseMessage = "OK: Vehiculo estacionado";
						}	
						else
						{
							responseMessage = "ERROR: Plaza ya ocupada";
						}
					}
				}
			}

			send(nuevo_socket, responseMessage, static_cast<int>(strlen(responseMessage)), 0);

			printParkingStatus(parkingSpots, NUM_SPOTS);
		}

		if (valread == 0)
		{
			cout << "Cliente desconectado.\n";
		}else
		{
			cerr << "Fallo en recv. Código de error: " << WSAGetLastError() << endl;
		}

		closesocket(nuevo_socket);
	}

	cout << "Cerrando servidor y liberando memoria...\n";
	for (int i=0; i < NUM_SPOTS; ++i)
	{
		if (parkingSpots[i] != nullptr)
		{
			delete[] parkingSpots[i];
		}
	}
	delete[] parkingSpots;

	closesocket(servidor_fd);
	WSACleanup();

	return 0;
}
