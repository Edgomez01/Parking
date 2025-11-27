// ============================================================================
// ARCHIVO: cliente.cpp
// PROPÓSITO: Cliente que genera placas automáticamente y las envía al servidor
// DESCRIPCIÓN: Conecta al servidor de parqueadero y envía placas aleatorias
//              cada 2-5 segundos automáticamente
// ============================================================================

#include <iostream>
#include <WinSock2.h>   // Librería de sockets de Windows
#include <WS2tcpip.h>   // Funciones adicionales de TCP/IP
#include <string>       // Para usar std::string
#include <cstdlib>      // Para rand() y srand()
#include <ctime>        // Para time() (semilla aleatoria)

// Vincular la librería de sockets de Windows
#pragma comment(lib, "ws2_32.lib")

// Definir el puerto del servidor (debe coincidir con servidor.cpp)
#define PORT 8080

using namespace std;

// ============================================================================
// FUNCIÓN: generateRandomPlate
// PROPÓSITO: Genera una placa aleatoria con formato AAA000
// RETORNA: String con 3 letras mayúsculas + 3 números (ej: "XYZ789")
// ============================================================================
string generateRandomPlate() {
	string plate = "";
	
	// GENERAR 3 LETRAS ALEATORIAS (A-Z)
	// ----------------------------------
	// 'A' tiene código ASCII 65
	// rand() % 26 genera números entre 0 y 25
	// 'A' + 0 = 'A', 'A' + 1 = 'B', ... 'A' + 25 = 'Z'
	for (int i = 0; i < 3; i++) {
		plate += (char)('A' + rand() % 26);
	}
	
	// GENERAR 3 NÚMEROS ALEATORIOS (0-9)
	// -----------------------------------
	// '0' tiene código ASCII 48
	// rand() % 10 genera números entre 0 y 9
	// '0' + 0 = '0', '0' + 1 = '1', ... '0' + 9 = '9'
	for (int i = 0; i < 3; i++) {
		plate += (char)('0' + rand() % 10);
	}
	
	return plate;
}

// ============================================================================
// FUNCIÓN: generateRandomSpot
// PROPÓSITO: Genera un número de plaza aleatorio entre 1 y 40
// RETORNA: Entero entre 1 y 40
// ============================================================================
int generateRandomSpot() {
	// rand() % 40 genera 0-39, sumamos 1 para obtener 1-40
	return (rand() % 40) + 1;
}

int main()
{
	// INICIALIZAR GENERADOR DE NÚMEROS ALEATORIOS
	// --------------------------------------------
	// srand() establece la "semilla" del generador aleatorio
	// time(0) = segundos desde 1970 (siempre diferente)
	// Sin esto, rand() generaría siempre los mismos números
	srand(static_cast<unsigned int>(time(0)));
	
	// VARIABLES PARA SOCKETS
	// ----------------------
	WSADATA wsaData;               // Información de Winsock
	SOCKET sock = INVALID_SOCKET;  // Socket del cliente
	struct sockaddr_in serv_addr;  // Dirección del servidor
	char buffer[1024] = { 0 };     // Buffer para recibir respuestas

	// ========================================================================
	// PASO 1: INICIALIZAR WINSOCK
	// ========================================================================
	// WSAStartup inicializa la librería de sockets de Windows
	// MAKEWORD(2, 2) = versión 2.2 de Winsock
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cerr << "Fallo al inicializar el winsock. Código de error: " << WSAGetLastError() << endl;
		return 1;
	}

	// ========================================================================
	// PASO 2: CREAR SOCKET
	// ========================================================================
	// socket() crea un nuevo socket
	// AF_INET = IPv4
	// SOCK_STREAM = TCP (conexión confiable)
	// 0 = protocolo automático
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		cerr << "Fallo el socket. Código de error: " << WSAGetLastError() << endl;
		WSACleanup();
		return 1;
	}

	// ========================================================================
	// PASO 3: CONFIGURAR DIRECCIÓN DEL SERVIDOR
	// ========================================================================
	serv_addr.sin_family = AF_INET;      // IPv4
	serv_addr.sin_port = htons(PORT);    // Puerto 8080 (htons = host to network short)

	// CONVERTIR DIRECCIÓN IP DE TEXTO A BINARIO
	// ------------------------------------------
	// inet_pton = "Internet presentation to numeric"
	// Convierte "127.0.0.1" (texto) a formato binario
	int result = inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
	if (result <= 0)
	{
		cerr << "Direccion IPv4 inválida o no soportada." << endl;
		closesocket(sock);
		WSACleanup();
		return 1;
	}

	// ========================================================================
	// PASO 4: CONECTAR AL SERVIDOR
	// ========================================================================
	// connect() establece la conexión con el servidor
	// Si el servidor no está ejecutándose, esto fallará
	if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR)
	{
		cerr << "Fallo en connect. Código de error: " << WSAGetLastError() << endl;
		closesocket(sock);
		WSACleanup();
		return 1;
	}

	cout << "\n";
	cout << "============================================\n";
	cout << "  CLIENTE - GENERADOR AUTOMATICO DE PLACAS\n";
	cout << "============================================\n";
	cout << "[OK] Conectado al servidor en puerto " << PORT << "\n";
	cout << "[*] Generando placas automaticamente cada 2-5 segundos...\n";
	cout << "[*] Formato: AAA000 (3 letras + 3 numeros)\n";
	cout << "[*] Plaza aleatoria entre 1 y 40\n";
	cout << "\n";

	// ========================================================================
	// BUCLE PRINCIPAL: GENERAR Y ENVIAR PLACAS AUTOMÁTICAMENTE
	// ========================================================================
	// Este bucle se ejecuta indefinidamente hasta que se cierre el programa
	while (true)
	{
		// GENERAR DATOS ALEATORIOS
		// ------------------------
		// Generar número de plaza aleatorio (1-40)
		int spotNum = generateRandomSpot();
		
		// Generar placa aleatoria (AAA000)
		string plate = generateRandomPlate();

		// OBTENER TIMESTAMP ACTUAL
		// -------------------------
		// time_t = tipo de dato para tiempo
		// time(0) = obtiene el tiempo actual
		time_t now = time(0);
		
		// Convertir a estructura tm para formatear
		struct tm timeinfo;
		localtime_s(&timeinfo, &now);
		
		// Formatear como "YYYY-MM-DD HH:MM:SS"
		char timestamp[30];
		strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);

		// CONSTRUIR MENSAJE EN FORMATO "PLAZA:PLACA:TIMESTAMP"
		// -----------------------------------------------------
		// Ejemplo: "15:XYZ789:2024-11-25 14:30:45"
		// Incluye: número de plaza, placa del vehículo y hora exacta
		string message = to_string(spotNum) + ":" + plate + ":" + string(timestamp);

		// ENVIAR MENSAJE AL SERVIDOR
		// ---------------------------
		// send() envía datos por el socket
		// message.c_str() convierte string a char* (C-style string)
		// message.length() = número de bytes a enviar
		send(sock, message.c_str(), static_cast<int>(message.length()), 0);
		
		// Mostrar lo que se envió (con timestamp)
		cout << ">> [" << spotNum << "] Enviando:\n";
		cout << "   Plaza: " << spotNum << "\n";
		cout << "   Placa: " << plate << "\n";
		cout << "   Hora: " << timestamp << endl;

		// RECIBIR RESPUESTA DEL SERVIDOR
		// -------------------------------
		// recv() espera y recibe datos del servidor
		// buffer = donde se guardan los datos recibidos
		// 1024 = tamaño máximo a recibir
		// 0 = flags (ninguno)
		int valread = recv(sock, buffer, 1024, 0);
		
		// PROCESAR LA RESPUESTA
		// ---------------------
		if (valread > 0)
		{
			// valread = número de bytes recibidos
			
			// AGREGAR TERMINADOR NULO AL FINAL
			// ---------------------------------
			// Los strings en C++ necesitan '\0' al final
			if (valread < 1024)
			{
				buffer[valread] = '\0';  // Terminar el string
			}
			else
			{
				buffer[1023] = '\0';  // Si el buffer está lleno, terminar en la última posición
			}
			
			// Mostrar la respuesta del servidor
			cout << "<< Respuesta: " << buffer << endl;
			cout << "----------------------------------------\n";

		}
		else if (valread == 0)
		{
			// valread == 0 significa que el servidor cerró la conexión
			cout << "[!] El servidor cerro la conexion." << endl;
			break;  // Salir del bucle
		}
		else
		{
			// valread < 0 significa error
			cerr << "[ERROR] Fallo en recv. Codigo de error: " << WSAGetLastError() << endl;
			break;  // Salir del bucle
		}

		// ESPERAR TIEMPO ALEATORIO ANTES DE LA SIGUIENTE PLACA
		// -----------------------------------------------------
		// rand() % 3000 = número entre 0 y 2999 milisegundos
		// 2000 + rand() % 3000 = entre 2000 y 4999 milisegundos
		// Es decir: entre 2 y 5 segundos
		int waitTime = 2000 + (rand() % 3001);  // 2000-5000 ms
		cout << "** Esperando " << (waitTime / 1000.0) << " segundos...\n\n";
		Sleep(waitTime);

	}

	// ========================================================================
	// LIMPIEZA Y CIERRE
	// ========================================================================
	// Cuando se sale del bucle, cerrar el socket y limpiar Winsock
	cout << "\nCerrando conexión...\n";
	closesocket(sock);
	WSACleanup();
	return 0;
}
