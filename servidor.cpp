// ============================================================================
// ARCHIVO: servidor.cpp
// PROPÓSITO: Servidor de parqueadero que gestiona 40 plazas mediante sockets
// DESCRIPCIÓN: Recibe conexiones de clientes, procesa entradas/salidas de
//              vehículos y mantiene el estado del parqueadero
// ============================================================================

#include <iostream>      // Para cout, cerr
#include <WinSock2.h>    // Librería de sockets de Windows
#include <WS2tcpip.h>    // Funciones adicionales de TCP/IP
#include <string.h>      // Para strchr, strcmp, strlen
#include <cctype>        // Para isalpha, isdigit

// Vincular la librería de sockets de Windows
#pragma comment(lib, "ws2_32.lib")

// CONSTANTES DEL SISTEMA
// ----------------------
#define PORT 8080        // Puerto donde el servidor escucha conexiones
#define NUM_SPOTS 40     // Número total de plazas de parqueadero

using namespace std;

// ============================================================================
// FUNCIÓN: printParkingStatus
// PROPÓSITO: Muestra en consola el estado actual de todas las plazas
// PARÁMETROS:
//   - spots: Arreglo de punteros a char* (cada plaza apunta a una placa o nullptr)
//   - numSpots: Número total de plazas (40)
// ============================================================================
void printParkingStatus(char** spots, int numSpots)
{
	cout << "\n---[ ESTADO DEL PARKING ]---\n";
	
	// ITERAR POR TODAS LAS PLAZAS
	// ----------------------------
	for (int i = 0; i < numSpots; i++)
	{
		// Mostrar número de plaza (1-40, no 0-39)
		cout << " Plaza " << (i + 1) << ": ";
		
		// VERIFICAR SI LA PLAZA ESTÁ VACÍA
		// ---------------------------------
		// nullptr = puntero nulo = plaza vacía
		if (spots[i] == nullptr)
		{
			cout << "[ VACIO ]";
		}
		else
		{
			// Plaza ocupada: mostrar la placa
			cout << "[ " << spots[i] << " ]";
		}
		cout << endl;
	}
	cout << "----------------------------------\n\n";
}

// ============================================================================
// FUNCIÓN: isValidPlate
// PROPÓSITO: Valida que una placa tenga el formato correcto AAA000
// PARÁMETROS:
//   - plate: Puntero a string con la placa a validar
// RETORNA: true si es válida, false si no
// REGLAS:
//   - Debe tener exactamente 6 caracteres
//   - Primeros 3 caracteres: LETRAS (A-Z)
//   - Últimos 3 caracteres: NÚMEROS (0-9)
// ============================================================================
bool isValidPlate(const char* plate)
{
	// VALIDAR LONGITUD
	// ----------------
	// strlen() = "string length" = longitud del string
	if (strlen(plate) != 6)
	{
		cout << "⚠ La placa debe tener 6 caracteres.\n";
		return false;
	}
	
	// VALIDAR PRIMEROS 3 CARACTERES (LETRAS)
	// ---------------------------------------
	for (int i = 0; i < 3; ++i)
	{
		// isalpha() = "is alphabetic" = verifica si es letra
		if (!isalpha(plate[i]))
		{
			return false;  // No es letra → placa inválida
		}
	}
	
	// VALIDAR ÚLTIMOS 3 CARACTERES (NÚMEROS)
	// ---------------------------------------
	for (int i = 3; i < 6; ++i)
	{
		// isdigit() = "is digit" = verifica si es dígito (0-9)
		if (!isdigit(plate[i]))
		{
			return false;  // No es número → placa inválida
		}
	}
	
	// Si pasó todas las validaciones, la placa es válida
	return true;
}

// ============================================================================
// FUNCIÓN: findPlate
// PROPÓSITO: Busca una placa en el parqueadero
// PARÁMETROS:
//   - plate: Placa a buscar (ej: "ABC123")
//   - spots: Arreglo de plazas
//   - numSpots: Número total de plazas
// RETORNA: Índice de la plaza (0-39) si se encuentra, -1 si no existe
// ============================================================================
int findPlate(const char* plate, char** spots, int numSpots)
{
	// ITERAR POR TODAS LAS PLAZAS
	// ----------------------------
	for (int i = 0; i < numSpots; ++i)
	{
		// VERIFICAR SI LA PLAZA TIENE UNA PLACA
		// --------------------------------------
		// spots[i] != nullptr → plaza ocupada
		// strcmp(a, b) == 0 → los strings son iguales
		if (spots[i] != nullptr && strcmp(spots[i], plate) == 0)
		{
			return i;  // Encontrada: retornar el índice
		}
	}
	// No encontrada en ninguna plaza
	return -1;
}

// ============================================================================
// FUNCIÓN PRINCIPAL: main
// PROPÓSITO: Inicializa el servidor, escucha conexiones y procesa mensajes
// ============================================================================
int main()
{
	// VARIABLES PARA SOCKETS
	// ----------------------
	WSADATA wsaData;              // Información de Winsock
	SOCKET servidor_fd;           // Socket del servidor (escucha conexiones)
	SOCKET nuevo_socket;          // Socket del cliente conectado
	struct sockaddr_in direccion; // Estructura con la dirección del servidor
	int addrlen = sizeof(direccion); // Tamaño de la estructura

	// ========================================================================
	// INICIALIZAR ESTRUCTURA DE DATOS DEL PARQUEADERO
	// ========================================================================
	// Crear un arreglo dinámico de 40 punteros a char*
	// char** = puntero a puntero = arreglo de strings
	// Cada posición del arreglo representa una plaza:
	//   - nullptr = plaza vacía
	//   - puntero a string = plaza ocupada (la string es la placa)
	char** parkingSpots = new char* [NUM_SPOTS];
	
	// INICIALIZAR TODAS LAS PLAZAS COMO VACÍAS
	// -----------------------------------------
	for (int i = 0; i < NUM_SPOTS; ++i)
	{
		parkingSpots[i] = nullptr;  // Todas las plazas comienzan vacías
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cerr << "Fall� la inicializaci�n del winsock. C�digo de error: " << WSAGetLastError() << endl;
		return 1;
	}

	servidor_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (servidor_fd == INVALID_SOCKET)
	{
		cerr << "Fallo en el socket. C�digo de error: " << WSAGetLastError() << endl;
		WSACleanup();
		return 1;
	}

	direccion.sin_family = AF_INET;
	direccion.sin_addr.s_addr = INADDR_ANY;
	direccion.sin_port = htons(PORT);

	if (bind(servidor_fd, (struct sockaddr*)&direccion, sizeof(direccion)) == SOCKET_ERROR)
	{
		cerr << "Fallo en bind. C�digo de error: " << WSAGetLastError() << endl;
		closesocket(servidor_fd);
		WSACleanup();
		return 1;
	}

	if (listen(servidor_fd, 3) == SOCKET_ERROR)
	{
		cerr << "Fallo el listen. C�digo de error: " << WSAGetLastError() << endl;
		closesocket(servidor_fd);
		WSACleanup();
		return 1;
	}

	cout << "Servidor del parqueadero iniciado. Esperando conexi�n en el puerto " << PORT << "...\n";

	while (true)
	{
		nuevo_socket = accept(servidor_fd, (struct sockaddr*)&direccion, &addrlen);
		if (nuevo_socket == INVALID_SOCKET)
		{
			cerr << "Fallo el accept. C�digop de error: " << WSAGetLastError() << endl;
			continue;
		}

		cout << "\n�Cliente conectado! Enviando estado inicial.\n";
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
			cerr << "Fallo en recv. C�digo de error: " << WSAGetLastError() << endl;
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
