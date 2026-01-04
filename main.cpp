#include <iostream>
#include <string>

// Pour Windows
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
// Pour Linux/Mac
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#endif

class SimpleServer {
private:
#ifdef _WIN32
    SOCKET serverSocket;
    SOCKET clientSocket;
    WSADATA wsaData;
#else
    int serverSocket;
    int clientSocket;
#endif
    
    int port;
    
public:
    SimpleServer(int port = 8080) : port(port) {
#ifdef _WIN32
        serverSocket = INVALID_SOCKET;
        clientSocket = INVALID_SOCKET;
#else
        serverSocket = -1;
        clientSocket = -1;
#endif
    }
    
    bool initialize() {
#ifdef _WIN32
        // Initialiser Winsock sur Windows
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed" << std::endl;
            return false;
        }
#endif
        
        // Créer le socket
#ifdef _WIN32
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == INVALID_SOCKET) {
            std::cerr << "Socket creation failed" << std::endl;
            return false;
        }
#else
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket < 0) {
            std::cerr << "Socket creation failed" << std::endl;
            return false;
        }
#endif
        
        // Configurer les options du socket
        int opt = 1;
#ifdef _WIN32
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
#else
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif
        
        // Configurer l'adresse
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);
        
        // Binder le socket
        if (bind(serverSocket, (struct sockaddr*)&address, sizeof(address)) < 0) {
            std::cerr << "Bind failed" << std::endl;
            return false;
        }
        
        // Écouter
        if (listen(serverSocket, 3) < 0) {
            std::cerr << "Listen failed" << std::endl;
            return false;
        }
        
        std::cout << "Server listening on port " << port << std::endl;
        return true;
    }
    
    void start() {
        struct sockaddr_in clientAddress;
#ifdef _WIN32
        int clientAddrLen = sizeof(clientAddress);
#else
        socklen_t clientAddrLen = sizeof(clientAddress);
#endif
        
        while (true) {
            std::cout << "Waiting for connection..." << std::endl;
            
#ifdef _WIN32
            clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddrLen);
            if (clientSocket == INVALID_SOCKET) {
                std::cerr << "Accept failed" << std::endl;
                continue;
            }
#else
            clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddrLen);
            if (clientSocket < 0) {
                std::cerr << "Accept failed" << std::endl;
                continue;
            }
#endif
            
            std::cout << "Client connected!" << std::endl;
            
            // Lire la requête (simplifié)
            char buffer[1024] = {0};
#ifdef _WIN32
            recv(clientSocket, buffer, sizeof(buffer), 0);
#else
            read(clientSocket, buffer, sizeof(buffer));
#endif
            
            // Réponse HTTP simple
            std::string message = "Hello World modifiedddd";
            std::string response = 
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/plain\r\n"
                "Access-Control-Allow-Origin: *\r\n"
                "Content-Length: " + std::to_string(message.length()) + "\r\n"
                "\r\n" + 
                message;
            
            // Envoyer la réponse
#ifdef _WIN32
            send(clientSocket, response.c_str(), response.length(), 0);
            closesocket(clientSocket);
#else
            send(clientSocket, response.c_str(), response.length(), 0);
            close(clientSocket);
#endif
            
            std::cout << "Response sent: Hello World" << std::endl;
        }
    }
    
    ~SimpleServer() {
#ifdef _WIN32
        if (serverSocket != INVALID_SOCKET) {
            closesocket(serverSocket);
        }
        WSACleanup();
#else
        if (serverSocket >= 0) {
            close(serverSocket);
        }
#endif
    }
};

int main() {
    SimpleServer server(8080);
    
    if (server.initialize()) {
        server.start();
    } else {
        std::cerr << "Server initialization failed!" << std::endl;
        return 1;
    }
    
    return 0;
}