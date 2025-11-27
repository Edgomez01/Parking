#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <string.h>
#include <cctype>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define NUM_SPOTS 40

using namespace std;

// Variables globales protegidas por mutex
char** parkingSpots = nullptr;
mutex parkingMutex;
vector<SOCKET> connectedClients;
mutex clientsMutex;

void printParkingStatus(char** spots, int numSpots)
{
	cout << "\n---[ ESTADO DEL PARKING ]---\n";
	for (int i = 0; i < numSpots; i++)
	{
		cout << " Plaza " << (i + 1) << ": ";
		if (spots[i] == nullptr)
		{
			cout << "[ VACIO ]";
		}
		else
		{
			cout << "[ " << spots[i] << " ]";
		}
		cout << endl;
	}
	cout << "----------------------------------\n\n";
}

bool isValidPlate(const char* plate)
{
	if (strlen(plate) != 6)
	{
		return false;
	}
	for (int i = 0; i < 3; ++i)
	{
		if (!isalpha(plate[i]))
		{
			return false;
		}
	}
	for (int i = 3; i < 6; ++i)
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
	for (int i = 0; i < numSpots; ++i)
	{
		if (spots[i] != nullptr && strcmp(spots[i], plate) == 0)
		{
			return i;
		}
	}
	return -1;
}

// Envía mensaje a todos los clientes conectados
void broadcastMessage(const string& message, SOCKET excludeSocket = INVALID_SOCKET)
{
	lock_guard<mutex> lock(clientsMutex);
	
	for (auto it = connectedClients.begin(); it != connectedClients.end(); )
	{
		SOCKET clientSocket = *it;
		
		if (clientSocket != excludeSocket)
		{
			int result = send(clientSocket, message.c_str(), (int)message.length(), 0);
			
			if (result == SOCKET_ERROR)
			{
				closesocket(clientSocket);
				it = connectedClients.erase(it);
				continue;
			}
		}
		++it;
	}
}

// Maneja comunicación con un cliente (ejecutado en thread separado)
void handleClient(SOCKET clientSocket)
{
	char buffer[1024] = { 0 };
	int valread;

	cout << "[+] Nuevo cliente conectado (Socket: " << clientSocket << ")\n";

	while ((valread = recv(clientSocket, buffer, 1024, 0)) > 0)
	{
		if (valread < 1024)
		{
			buffer[valread] = '\0';
		}
		else
		{
			buffer[1023] = '\0';
		}

		cout << ">> Cliente " << clientSocket << " envia: \"" << buffer << "\"\n";

		// Parsear mensaje formato "PLAZA:PLACA:TIMESTAMP"
		char* separator1 = strchr(buffer, ':');
		const char* responseMessage = "mensaje no procesado";
		string broadcastMsg = "";

		if (separator1 == nullptr)
		{
			responseMessage = "ERROR: Formato invalido. Use PUESTO:PLACA:TIMESTAMP";
		}
		else
		{
			*separator1 = '\0';
			char* rest = separator1 + 1;
			
			char* separator2 = strchr(rest, ':');
			char* plate = rest;
			char* timestamp = nullptr;
			
			if (separator2 != nullptr)
			{
				*separator2 = '\0';
				timestamp = separator2 + 1;
			}
			
			int spotIndex = atoi(buffer) - 1;

			parkingMutex.lock();

			if (!isValidPlate(plate))
			{
				responseMessage = "ERROR: Placa invalida. Formato: AAA000";
			}
			else if (spotIndex < 0 || spotIndex >= NUM_SPOTS)
			{
				responseMessage = "ERROR: Puesto invalido. Use 1, 2, 3 ... 40";
			}
			else
			{
				int existingSpot = findPlate(plate, parkingSpots, NUM_SPOTS);

				if (existingSpot != -1)
				{
					// SALIDA: placa repetida libera plaza
					cout << "[-] SALIDA: Plaza " << (existingSpot + 1) << " | " << plate;
					if (timestamp) cout << " | " << timestamp;
					cout << "\n";
					
					delete[] parkingSpots[existingSpot];
					parkingSpots[existingSpot] = nullptr;
					responseMessage = "OK: Vehiculo salio. Plaza liberada";
					
					broadcastMsg = to_string(existingSpot + 1) + ":SALIDA";
				}
				else
				{
					if (parkingSpots[spotIndex] == nullptr)
					{
						// ENTRADA: ocupar plaza
						cout << "[+] ENTRADA: Plaza " << (spotIndex + 1) << " | " << plate;
						if (timestamp) cout << " | " << timestamp;
						cout << "\n";
						
						parkingSpots[spotIndex] = new char[strlen(plate) + 1];
						strcpy_s(parkingSpots[spotIndex], strlen(plate) + 1, plate);
						responseMessage = "OK: Vehiculo estacionado";
						
						broadcastMsg = string(buffer) + ":" + plate;
						if (timestamp)
						{
							broadcastMsg += ":" + string(timestamp);
						}
					}
					else
					{
						responseMessage = "ERROR: Plaza ya ocupada";
					}
				}
			}

			printParkingStatus(parkingSpots, NUM_SPOTS);
			parkingMutex.unlock();
		}

		send(clientSocket, responseMessage, (int)strlen(responseMessage), 0);

		if (!broadcastMsg.empty())
		{
			broadcastMessage(broadcastMsg, clientSocket);
		}
	}

	cout << "[-] Cliente " << clientSocket << " desconectado\n";
	
	clientsMutex.lock();
	connectedClients.erase(
		remove(connectedClients.begin(), connectedClients.end(), clientSocket),
		connectedClients.end()
	);
	clientsMutex.unlock();
	
	closesocket(clientSocket);
}

int main()
{
	WSADATA wsaData;
	SOCKET servidor_fd;
	struct sockaddr_in direccion;
	int addrlen = sizeof(direccion);

	parkingSpots = new char* [NUM_SPOTS];
	for (int i = 0; i < NUM_SPOTS; ++i)
	{
		parkingSpots[i] = nullptr;
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cerr << "Fallo la inicializacion del winsock. Codigo de error: " << WSAGetLastError() << endl;
		return 1;
	}

	servidor_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (servidor_fd == INVALID_SOCKET)
	{
		cerr << "Fallo en el socket. Codigo de error: " << WSAGetLastError() << endl;
		WSACleanup();
		return 1;
	}

	direccion.sin_family = AF_INET;
	direccion.sin_addr.s_addr = INADDR_ANY;
	direccion.sin_port = htons(PORT);

	if (bind(servidor_fd, (struct sockaddr*)&direccion, sizeof(direccion)) == SOCKET_ERROR)
	{
		cerr << "Fallo en bind. Codigo de error: " << WSAGetLastError() << endl;
		closesocket(servidor_fd);
		WSACleanup();
		return 1;
	}

	if (listen(servidor_fd, 10) == SOCKET_ERROR)
	{
		cerr << "Fallo el listen. Codigo de error: " << WSAGetLastError() << endl;
		closesocket(servidor_fd);
		WSACleanup();
		return 1;
	}

	cout << "\n======================================\n";
	cout << "  SERVIDOR MULTICLIENTE - PARQUEADERO\n";
	cout << "======================================\n";
	cout << "[OK] Iniciado en puerto " << PORT << "\n";
	cout << "[*] Gestiona " << NUM_SPOTS << " plazas\n";
	cout << "[*] Esperando conexiones...\n";
	cout << "======================================\n\n";

	while (true)
	{
		SOCKET nuevo_socket = accept(servidor_fd, (struct sockaddr*)&direccion, &addrlen);
		
		if (nuevo_socket == INVALID_SOCKET)
		{
			cerr << "Fallo el accept. Codigo de error: " << WSAGetLastError() << endl;
			continue;
		}

		clientsMutex.lock();
		connectedClients.push_back(nuevo_socket);
		clientsMutex.unlock();

		thread clientThread(handleClient, nuevo_socket);
		clientThread.detach();
	}

	closesocket(servidor_fd);
	WSACleanup();

	for (int i = 0; i < NUM_SPOTS; ++i)
	{
		if (parkingSpots[i] != nullptr)
		{
			delete[] parkingSpots[i];
		}
	}
	delete[] parkingSpots;

	return 0;
}
