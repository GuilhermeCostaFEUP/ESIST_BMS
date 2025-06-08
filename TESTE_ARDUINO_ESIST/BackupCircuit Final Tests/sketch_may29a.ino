#include <WiFiS3.h>

// Credenciais do AP
#define SECRET_SSID  "RedeFixe"
#define SECRET_PASS  "12345678"

WiFiServer server(80);

const int numCells = 4;
const int analogPins[numCells] = {A0, A1, A2, A3};
const int balancePins[numCells] = {5, 6, 7, 8};

bool balanceActive = false;
float balanceMinVoltage = 0.0;
const float tolerance = 0.02;
const float VREF = 4.38;

float readVoltage(int pin) {
  int raw = analogRead(pin);
  return raw * (VREF / 1023.0);
}

void startBalance() {
  float voltages[numCells];
  balanceMinVoltage = 5.0;

  for (int i = 0; i < numCells; i++) {
    voltages[i] = readVoltage(analogPins[i]);
    if (voltages[i] < balanceMinVoltage) {
      balanceMinVoltage = voltages[i];
    }
  }
  balanceActive = true;
  Serial.print("Balanceamento iniciado. Min Voltage: ");
  Serial.println(balanceMinVoltage, 3);
}

void stopBalance() {
  balanceActive = false;
  for (int i = 0; i < numCells; i++) {
    digitalWrite(balancePins[i], LOW);
  }
  Serial.println("Balanceamento finalizado.");
}

void handleBalance() {
  if (!balanceActive) return;

  bool allBalanced = true;
  for (int i = 0; i < numCells; i++) {
    float v = readVoltage(analogPins[i]);
    if (v > balanceMinVoltage + tolerance) {
      digitalWrite(balancePins[i], HIGH);
      allBalanced = false;
    } else {
      digitalWrite(balancePins[i], LOW);
    }
  }

  if (allBalanced) {
    stopBalance();
  }
}

void sendVoltagesPage(WiFiClient &client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println();

  client.println(R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>Cell Monitoring - Dark Mode</title>
  <meta http-equiv="refresh" content="60">
  <script>
    setInterval(() => {
      window.location.reload();
    }, 3000);
    function toggleBalance() {
      fetch('/balance/toggle')
        .then(resp => resp.text())
        .then(alert);
    }
  </script>
  <style>
    body {
      font-family: "Segoe UI", Tahoma, Geneva, Verdana, sans-serif;
      background-color: #121212;
      color: #e0e0e0;
      padding: 20px;
      max-width: 600px;
      margin: auto;
    }
    h1 {
      color: #66aaff;
      text-align: center;
    }
    .cell {
      background-color: #1e1e1e;
      padding: 10px 15px;
      margin: 10px 0;
      border-radius: 20px;
      box-shadow: 0 2px 5px rgba(0,0,0,0.7);
      border: 1px solid #333;
    }
    .status {
      font-weight: bold;
      text-align: center;
      margin: 20px 0;
    }
    .active {
      color: #4caf50;
    }
    .inactive {
      color: #f44336;
    }
    button {
      display: block;
      width: 100%;
      padding: 12px;
      background-color: #2979ff;
      color: #ffeb3b; /* texto amarelo */
      border: none;
      border-radius: 6px;
      font-size: 16px;
      cursor: pointer;
      margin-top: 20px;
      box-shadow: 0 3px 7px rgba(41, 121, 255, 0.6);
      transition: color 0.3s ease, background-color 0.3s ease;
    }
    button:hover {
      background-color: #1565c0;
    }
    .discharge {
      background-color: #3e2723;
      padding: 10px;
      margin: 10px 0;
      border-radius: 20px;
      color: #ffcc80;
    }
  </style>
</head>
<body>
  <h1>Cell Voltages</h1>
)rawliteral");

  for (int i = 0; i < numCells; i++) {
    float v = readVoltage(analogPins[i]);
    client.print("<div class='cell'>Cell ");
    client.print(i);
    client.print(": <strong>");
    client.print(v, 3);
    client.println(" V</strong></div>");
  }

  client.print("<div class='status'>Balancing is ");
  client.print(balanceActive ? "<span class='active'>ACTIVE</span>" : "<span class='inactive'>INACTIVE</span>");
  client.println("</div>");

  if (balanceActive) {
    client.println("<div class='discharge'><strong>Cells with balancing circuit on:</strong> ");
    bool anyActive = false;
    for (int i = 0; i < numCells; i++) {
      if (digitalRead(balancePins[i]) == HIGH) {
        client.print("Cell ");
        client.print(i);
        client.print(" ");
        anyActive = true;
      }
    }
    if (!anyActive) client.print("None");
    client.println("</div>");
  }

  client.println(R"rawliteral(
  <button onclick="toggleBalance()">Toggle Passive Balancing</button>
</body>
</html>
)rawliteral");

  client.println();
}

void sendSimpleResponse(WiFiClient &client, String msg) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/plain");
  client.println("Connection: close");
  client.println();
  client.print(msg);
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  for (int i = 0; i < numCells; i++) {
    pinMode(balancePins[i], OUTPUT);
    digitalWrite(balancePins[i], LOW);
  }

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }

  int status = WiFi.beginAP(SECRET_SSID, SECRET_PASS);
  if (status != WL_AP_LISTENING) {
    Serial.println("Failed to start AP");
    while (true);
  }

  delay(10000);
  server.begin();

  Serial.println("Access Point criado.");
  Serial.print("SSID: ");
  Serial.println(SECRET_SSID);
  Serial.print("Senha: ");
  Serial.println(SECRET_PASS);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  WiFiClient client = server.available();

  if (client) {
    Serial.println("Novo cliente conectado");
    String request = client.readStringUntil('\r');
    client.readStringUntil('\n');
    Serial.println("Request: " + request);

    if (request.startsWith("GET /balance/toggle")) {
      if (!balanceActive) {
        startBalance();
        sendSimpleResponse(client, "Balanceamento iniciado");
      } else {
        stopBalance();
        sendSimpleResponse(client, "Balanceamento parado");
      }
    } else if (request.startsWith("GET /")) {
      sendVoltagesPage(client);
    } else {
      sendSimpleResponse(client, "Endpoint invÃ¡lido");
    }

    client.stop();
    Serial.println("Cliente desconectado");
  }

  handleBalance();
}