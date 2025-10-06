#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <WebServer.h>

// ---------------- LCD ----------------
#define col 16
#define lin  2
#define ende 0x27

#define s1 32     // sensor esquerdo
#define s2 33     // sensor direito
#define rele 25   // bomba (relé)

LiquidCrystal_I2C lcd(ende, col, lin);

// ---------------- WIFI ----------------
const char* ssid_AP = "ESP32_WIFI";
const char* senha_AP = "123456789";
WebServer server(80);

// Variáveis globais para log
String logBuffer = "";

// ---------------- Página HTML ----------------
const char MAIN_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-br">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <title>Log do ESP32</title>
  <style>
    body {
        font-family: 'Segoe UI', sans-serif;
        background-color: #f4f6f8;
        color: #333;
        display: flex;
        justify-content: center;
        align-items: flex-start;
        padding: 40px;
    }
    .container {
        width: 90%;
        max-width: 600px;
        background: #fff;
        border-radius: 10px;
        box-shadow: 0 8px 20px rgba(0, 0, 0, 0.1);
        padding: 30px;
    }
    h1 {
        text-align: center;
        margin-bottom: 20px;
        color: #0077cc;
    }
    #log {
        height: 300px;
        overflow-y: auto;
        background: #f1f1f1;
        border-radius: 5px;
        padding: 15px;
        font-family: monospace;
        white-space: pre-wrap;
        font-size: 0.95rem;
        border: 1px solid #ccc;
    }
    .timestamp {
        color: #888;
        font-size: 0.8rem;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>Log do ESP32</h1>
    <div id="log">Carregando log...</div>
  </div>

  <script>
    async function fetchLog() {
        try {
            const response = await fetch('/log');
            if (!response.ok) throw new Error('Erro na requisição');
            const text = await response.text();
            document.getElementById('log').textContent = text;
        } catch (err) {
            document.getElementById('log').textContent = 'Erro ao buscar log: ' + err.message;
        }
    }
    setInterval(fetchLog, 3000);
    fetchLog();
  </script>
</body>
</html>
)rawliteral";

// ---------------- Rotas ----------------
void handleRoot() {
  server.send(200, "text/html", MAIN_page);
}

void handleLog() {
  server.send(200, "text/plain", logBuffer);
}

// ---------------- Setup ----------------
void setup() {
  pinMode(s1, INPUT);
  pinMode(s2, INPUT);
  pinMode(rele, OUTPUT);

  lcd.init();
  lcd.backlight();
  lcd.clear();

  Serial.begin(115200);

  lcd.setCursor(1, 1);
  lcd.print("Iniciando...");
  delay(1000);
  lcd.clear();

  // Inicia AP
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid_AP, senha_AP, 6);

  Serial.println("AP iniciado!");
  Serial.print("IP do ESP32: ");
  Serial.println(WiFi.softAPIP());

  // Servidor
  server.on("/", handleRoot);
  server.on("/log", handleLog);
  server.begin();
  Serial.println("Servidor iniciado!");
}

// ---------------- Loop ----------------
void loop() {
  server.handleClient();

  int s_direito = analogRead(s2);
  int s_esquerdo = analogRead(s1);

  lcd.setCursor(0,0);
  lcd.print("s_d: ");
  lcd.print(s_direito);
  lcd.setCursor(0,1);
  lcd.print("s_e: ");
  lcd.print(s_esquerdo);

  // Atualiza log
  logBuffer = "";
  logBuffer += "Sensor direito: " + String(s_direito) + "\n";
  logBuffer += "Sensor esquerdo: " + String(s_esquerdo) + "\n";

  if ((s_direito > 2000) || (s_esquerdo > 2000)) {
    logBuffer += "Solo seco! Ligando bomba...\n";
    Serial.println("Lig. a bomba");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Solo seco!");
    digitalWrite(rele, HIGH);
    delay(5000);

    logBuffer += "Desligando bomba...\n";
    Serial.println("Desl. a bomba");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Desl. a bomba");
    digitalWrite(rele, LOW);
    delay(5000);
    lcd.clear();
  } else {
    logBuffer += "Solo úmido :)\n";
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Solo umido :)");
    delay(1000);
    lcd.clear();
  }

  delay(5000);
}
