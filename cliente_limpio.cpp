#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <cstdlib>
#include <ctime>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080

using namespace std;

// Genera placa aleatoria formato AAA000
string generateRandomPlate() {
	string plate = "";
	for (int i = 0; i < 3; i++) {
		plate += (char)('A' + rand() % 26);
	}
	for (int i = 0; i < 3; i++) {
		plate += (char)('0' + rand() % 10);
	}
	return plate;
}

// Genera número de plaza aleatorio entre 1-40
int generateRandomSpot() {
	return (rand() % 40) + 1;
}

int main()
{
	srand(static_cast<unsigned int>(time(0)));
	
	WSADATA wsaData;
	SOCKET sock = INVALID_SOCKET;
	struct sockaddr_in serv_addr;
	char buffer[1024] = { 0 };

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cerr << "Fallo al inicializar el winsock. Código de error: " << WSAGetLastError() << endl;
		return 1;
	}

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		cerr << "Fallo el socket. Código de error: " << WSAGetLastError() << endl;
		WSACleanup();
		return 1;
	}
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	int result = inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
	if (result <= 0)
	{
		cerr << "Direccion IPv4 inválida o no soportada." << endl;
		closesocket(sock);
		WSACleanup();
		return 1;
	}

	if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR)
	{
		cerr << "Fallo en connect. Código de error: " << WSAGetLastError() << endl;
		closesocket(sock);
		WSACleanup();
		return 1;
	}

	cout << "\n[OK] Conectado al servidor en puerto " << PORT << "\n";
	cout << "[*] Generando placas automaticamente cada 2-5 segundos...\n\n";

	while (true)
	{
		int spotNum = generateRandomSpot();
		string plate = generateRandomPlate();

		// Obtener timestamp actual
		time_t now = time(0);
		struct tm timeinfo;
		localtime_s(&timeinfo, &now);
		char timestamp[30];
		strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);

		// Formato: "PLAZA:PLACA:TIMESTAMP"
		string message = to_string(spotNum) + ":" + plate + ":" + string(timestamp);

		send(sock, message.c_str(), static_cast<int>(message.length()), 0);
		cout << ">> [" << spotNum << "] " << plate << " | " << timestamp << endl;

		int valread = recv(sock, buffer, 1024, 0);
		if (valread > 0)
		{
			if (valread < 1024)
			{
				buffer[valread] = '\0';
			}
			else
			{
				buffer[1023] = '\0';
			}
			cout << "<< " << buffer << "\n";

		}
		else if (valread == 0)
		{
			cout << "[!] El servidor cerro la conexion." << endl;
			break;
		}
		else
		{
			cerr << "[ERROR] Fallo en recv. Codigo de error: " << WSAGetLastError() << endl;
			break;
		}

		int waitTime = 2000 + (rand() % 3001);
		cout << "** Esperando " << (waitTime / 1000.0) << "s...\n\n";
		Sleep(waitTime);
	}

	closesocket(sock);
	WSACleanup();
	return 0;
}
