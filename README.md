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

---
