#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <cstdint>

#pragma comment(lib, "ws2_32.lib")

#pragma pack(push, 1)
typedef struct
{
    uint8_t engine_state;
    uint8_t horn_state;
    uint8_t toggle_state;
    uint8_t light_state;
} button_packet_t;
#pragma pack(pop)

void SendKeystroke(WORD key)
{
    INPUT inputs[2] = {};

    UINT scanCode = MapVirtualKey(key, MAPVK_VK_TO_VSC);

    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = 0;
    inputs[0].ki.wScan = scanCode;
    inputs[0].ki.dwFlags = KEYEVENTF_SCANCODE;

    SendInput(1, &inputs[0], sizeof(INPUT));

    Sleep(100);

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = 0;
    inputs[1].ki.wScan = scanCode;
    inputs[1].ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;

    SendInput(1, &inputs[1], sizeof(INPUT));
}

int main()
{
    WSADATA wsaData;
    SOCKET recvSocket;
    struct sockaddr_in server, client;
    int slen = sizeof(client);
    char buf[512];

    WSAStartup(MAKEWORD(2, 2), &wsaData);

    recvSocket = socket(AF_INET, SOCK_DGRAM, 0);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("IP_ADDRESS"); // Check the IP Address.
    server.sin_port = htons(1234);

    bind(recvSocket, (struct sockaddr *)&server, sizeof(server));

    uint8_t last_engine = 0, last_horn = 0, last_toggle = 0;
    uint8_t last_light = 0;

    std::cout << "Receiver is ready" << std::endl;

    while (true)
    {
        int recv_len = recvfrom(recvSocket, buf, sizeof(button_packet_t), 0, (struct sockaddr *)&client, &slen);

        if (recv_len == sizeof(button_packet_t))
        {
            button_packet_t *packet = (button_packet_t *)buf;

            if (packet->engine_state != last_engine)
            {
                if (packet->engine_state == 1 || packet->engine_state == 0)
                {
                    // std::cout << "Ignition -> E" << std::endl;
                    SendKeystroke(0x45);
                }
                last_engine = packet->engine_state;
            }

            if (packet->horn_state != last_horn)
            {
                if (packet->horn_state == 1)
                {
                    // std::cout << "Horn -> H" << std::endl;
                    SendKeystroke(0x48);
                }
                last_horn = packet->horn_state;
            }

            if (packet->toggle_state != last_toggle)
            {
                // std::cout << "Handbrake -> SPACE" << std::endl;
                SendKeystroke(VK_SPACE);
                last_toggle = packet->toggle_state;
            }

            if (packet->light_state != last_light)
            {
                // std::cout << "Hazard Lights -> F" << std::endl;
                SendKeystroke(0x46);
                last_light = packet->light_state;
            }
        }
    }

    closesocket(recvSocket);
    WSACleanup();
    return 0;
}