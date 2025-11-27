// ============================================================================
// ARCHIVO: servidor_multicliente.cpp
// PROPÓSITO: Servidor de parqueadero con soporte para MÚLTIPLES CLIENTES
// DESCRIPCIÓN: Usa threads para manejar varios clientes simultáneamente
//              Permite que cliente.exe Y visualizador se conecten al mismo tiempo
// ============================================================================

#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>      // Para std::string y to_string
#include <string.h>    // Para strchr, strcmp, strlen
#include <cctype>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>   // Para std::remove

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define NUM_SPOTS 40

using namespace std;

// ============================================================================
// VARIABLES GLOBALES COMPARTIDAS (protegidas por mutex)
// ============================================================================

// Arreglo de plazas compartido entre todos los threads
char** parkingSpots = nullptr;

// Mutex para sincronizar acceso al arreglo de plazas
// Evita que dos threads modifiquen el arreglo simultáneamente
mutex parkingMutex;

// Vector con todos los sockets de clientes conectados
vector<SOCKET> connectedClients;
mutex clientsMutex;

// ============================================================================
// FUNCIÓN: printParkingStatus
// ============================================================================
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

// ============================================================================
// FUNCIÓN: isValidPlate
// ============================================================================
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

// ============================================================================
// FUNCIÓN: findPlate
// ============================================================================
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

// ============================================================================
// FUNCIÓN: broadcastMessage
// PROPÓSITO: Envía un mensaje a TODOS los clientes conectados
// ============================================================================
void broadcastMessage(const string& message, SOCKET excludeSocket = INVALID_SOCKET)
{
	// BLOQUEAR ACCESO AL VECTOR DE CLIENTES
	lock_guard<mutex> lock(clientsMutex);
	
	// ENVIAR A TODOS LOS CLIENTES (excepto al que envió el mensaje original)
	for (auto it = connectedClients.begin(); it != connectedClients.end(); )
	{
		SOCKET clientSocket = *it;
		
		// No enviar al cliente que originó el mensaje
		if (clientSocket != excludeSocket)
		{
			int result = send(clientSocket, message.c_str(), (int)message.length(), 0);
			
			// Si falla el envío, el cliente se desconectó
			if (result == SOCKET_ERROR)
			{
				cout << "⚠ Cliente desconectado durante broadcast\n";
				closesocket(clientSocket);
				it = connectedClients.erase(it);
				continue;
			}
		}
		++it;
	}
}

// ============================================================================
// FUNCIÓN: handleClient (SE EJECUTA EN UN THREAD SEPARADO PARA CADA CLIENTE)
// PROPÓSITO: Maneja la comunicación con un cliente específico
// ============================================================================
void handleClient(SOCKET clientSocket)
{
	char buffer[1024] = { 0 };
	int valread;

	cout << "[+] Nuevo cliente conectado (Socket: " << clientSocket << ")\n";

	// BUCLE DE RECEPCIÓN DE MENSAJES
	while ((valread = recv(clientSocket, buffer, 1024, 0)) > 0)
	{
		// Agregar terminador nulo
		if (valread < 1024)
		{
			buffer[valread] = '\0';
		}
		else
		{
			buffer[1023] = '\0';
		}

		cout << ">> Cliente " << clientSocket << " envia: \"" << buffer << "\"\n";

		// PARSEAR MENSAJE (formato: "PLAZA:PLACA:TIMESTAMP")
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
			
			// Buscar el segundo separador (:)
			char* separator2 = strchr(rest, ':');
			char* plate = rest;
			char* timestamp = nullptr;
			
			if (separator2 != nullptr)
			{
				*separator2 = '\0';
				timestamp = separator2 + 1;
			}
			
			int spotIndex = atoi(buffer) - 1;

			// BLOQUEAR ACCESO AL ARREGLO DE PLAZAS (CRITICAL SECTION)
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
					// SALIDA: liberar plaza
					cout << "[-] SALIDA:\n";
					cout << "    Plaza: " << (existingSpot + 1) << "\n";
					cout << "    Placa: " << plate << "\n";
					if (timestamp) cout << "    Hora: " << timestamp << "\n";
					
					delete[] parkingSpots[existingSpot];
					parkingSpots[existingSpot] = nullptr;
					responseMessage = "OK: Vehiculo salio. Plaza liberada";
					
					// Mensaje para broadcast a otros clientes
					broadcastMsg = to_string(existingSpot + 1) + ":SALIDA";
				}
				else
				{
					if (parkingSpots[spotIndex] == nullptr)
					{
						// ENTRADA: ocupar plaza
						cout << "[+] ENTRADA:\n";
						cout << "    Plaza: " << (spotIndex + 1) << "\n";
						cout << "    Placa: " << plate << "\n";
						if (timestamp) cout << "    Hora: " << timestamp << "\n";
						
						parkingSpots[spotIndex] = new char[strlen(plate) + 1];
						strcpy_s(parkingSpots[spotIndex], strlen(plate) + 1, plate);
						responseMessage = "OK: Vehiculo estacionado";
						
						// Mensaje para broadcast: "PLAZA:PLACA:TIMESTAMP"
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

			// Mostrar estado actualizado
			printParkingStatus(parkingSpots, NUM_SPOTS);

			// DESBLOQUEAR ACCESO AL ARREGLO
			parkingMutex.unlock();
		}

		// ENVIAR RESPUESTA AL CLIENTE QUE HIZO LA SOLICITUD
		send(clientSocket, responseMessage, (int)strlen(responseMessage), 0);

		// ENVIAR ACTUALIZACIÓN A TODOS LOS DEMÁS CLIENTES
		if (!broadcastMsg.empty())
		{
			broadcastMessage(broadcastMsg, clientSocket);
		}
	}

	// CLIENTE DESCONECTADO
	cout << "[-] Cliente " << clientSocket << " desconectado\n";
	
	// Remover de la lista de clientes conectados
	clientsMutex.lock();
	connectedClients.erase(
		remove(connectedClients.begin(), connectedClients.end(), clientSocket),
		connectedClients.end()
	);
	clientsMutex.unlock();
	
	closesocket(clientSocket);
}

// ============================================================================
// FUNCIÓN PRINCIPAL
// ============================================================================
int main()
{
	WSADATA wsaData;
	SOCKET servidor_fd;
	struct sockaddr_in direccion;
	int addrlen = sizeof(direccion);

	// INICIALIZAR ARREGLO DE PLAZAS
	parkingSpots = new char* [NUM_SPOTS];
	for (int i = 0; i < NUM_SPOTS; ++i)
	{
		parkingSpots[i] = nullptr;
	}

	// INICIALIZAR WINSOCK
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cerr << "✗ Error al inicializar Winsock\n";
		return 1;
	}

	// CREAR SOCKET
	servidor_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (servidor_fd == INVALID_SOCKET)
	{
		cerr << "✗ Error al crear socket\n";
		WSACleanup();
		return 1;
	}

	// CONFIGURAR DIRECCIÓN
	direccion.sin_family = AF_INET;
	direccion.sin_addr.s_addr = INADDR_ANY;
	direccion.sin_port = htons(PORT);

	// BIND
	if (bind(servidor_fd, (struct sockaddr*)&direccion, sizeof(direccion)) == SOCKET_ERROR)
	{
		cerr << "✗ Error en bind\n";
		closesocket(servidor_fd);
		WSACleanup();
		return 1;
	}

	// LISTEN
	if (listen(servidor_fd, 10) == SOCKET_ERROR)  // Cola de hasta 10 conexiones
	{
		cerr << "✗ Error en listen\n";
		closesocket(servidor_fd);
		WSACleanup();
		return 1;
	}

	cout << "\n";
	cout << "========================================================\n";
	cout << "  SERVIDOR MULTICLIENTE - PARQUEADERO\n";
	cout << "========================================================\n";
	cout << "[OK] Servidor iniciado en puerto " << PORT << "\n";
	cout << "[*] Gestiona " << NUM_SPOTS << " plazas\n";
	cout << "[*] Soporta MULTIPLES clientes simultaneamente\n";
	cout << "[*] Esperando conexiones...\n";
	cout << "========================================================\n\n";

	// BUCLE PRINCIPAL: ACEPTAR CLIENTES
	while (true)
	{
		SOCKET nuevo_socket = accept(servidor_fd, (struct sockaddr*)&direccion, &addrlen);
		
		if (nuevo_socket == INVALID_SOCKET)
		{
			cerr << "✗ Error en accept\n";
			continue;
		}

		// AGREGAR A LA LISTA DE CLIENTES CONECTADOS
		clientsMutex.lock();
		connectedClients.push_back(nuevo_socket);
		clientsMutex.unlock();

		// CREAR UN NUEVO THREAD PARA ESTE CLIENTE
		thread clientThread(handleClient, nuevo_socket);
		
		// Detach = el thread se ejecuta independientemente
		clientThread.detach();
	}

	// LIMPIEZA (nunca se alcanza en este diseño)
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
