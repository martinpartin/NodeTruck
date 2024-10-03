# NodeTruck

NodeTruck is a project aimed at controlling and monitoring a small truck via a microcontroller and a web interface. The project consists of two main parts: the **Microcontroller Unit (MCU)** and a **Web Application**.

## Components

### NodeTruck MCU:
- Microcontroller setup using the WeMos board (ESP8266).
- Store credentials and configuration in `secrets.h`.
- Requires soldering and connecting the hardware components. Follow the [soldering diagram](https://github.com/martinpartin/NodeTruck/blob/6b665cc0c752c294dc6a638f0e604b1d3ef2b887/docs/circuit.drawio.png) for detailed instructions.

### NodeTruck WebApp:
- Configure secrets using `dotnet user-secrets` with the values from `secrets.h`.
- Run the web app using `dotnet run`.
- Aim for stable and crash-free operation.

## Prerequisites

- **MCU**: ESP8266 (WeMos) with appropriate sensors and motors.
- **WebApp**: ASP.NET Core and .NET 5+.

## Installation and Setup

### 1. Microcontroller Setup
- Upload the `secrets.h` file with necessary credentials.
- Flash the firmware onto the WeMos board.

### 2. Web Application Setup
- Configure the app using `dotnet user-secrets`.
  ```bash
  dotnet user-secrets set "Key" "Value"
