
/*
=========================================================
                    SONNAR IA
=========================================================
ESP32-CAM AI-Thinker
Gemini Vision - Google TTS - MAX98357A
OLED SSD1306 + PCF8574
Menu principal en menu.ino
=========================================================
*/

#include <WiFi.h>
#include <Preferences.h>
#include <WebServer.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include "esp_camera.h"
#include "mbedtls/base64.h"
#include "Audio.h"
#include "esp_heap_caps.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// =====================================================
// WIFI Y GEMINI
// Wi-Fi persistente: credenciales en Preferences/NVS.
// =====================================================

// Las credenciales Wi-Fi se cargan desde Preferences (memoria NVS).
// La clave anterior quedó expuesta: pegá aquí una NUEVA API key de Gemini.
const char* API_KEY = "";
const char* GEMINI_MODEL = "gemini-3.5-flash-lite";

// =====================================================
// I2C: OLED Y PCF8574
// =====================================================

#define SDA_PIN 13
#define SCL_PIN 14
#define PCF_ADDRESS 0x27

// Alias utilizados por menu.ino
#define I2C_SDA SDA_PIN
#define I2C_SCL SCL_PIN
#define PCF8574_ADDR PCF_ADDRESS

// =====================================================
// PCF8574: BOTONES
// UP = P0 | ENTER = P1 | DOWN = P2
// GATILLO = P4
// =====================================================

#define PIN_UP 0
#define PIN_ENTER 1
#define PIN_DOWN 2
#define PULSADOR 4

// =====================================================
// MAX98357A
// =====================================================

#define I2S_BCLK 2
#define I2S_LRC  15
#define I2S_DOUT 12

// =====================================================
// CAMARA AI-THINKER ESP32-CAM
// =====================================================

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// =====================================================
// AUDIO
// =====================================================

Audio audio(I2S_NUM_1);
String textoTTSActual = "";

// =====================================================
// ESTADO DEL SISTEMA
// =====================================================

bool sistemaEscaneando = false;
bool pulsadorAnterior = HIGH;

unsigned long ultimoAviso = 0;
const unsigned long INTERVALO_AVISO = 5000;

// =====================================================
// GEMINI: REINTENTOS
// =====================================================

const int MAX_INTENTOS_GEMINI = 3;
const unsigned long ESPERA_REINTENTO_MS = 500;

// =====================================================
// MEDICION DE LATENCIA
// =====================================================

unsigned long tiempoInicioEscaneo = 0;
unsigned long tiempoFinCaptura = 0;
unsigned long tiempoFinBase64 = 0;
unsigned long tiempoInicioGemini = 0;
unsigned long tiempoFinGemini = 0;
unsigned long tiempoInicioTTS = 0;
unsigned long tiempoFinTTSConexion = 0;

// =====================================================
// FUNCIONES DE OTRAS PESTAÑAS
// =====================================================

bool iniciarCamara();
void iniciarAudio();
bool conectarWiFi();
void gestionarPortalWiFi();
bool iniciarPortalConfiguracionWiFi();
void detenerPortalConfiguracionWiFi();
bool wifiPortalEstaActivo();
void anunciarEspera();
void capturarYEnviarAGemini();

void menuInicializar();
void menuLoop();

// =====================================================
// SETUP
// =====================================================

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("================================");
  Serial.println("          SONNAR IA");
  Serial.println("================================");

  // Inicializar bus I2C.
  // menuInicializar() se encarga del OLED y del PCF8574.
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(100000);

  // Inicializar cámara.
  if (!iniciarCamara()) {
    Serial.println("ERROR CRITICO: CAMARA");
    while (true) {
      delay(1000);
    }
  }

  Serial.println("Camara inicializada.");

  // Inicializar audio.
  iniciarAudio();
  Serial.println("Audio inicializado.");

  // Intentar conectar a la última red guardada.
  // Si no hay credenciales o la red falla, el sistema sigue arrancando
  // para que se pueda configurar el Wi-Fi desde el menú y el celular.
  if (!conectarWiFi()) {
    Serial.println("Wi-Fi no conectado. Se puede configurar desde Sistema > Wi-Fi.");
  } else {
    Serial.println("Wi-Fi conectado.");
  }

  // Inicializar OLED, PCF8574 y menú.
  menuInicializar();

  Serial.println("SISTEMA LISTO");
  Serial.println("Usa los botones del menu para navegar.");

  ultimoAviso = millis();
}

// =====================================================
// LOOP
// =====================================================


void loop() {
  // Mantener el audio funcionando.
  audio.loop();

  // Atender el portal web cuando se configura el Wi-Fi desde el celular.
  gestionarPortalWiFi();

  // Gestionar botones, menú y gatillo.
  menuLoop();


}

