# NodeTruck

NodeTruck is a project aimed at controlling and monitoring a kids Ride-on car via a microcontroller and a web interface. The project consists of three main parts: the **Kids Ride-on car**, a **Microcontroller Unit (MCU)** and a **Web Application**.

## Components

### Kids Ride-on car:
- A Kids Ride-on car with a remote simular to this [this type](https://www.aliexpress.com/item/1005007166105465.html).
- Other remotes will probably work just as well, but then soldering might differ

### NodeTruck MCU:
- Microcontroller setup using the WeMos board (ESP8266).
- Store credentials and configuration in `secrets.h`.
- **Disclaimer:** I have zero expertise in electronics or soldering. The [soldering diagram](https://github.com/martinpartin/NodeTruck/blob/6b665cc0c752c294dc6a638f0e604b1d3ef2b887/docs/circuit.drawio.png) might be highly flammable—proceed with caution, and have a fire extinguisher ready.

### NodeTruck WebApp:
- Configure secrets using `dotnet user-secrets` with the values from `secrets.h`.
- Run the web app using `dotnet run`.
- Requires an MQTT server for communication. I recommend using [HiveMQ](https://www.hivemq.com/)—easy setup, less fire.
- Aim for stable and crash-free operation.

## Prerequisites
- **Car**: You need a kids Ride-on car w/remote control (or something similar).
- **MCU**: ESP8266 (WeMos) or ESP32
- **WebApp**: ASP.NET Core and .NET 5+.
- **MQTT Server**: Set up an MQTT server, such as [HiveMQ](https://www.hivemq.com/), to enable communication between the web app and the MCU.

## Installation and Setup

### 1. Microcontroller Setup
- Upload the `secrets.h` file with necessary credentials.
- Flash the firmware onto the WeMos board.
- Configure [WifiManager](https://github.com/tzapu/WiFiManager) ("AutoConnectAP", "password")

### 2. Web Application Setup
- Configure the app using `dotnet user-secrets`.
  ```bash
  dotnet user-secrets set "Broker" "from secrets.h.. hivemq.cloud"
  dotnet user-secrets set "Username" "mqtt username from secrets.h.."
  dotnet user-secrets set "Key" "mqtt password from secrets.h.."
