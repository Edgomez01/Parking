#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <istream>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080

using namespace std;

int main()
{
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
		cerr << "Fallo el socked. Código de error: " << WSAGetLastError() << endl;
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

	cout << "¡Conectado al servidor del parqueadero! \n";

	while (true)
	{
		int spotNum = 0;
		string plate;

		while (true)
		{
			cout << "\nIngrese el número de plaza (1-40): ";
			if (cin >> spotNum)
			{
				if (spotNum >= 1 && spotNum <= 40)
				{
					break;
				}
				else
				{
					cout << "ERROR: Por favor ingrese un número entre 1 y 40" << endl;
				}
			}
			else
			{
				cout << "ERROR: Entrada no valida. Intente de nuevo. " << endl;
				cin.clear();
				string dummy;
				getline(cin, dummy);
			}
		}

		cout << "Ingrese la placa (ej: ABC123): ";
		cin >> plate;

		string message = to_string(spotNum) + ":" + plate;

		send(sock, message.c_str(), static_cast<int>(message.length()), 0);
		cout << "-> Enviando: \"" << message << "\"" << endl;

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
			cout << "Respuesta del servidor: " << buffer << endl;

		}
		else if (valread == 0)
		{
			cout << "El servidor cerró la conexión." << endl;
			break;
		}
		else
		{
			cerr << "Fallo en recv. Código de error: " << WSAGetLastError() << endl;
			break;
		}

		cout << "(Esperando 2 segundos...)" << endl;
		Sleep(2000);

	}

	closesocket(sock);
	WSACleanup();
	return 0;
}
