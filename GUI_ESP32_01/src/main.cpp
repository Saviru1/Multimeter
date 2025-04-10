#include <WiFi.h>
#include <WebServer.h>
#include <Arduino.h>

// Network 
const char* ssid = "ESP32-Access-Point";
const char* password = "12345678";

WebServer server(80);

String result; 

// HTML
const char* htmlContent = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Multimeter using STM32</title>
  <style>
     body { 
       font-family: Arial, sans-serif; 
       background-color:rgb(0, 0, 0); 
     }
     h1 { 
       font-family: 'Serif', 'Times New Roman'; color:rgb(108, 63, 113); padding: 2vh; font-weight: bold; font-size: 3rem; text-shadow: 1px 1px 1px rgb(138, 79, 145); 
     }
     p { font-size: 1.5rem; color: white; } 
     button { padding: 10px 24px; font-size: 1.2rem; background-color:rgb(249,248,113); color: black; border: 1px solid black; border-radius: 4px; cursor: pointer; margin: 10px 0; } 
     button:hover { background-color:rgb(174, 174, 158); }
     .selected { background-color: rgb(167,81,112); } 
     #modeSection { display: flex; justify-content: center; }
     #leftSection { width: 45%; text-align: center; padding: 20px; }
     #rightSection { width: 45%; text-align: center; padding: 20px; color: white; }
     #separator { border-left: 2px solid white; height: auto; }
     .center { text-align: center; }
  </style>
</head>
<body>
  <div class="center">
    <h1>MULTIMETER</h1>
    <div id="modeSection">
      <div id="leftSection">
        <p>Select the relevant mode</p>
        <br>
        <div>
          <button id="button1" onclick="toggleVoltage()">Voltage</button>
          <button id="button2" onclick="toggleCurrent()">Current</button>
        </div>
      </div>
      <div id="separator"></div>
      <div id="rightSection">
        <p>Multimeter Reading</p>
        <p id="reading" style="font-size: 4rem; color: rgb(52, 181, 13);">-</p>
      </div>
    </div>
  </div>
  <script>
    function toggleVoltage() {
      fetch('/toggleVoltage').then(response => response.text()).then(reading => {
        document.getElementById('reading').innerHTML = reading; 
        updateButtonState('button1', reading);
      });
    }

    function toggleCurrent() {
      fetch('/toggleCurrent').then(response => response.text()).then(reading => {
        document.getElementById('reading').innerHTML = reading; 
        updateButtonState('button2', reading);
      });
    }

    function updateButtonState(buttonId, state) {
      const button1 = document.getElementById('button1');
      const button2 = document.getElementById('button2');

      if (button1) button1.classList.remove('selected');
      if (button2) button2.classList.remove('selected');

      if (state !== "No data") {
        document.getElementById(buttonId).classList.add('selected');
      }
    }
  </script>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send(200, "text/html", htmlContent);
}

//Voltage
void handleToggleVoltage() {
  Serial2.print("nooV");
  result = "";
  unsigned long timeout = millis() + 1000; 
  while (millis() < timeout) {
    if (Serial2.available()) {
      result += Serial2.readStringUntil('\n') + "<br>"; 
    }
  }
  server.send(200, "text/plain", result);
}

//Current
void handleToggleCurrent() {
  Serial2.print("nooA"); 
  result = "";
  unsigned long timeout = millis() + 1000; 
  while (millis() < timeout) {
    if (Serial2.available()) {
      result += Serial2.readStringUntil('\n') + "<br>"; 
    }
  }
  server.send(200, "text/plain", result);
}


void setupWiFi() {
  Serial.begin(115200);
  Serial.println("Setting up WiFi Access Point...");
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
}

void setup() {
  Serial.begin(115200); 
  setupWiFi();
  server.on("/", handleRoot);
  server.on("/toggleVoltage", handleToggleVoltage); 
  server.on("/toggleCurrent", handleToggleCurrent); 
  server.begin();
  Serial.println("HTTP server started.");

  // UART
  Serial2.begin(115200, SERIAL_8N1, 16, 17); 
}

void loop() {
  server.handleClient();
}