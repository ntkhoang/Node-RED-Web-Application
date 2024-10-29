# ESP32 HTTP Server Example

ESP32 web server example: control 2 LEDs from a web page hosted on the ESP32.

Use [Wokwi for Visual Studio Code](https://marketplace.visualstudio.com/items?itemName=wokwi.wokwi-vscode) to simulate this project.

## Building

This is a [PlatformIO](https://platformio.org) project. To build it, [install PlatformIO](https://docs.platformio.org/en/latest/core/installation/index.html), and then run the following command:

```
pio run
```

## Simulating

To simulate this project, install [Wokwi for VS Code](https://marketplace.visualstudio.com/items?itemName=wokwi.wokwi-vscode). Open the project directory in Visual Studio Code, press **F1** and select "Wokwi: Start Simulator".

Once the simulation is running, open http://localhost:8180 in your web browser to interact with the simulated HTTP server.

## Node-red Web

To use the web site, install Node-red locally you will need a supported version of Node.js

To install Node-RED you can use the npm command that comes with node.js:

```
sudo npm install -g --unsafe-perm node-red
```

Then import the flow1.json and flow2.json into Node-red and deploy to use the web site
