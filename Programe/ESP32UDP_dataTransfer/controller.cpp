#include <winsock2.h>
#include <windows.h>
#include <string>
#include <map>
#include <iostream>
#include <thread>
#include <ws2tcpip.h>
#include <shellapi.h>  // Required for ShellExecute

#pragma comment(lib, "ws2_32.lib")

const char* TCP_IP = "192.168.4.1";
const int TCP_PORT = 4210;
const char* STREAM_URL = "http://192.168.4.3";
SOCKET sock;
HBRUSH hbrBkgnd = CreateSolidBrush(RGB(255, 255, 255));

std::map<char, HWND> key_labels;
HWND sensor_labels[4];
bool is_connected = false;

void open_stream_url(const char* url) {
    ShellExecute(0, "open", url, 0, 0, SW_SHOWNORMAL);
}

void connect_to_server() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return;
    }
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(TCP_PORT);
    InetPton(AF_INET, TCP_IP, &server_addr.sin_addr);
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Error connecting to " << TCP_IP << ":" << TCP_PORT << std::endl;
        is_connected = false;
        closesocket(sock);
        WSACleanup();
        return;
    }
    std::cout << "Connected to " << TCP_IP << " on port " << TCP_PORT << std::endl;
    is_connected = true;
}

void send_command(const std::string& command) {
    if (is_connected) {
        int result = send(sock, command.c_str(), command.length(), 0);
        if (result == SOCKET_ERROR) {
            std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
        }
    } else {
        std::cerr << "Not connected to server." << std::endl;
    }
}

void update_sensor_labels(const std::string& message) {
    size_t start = 0, end;
    for (int i = 0; i < 4; i++) {
        end = message.find(';', start);
        std::string value = message.substr(start, end - start);
        SetWindowText(sensor_labels[i], value.c_str());
        start = end + 1;
    }
}

void listen_for_messages(HWND hwnd) {
    char buffer[1024];
    while (is_connected) {
        int bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            std::string message = buffer;
            update_sensor_labels(message);
        } else if (bytes_received == 0) {
            std::cerr << "Connection closed by server." << std::endl;
            is_connected = false;
        } else {
            std::cerr << "Recv failed: " << WSAGetLastError() << std::endl;
            is_connected = false;
        }
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_KEYDOWN: {
            char key = toupper(static_cast<char>(wParam));
            std::cerr << "Key down: " << key << std::endl;
            if (key == 'W') key = 'F';
            else if (key == 'A') key = 'L';
            else if (key == 'S') key = 'B';
            else if (key == 'D') key = 'R';
            send_command(std::string(1, key) + "\n");
            if (key_labels.find(key) != key_labels.end()) {
                SetClassLongPtr(key_labels[key], GCLP_HBRBACKGROUND, reinterpret_cast<LONG_PTR>(CreateSolidBrush(RGB(150, 150, 150))));
                InvalidateRect(key_labels[key], NULL, TRUE);
            }
        } break;
        case WM_KEYUP: {
            send_command("P\n");
            char key = toupper(static_cast<char>(wParam));
            std::cerr << "Key up: " << key << std::endl;
            if (key_labels.find(key) != key_labels.end()) {
                SetClassLongPtr(key_labels[key], GCLP_HBRBACKGROUND, reinterpret_cast<LONG_PTR>(CreateSolidBrush(RGB(255, 255, 255))));
                InvalidateRect(key_labels[key], NULL, TRUE);
            }
        } break;
        case WM_DESTROY:
            is_connected = false;
            closesocket(sock);
            WSACleanup();
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "WASD Keyboard Controller";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    open_stream_url(STREAM_URL);

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, "WASD Keyboard Controller", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, hInstance, NULL);
    if (hwnd == NULL) {
        std::cerr << "CreateWindowEx failed." << std::endl;
        return 0;
    }
    ShowWindow(hwnd, nCmdShow);

    key_labels['W'] = CreateWindow("STATIC", "W", WS_CHILD | WS_VISIBLE | SS_CENTER, 85, 50, 60, 60, hwnd, NULL, hInstance, NULL);
    key_labels['A'] = CreateWindow("STATIC", "A", WS_CHILD | WS_VISIBLE | SS_CENTER, 20, 120, 60, 60, hwnd, NULL, hInstance, NULL);
    key_labels['S'] = CreateWindow("STATIC", "S", WS_CHILD | WS_VISIBLE | SS_CENTER, 85, 120, 60, 60, hwnd, NULL, hInstance, NULL);
    key_labels['D'] = CreateWindow("STATIC", "D", WS_CHILD | WS_VISIBLE | SS_CENTER, 150, 120, 60, 60, hwnd, NULL, hInstance, NULL);

    connect_to_server();
    if (is_connected) {
        std::thread listener_thread(listen_for_messages, hwnd);
        listener_thread.detach();
    } else {
        std::cerr << "Failed to connect to server." << std::endl;
    }

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
