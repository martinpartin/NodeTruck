# NodeTruck

NodeTruck is a project aimed at controlling and monitoring a kids Ride-on car via a microcontroller and a web interface. The project consists of two main parts: the **Microcontroller Unit (MCU)** and a **Web Application**.

## Components

### NodeTruck MCU:
- Microcontroller setup using the WeMos board (ESP8266).
- Store credentials and configuration in `secrets.h`.
- The system is designed to control a kids Kids Ride-on car (yes, you need an actual car for this!). You can pair it with a remote control of [this type](https://www.aliexpress.com/item/1005007166105465.html).
- **Disclaimer:** I have zero expertise in electronics or soldering. The [soldering diagram](https://github.com/martinpartin/NodeTruck/blob/6b665cc0c752c294dc6a638f0e604b1d3ef2b887/docs/circuit.drawio.png) might be highly flammable—proceed with caution, and have a fire extinguisher ready.

### NodeTruck WebApp:
- Configure secrets using `dotnet user-secrets` with the values from `secrets.h`.
- Run the web app using `dotnet run`.
- Requires an MQTT server for communication. I recommend using [HiveMQ](https://www.hivemq.com/)—easy setup, less fire.
- Aim for stable and crash-free operation.

## Prerequisites

- **MCU**: ESP8266 (WeMos) with appropriate sensors and motors.
- **Car**: You need a small RC car (or something similar).
- **Remote Control**: A remote like [this one](https://www.aliexpress.com/item/1005007166105465.html).
- **WebApp**: ASP.NET Core and .NET 5+.
- **MQTT Server**: Set up an MQTT server, such as [HiveMQ](https://www.hivemq.com/), to enable communication between the web app and the MCU.

## Installation and Setup

### 1. Microcontroller Setup
- Upload the `secrets.h` file with necessary credentials.
- Flash the firmware onto the WeMos board.

### 2. Web Application Setup
- Configure the app using `dotnet user-secrets`.
  ```bash
  dotnet user-secrets set "Key" "Value"
