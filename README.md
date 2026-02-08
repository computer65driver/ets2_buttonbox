# Simple Wireless Button Box (ESP32)

A hobby project for **Euro Truck Simulator 2**. It sends button presses from an ESP32 to a Windows PC over WiFi (UDP).

### How it works:
1. **ESP32** reads 4 buttons/switches.
2. It sends a small data packet over the local network via **UDP**.
3. A **C++ Receiver App** on the PC catches the packet and presses the corresponding key (E, H, Space, F).

### Project Structure:
* `/ESP32_Code`: Contains the ESP-IDF source code.
* `/PC_Receiver`: Contains the C++ WinSock receiver code.

### Quick Setup:
1. Update the **WiFi SSID, Password, and PC IP Address** in the ESP32 code.
2. Flash the ESP32 using ESP-IDF.
3. Update the **IP Address** in the PC Receiver code.
4. Compile and run the Receiver on your Windows PC.
5. Start the game and enjoy!

*Note: This is an experimental student project.*
