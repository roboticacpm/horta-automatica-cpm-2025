# Projeto de Monitoramento de Umidade do Solo com ESP32

Este projeto utiliza um **ESP32**, sensores de umidade do solo, **relé para controle de bomba**, **display LCD I2C** e um **servidor web** para exibir **logs em tempo real** via Wi-Fi (modo Access Point).

---

## Funcionalidades

- Leitura dos sensores de umidade (2 sensores analógicos).
- Exibição das leituras no **LCD 16x2**.
- Ativação automática da bomba de irrigação (via relé) quando o solo estiver seco.
- Criação de uma **rede Wi-Fi local** (modo AP) para visualização dos logs via navegador.
- Interface web simples e responsiva para monitoramento.

---

## Hardware Necessário

- 1x ESP32
- 2x Sensores de umidade do solo analógicos
- 1x Relé (para controle de bomba ou LED simulando bomba)
- 1x Display LCD 16x2 com interface I2C
- Fios jumpers e fonte de alimentação apropriada
- 1x Bomba de água apropriada
- 1x Sensor de nível de água

---


## Interface Web

- Acesse a rede Wi-Fi criada pelo ESP32:
  - **SSID:** `ESP32_WIFI`
  - **Senha:** `123456789`
- No navegador, acesse o IP: [http://192.168.4.1](http://192.168.4.1)
- A página exibirá os logs atualizados a cada 3 segundos:
  - Leituras dos sensores
  - Estado do solo (úmido/seco)
  - Ação do sistema (ligando/desligando a bomba)

---

## Lógica de Funcionamento

1. O ESP32 inicia como **Access Point (AP)**.
2. Lê os sensores de umidade.
3. Exibe os valores no LCD.
4. Se qualquer sensor detectar valor > 2000 (indicação de solo seco):
   - Liga a bomba por 5 segundos.
   - Desliga a bomba e aguarda 5 segundos.
5. Se o solo estiver úmido:
   - Mostra mensagem de "Solo úmido" e aguarda 1 segundo.
6. Todos os eventos são registrados em uma variável `logBuffer`, atualizada na interface web.

---

## Bibliotecas Necessárias

Antes de compilar o código no Arduino IDE ou PlatformIO, instale as seguintes bibliotecas:

- `Wire.h` (padrão)
- `LiquidCrystal_I2C` *(para o LCD)*
- `WiFi.h` *(inclusa no framework do ESP32)*
- `WebServer.h` *(para o servidor HTTP)*

## Código
```cpp
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
```

## Autores
***Grupo de Robótica do Colégio da Polícia Militar da Paraíba, do ano de 2025.***
- Representantes (professores): Christiano Rodrigues e Wandeilson Ferreira.
- Integrantes do grupo (alunos): Vitor Veloso, Pedro Ferreira, Matheus Henrique e Lyrvson Bryan.

---
