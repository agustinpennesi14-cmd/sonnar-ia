
/*
  SONNAR IA - MENU OLED
  OLED SSD1306 128x32
  PCF8574: direccion I2C 0x27

  Botones:
  UP      = P0
  ENTER   = P1
  DOWN    = P2
  GATILLO = P4

  Flash blanco:
  GPIO 4 = LED del flash de la ESP32-CAM
  Encendido solamente en la pantalla ESCANEAR.

  BCLK del MAX98357A fue trasladado a GPIO 2.
*/

// =====================================================
// CONFIGURACION DEL FLASH
// =====================================================

#define LED_FLASH 4

void menuControlarFlash(bool encendido) {
  pinMode(LED_FLASH, OUTPUT);
  digitalWrite(LED_FLASH, encendido ? HIGH : LOW);
}

// =====================================================
// CONFIGURACION DEL MENU
// =====================================================

Adafruit_SSD1306 displayMenu(128, 32, &Wire, -1);

enum PantallaMenu : uint8_t {
  MENU_PRINCIPAL,
  MENU_ESCANEAR,
  MENU_VOLUMEN,
  MENU_SISTEMA,
  MENU_ACERCA,
  MENU_CAMARA_ESTADO,
  MENU_WIFI_PORTAL
};

PantallaMenu pantallaActual = MENU_PRINCIPAL;

int volumenActual = 20;

const char* opcionesMenu[] = {
  "Escanear",
  "Volumen",
  "Sistema",
  "Acerca de"
};

const uint8_t CANT_OPCIONES = 4;
uint8_t opcionSeleccionada = 0;

const char* opcionesSistema[] = {
  "Wi-Fi",
  "Camara",
  "Volver"
};
const uint8_t CANT_OPCIONES_SISTEMA = 3;
uint8_t opcionSistemaSeleccionada = 0;

bool menuPrevArriba = false;
bool menuPrevEnter = false;
bool menuPrevAbajo = false;
bool menuPrevGatillo = false;

unsigned long menuUltimoInput = 0;
const unsigned long MENU_DEBOUNCE_MS = 160;

// =====================================================
// PROTOTIPOS
// =====================================================

void menuInicializar();
void menuLoop();
void menuEntrar();
void menuEjecutarEscaneo();

void menuDibujarPrincipal();
void menuDibujarEscaneo(
  const char* linea2 = "Gatillo: capturar"
);
void menuDibujarVolumen();
void menuDibujarSistema();
void menuDibujarEstadoCamara();
void menuDibujarPortalWiFi();
void menuDibujarAcerca();
void menuTitulo(const char* titulo);

void menuAnimacionInicio();
void menuTransicionOpcion(uint8_t anterior, int8_t direccion);
void menuDibujarTextoGrandeCentrado(const char* texto, int16_t y);

// =====================================================
// INICIALIZAR OLED Y PCF8574
// =====================================================

void menuInicializar() {

  // El flash comienza apagado.
  menuControlarFlash(false);

  Wire.beginTransmission(PCF8574_ADDR);
  Wire.write(0xFF);

  uint8_t errorPCF = Wire.endTransmission();

  if (errorPCF != 0) {
    Serial.println("ERROR: PCF8574 no responde en 0x27.");
  }

  if (!displayMenu.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("ERROR: OLED no encontrada en 0x3C.");
    return;
  }

  displayMenu.clearDisplay();
  displayMenu.setTextColor(SSD1306_WHITE);
  displayMenu.setTextWrap(false);
  displayMenu.display();

  audio.setVolume(volumenActual);

  menuAnimacionInicio();
  menuDibujarPrincipal();

  Serial.println("Menu OLED listo.");
}

// =====================================================
// ANIMACION DE INICIO
// Texto de izquierda a derecha, mas rapida
// =====================================================

void menuAnimacionInicio() {

  // Garantizar que el flash este apagado durante el inicio.
  menuControlarFlash(false);

  displayMenu.clearDisplay();
  displayMenu.display();

  const int16_t yInicio = 5;
  const int16_t xFinal = 10;

  for (int16_t x = -108; x <= xFinal; x += 4) {

    displayMenu.clearDisplay();

    displayMenu.setTextSize(2);
    displayMenu.setTextColor(SSD1306_WHITE);
    displayMenu.setCursor(x, yInicio);
    displayMenu.print("SONNAR IA");

    displayMenu.display();

    delay(25);
  }

  delay(400);

  displayMenu.clearDisplay();

  menuDibujarTextoGrandeCentrado("SONNAR IA", 4);

  displayMenu.drawLine(8, 22, 119, 22, SSD1306_WHITE);

  displayMenu.setTextSize(1);
  displayMenu.setCursor(1, 24);
  displayMenu.print("ASISTENCIA INTELIGENTE");

  displayMenu.display();

  delay(900);

  for (uint8_t progreso = 0; progreso <= 100; progreso += 10) {

    displayMenu.clearDisplay();

    displayMenu.setTextSize(1);
    displayMenu.setCursor(0, 0);
    displayMenu.print("SONNAR IA");

    displayMenu.setCursor(0, 11);
    displayMenu.print("Iniciando sistema");

    displayMenu.drawRect(0, 23, 128, 8, SSD1306_WHITE);

    uint8_t ancho = (uint16_t)124 * progreso / 100;

    if (ancho > 0) {
      displayMenu.fillRect(2, 25, ancho, 4, SSD1306_WHITE);
    }

    displayMenu.display();

    delay(60);
  }

  delay(400);
}

// =====================================================
// TEXTO GRANDE CENTRADO
// =====================================================

void menuDibujarTextoGrandeCentrado(
  const char* texto,
  int16_t y
) {

  displayMenu.setTextSize(2);
  displayMenu.setTextColor(SSD1306_WHITE);

  int16_t anchoTexto = strlen(texto) * 12;
  int16_t x = (128 - anchoTexto) / 2;

  if (x < 0) {
    x = 0;
  }

  displayMenu.setCursor(x, y);
  displayMenu.print(texto);
}

// =====================================================
// LEER ESTADO DEL PCF8574
// =====================================================

bool menuLeerPCF(uint8_t &estado) {

  Wire.requestFrom((uint8_t)PCF8574_ADDR, (uint8_t)1);

  if (Wire.available() < 1) {
    return false;
  }

  estado = Wire.read();
  return true;
}

// =====================================================
// TITULO COMUN
// =====================================================

void menuTitulo(const char* titulo) {

  displayMenu.clearDisplay();
  displayMenu.setTextSize(1);
  displayMenu.setTextColor(SSD1306_WHITE);
  displayMenu.setCursor(0, 0);
  displayMenu.print(titulo);

  displayMenu.drawLine(0, 9, 127, 9, SSD1306_WHITE);
}

// =====================================================
// MENU PRINCIPAL: UNA OPCION + FLECHA
// =====================================================

void menuDibujarPrincipal() {

  // Flash apagado en el menu principal.
  menuControlarFlash(false);

  pantallaActual = MENU_PRINCIPAL;

  displayMenu.clearDisplay();

  displayMenu.setTextSize(1);
  displayMenu.setTextColor(SSD1306_WHITE);
  displayMenu.setCursor(0, 0);
  displayMenu.print("SONNAR IA");

  displayMenu.setCursor(105, 0);
  displayMenu.print(opcionSeleccionada + 1);
  displayMenu.print("/");
  displayMenu.print(CANT_OPCIONES);

  displayMenu.setTextSize(2);
  displayMenu.setCursor(0, 11);
  displayMenu.print(">");

  const char* texto = opcionesMenu[opcionSeleccionada];

  int16_t anchoTexto = strlen(texto) * 12;
  int16_t x = (128 - anchoTexto + 12) / 2;

  if (x < 20) {
    x = 20;
  }

  displayMenu.setCursor(x, 11);
  displayMenu.print(texto);

  displayMenu.display();
}

// =====================================================
// TRANSICION ENTRE OPCIONES
// =====================================================

void menuTransicionOpcion(
  uint8_t anterior,
  int8_t direccion
) {

  const char* textoAnterior = opcionesMenu[anterior];
  const char* textoNuevo = opcionesMenu[opcionSeleccionada];

  for (int16_t paso = 0; paso <= 10; paso++) {

    displayMenu.clearDisplay();

    int16_t desplazamiento = paso * 2;
    int16_t yAnterior;
    int16_t yNuevo;

    if (direccion > 0) {
      yAnterior = 8 - desplazamiento;
      yNuevo = 32 - desplazamiento;
    } else {
      yAnterior = 8 + desplazamiento;
      yNuevo = -16 + desplazamiento;
    }

    menuDibujarTextoGrandeCentrado(textoAnterior, yAnterior);
    menuDibujarTextoGrandeCentrado(textoNuevo, yNuevo);

    displayMenu.display();

    delay(25);
  }

  menuDibujarPrincipal();
}

// =====================================================
// PANTALLA ESCANEAR
// Flash encendido mientras estamos en esta pantalla
// =====================================================

void menuDibujarEscaneo(const char* linea2) {

  menuControlarFlash(true);

  pantallaActual = MENU_ESCANEAR;

  menuTitulo("ESCANEAR");

  displayMenu.setCursor(0, 13);
  displayMenu.print("Apunte al objeto");

  displayMenu.setCursor(0, 23);
  displayMenu.print(linea2);

  displayMenu.display();
}

// =====================================================
// PANTALLA VOLUMEN
// =====================================================

void menuDibujarVolumen() {

  menuControlarFlash(false);

  pantallaActual = MENU_VOLUMEN;

  menuTitulo("VOLUMEN");

  displayMenu.setCursor(0, 13);
  displayMenu.print("Nivel: ");
  displayMenu.print(volumenActual);
  displayMenu.print("/21");

  displayMenu.drawRect(0, 24, 100, 7, SSD1306_WHITE);

  int ancho = (int)(96.0f * volumenActual / 21.0f);

  if (ancho > 0) {
    displayMenu.fillRect(2, 26, ancho, 3, SSD1306_WHITE);
  }

  displayMenu.display();
}

// =====================================================
// SUBMENU SISTEMA
// Wi-Fi / estado de camara / volver
// =====================================================

void menuDibujarSistema() {

  menuControlarFlash(false);
  pantallaActual = MENU_SISTEMA;

  displayMenu.clearDisplay();
  displayMenu.setTextSize(1);
  displayMenu.setTextColor(SSD1306_WHITE);
  displayMenu.setCursor(0, 0);
  displayMenu.print("SISTEMA");

  displayMenu.setCursor(100, 0);
  displayMenu.print(opcionSistemaSeleccionada + 1);
  displayMenu.print("/");
  displayMenu.print(CANT_OPCIONES_SISTEMA);

  displayMenu.drawLine(0, 9, 127, 9, SSD1306_WHITE);

  displayMenu.setTextSize(2);
  displayMenu.setCursor(0, 12);
  displayMenu.print(">");

  const char* texto = opcionesSistema[opcionSistemaSeleccionada];
  int16_t anchoTexto = strlen(texto) * 12;
  int16_t x = (128 - anchoTexto + 12) / 2;
  if (x < 20) {
    x = 20;
  }

  displayMenu.setCursor(x, 12);
  displayMenu.print(texto);
  displayMenu.display();
}

// =====================================================
// ESTADO DE LA CAMARA
// =====================================================

void menuDibujarEstadoCamara() {

  menuControlarFlash(false);
  pantallaActual = MENU_CAMARA_ESTADO;

  menuTitulo("CAMARA");

  displayMenu.setCursor(0, 13);
  displayMenu.print("Estado: ");
  displayMenu.print(
    esp_camera_sensor_get() != nullptr ? "OK" : "ERROR"
  );

  displayMenu.setCursor(0, 23);
  displayMenu.print("ENTER: volver");

  displayMenu.display();
}

// =====================================================
// PANTALLA DE AYUDA DEL PORTAL WIFI
// =====================================================

void menuDibujarPortalWiFi() {

  menuControlarFlash(false);
  pantallaActual = MENU_WIFI_PORTAL;

  displayMenu.clearDisplay();
  displayMenu.setTextColor(SSD1306_WHITE);
  displayMenu.setTextSize(1);

  displayMenu.setCursor(0, 0);
  displayMenu.print("CONFIGURAR WIFI");

  displayMenu.setCursor(0, 8);
  displayMenu.print("Red: SonnarIA-Setup");

  displayMenu.setCursor(0, 16);
  displayMenu.print("Clave: SonnarIA2026");

  displayMenu.setCursor(0, 24);
  displayMenu.print("IP: 192.168.4.1");

  displayMenu.display();
}

// =====================================================
// PANTALLA ACERCA DE
// =====================================================

void menuDibujarAcerca() {

  menuControlarFlash(false);

  menuTitulo("ACERCA DE");

  displayMenu.setCursor(0, 12);
  displayMenu.print("Sonnar IA v1.0");

  displayMenu.setCursor(0, 22);
  displayMenu.print("Expo Edu-Innova");

  displayMenu.display();

  pantallaActual = MENU_ACERCA;
}

// =====================================================
// ENTRAR EN UNA OPCION
// =====================================================

void menuEntrar() {

  switch (opcionSeleccionada) {

    case 0:
      menuDibujarEscaneo();
      break;

    case 1:
      menuDibujarVolumen();

      hablarTexto(
        "Presione los botones para subir y bajar el volumen del sistema."
      );
      break;

    case 2:
      opcionSistemaSeleccionada = 0;
      menuDibujarSistema();
      break;

    case 3:
      menuDibujarAcerca();
      break;
  }
}

// =====================================================
// EJECUTAR ESCANEO REAL CON GEMINI
// =====================================================

void menuEjecutarEscaneo() {

  if (sistemaEscaneando) {
    return;
  }

  if (WiFi.status() != WL_CONNECTED) {
    menuDibujarEscaneo("WiFi desconectado");
    hablarTexto("No hay conexion WiFi.");
    return;
  }

  if (esp_camera_sensor_get() == nullptr) {
    menuDibujarEscaneo("Error de camara");
    hablarTexto("Hay un problema con la camara.");
    return;
  }

  // El flash permanece encendido durante el analisis.
  menuDibujarEscaneo("Analizando imagen...");

  Serial.println("[MENU] Escaneo iniciado desde P4.");

  capturarYEnviarAGemini();

  // Seguimos en la pantalla de escaneo.
  menuDibujarEscaneo("Gatillo: repetir");
}

// =====================================================
// BUCLE DEL MENU
// =====================================================

void menuLoop() {

  if (sistemaEscaneando) {
    return;
  }

  uint8_t estado = 0xFF;

  if (!menuLeerPCF(estado)) {
    return;
  }

  // Los botones son activos en LOW.
  bool arriba  = !(estado & (1 << 0));
  bool enter   = !(estado & (1 << 1));
  bool abajo   = !(estado & (1 << 2));
  bool gatillo = !(estado & (1 << 4));

  unsigned long ahora = millis();

  bool inputPermitido =
    (ahora - menuUltimoInput >= MENU_DEBOUNCE_MS);

  // SUBIR
  if (inputPermitido && arriba && !menuPrevArriba) {

    menuUltimoInput = ahora;

    if (pantallaActual == MENU_PRINCIPAL) {

      uint8_t anterior = opcionSeleccionada;

      opcionSeleccionada =
        (opcionSeleccionada + CANT_OPCIONES - 1)
        % CANT_OPCIONES;

      menuTransicionOpcion(anterior, -1);

    } else if (pantallaActual == MENU_VOLUMEN) {

      if (volumenActual < 21) {
        volumenActual++;
      }

      audio.setVolume(volumenActual);
      menuDibujarVolumen();

    } else if (pantallaActual == MENU_SISTEMA) {

      opcionSistemaSeleccionada =
        (opcionSistemaSeleccionada + CANT_OPCIONES_SISTEMA - 1)
        % CANT_OPCIONES_SISTEMA;

      menuDibujarSistema();
    }
  }

  // BAJAR
  else if (inputPermitido && abajo && !menuPrevAbajo) {

    menuUltimoInput = ahora;

    if (pantallaActual == MENU_PRINCIPAL) {

      uint8_t anterior = opcionSeleccionada;

      opcionSeleccionada =
        (opcionSeleccionada + 1) % CANT_OPCIONES;

      menuTransicionOpcion(anterior, 1);

    } else if (pantallaActual == MENU_VOLUMEN) {

      if (volumenActual > 0) {
        volumenActual--;
      }

      audio.setVolume(volumenActual);
      menuDibujarVolumen();

    } else if (pantallaActual == MENU_SISTEMA) {

      opcionSistemaSeleccionada =
        (opcionSistemaSeleccionada + 1)
        % CANT_OPCIONES_SISTEMA;

      menuDibujarSistema();
    }
  }

  // ENTER
  else if (inputPermitido && enter && !menuPrevEnter) {

    menuUltimoInput = ahora;

    if (pantallaActual == MENU_PRINCIPAL) {
      menuEntrar();

    } else if (pantallaActual == MENU_SISTEMA) {

      if (opcionSistemaSeleccionada == 0) {
        if (iniciarPortalConfiguracionWiFi()) {
          menuDibujarPortalWiFi();
        } else {
          menuDibujarSistema();
          hablarTexto("No se pudo iniciar la configuracion WiFi.");
        }

      } else if (opcionSistemaSeleccionada == 1) {
        menuDibujarEstadoCamara();

      } else {
        menuDibujarPrincipal();
      }

    } else if (pantallaActual == MENU_WIFI_PORTAL) {

      // ENTER cancela el portal y vuelve al submenu Sistema.
      detenerPortalConfiguracionWiFi();
      menuDibujarSistema();

    } else if (pantallaActual == MENU_CAMARA_ESTADO) {

      menuDibujarSistema();

    } else {
      // En volumen, escaneo y acerca de, ENTER vuelve al menu principal.
      menuDibujarPrincipal();
    }
  }

  // GATILLO P4
  else if (inputPermitido && gatillo && !menuPrevGatillo) {

    menuUltimoInput = ahora;

    if (pantallaActual == MENU_ESCANEAR) {
      menuEjecutarEscaneo();
    }
  }

  // Guardar estados anteriores.
  menuPrevArriba = arriba;
  menuPrevEnter = enter;
  menuPrevAbajo = abajo;
  menuPrevGatillo = gatillo;
}

// =====================================================
// ANIMACION DE ONDA DE AUDIO - SONNAR IA
// =====================================================

unsigned long ondaUltimoFrame = 0;
uint8_t ondaFase = 0;
bool ondaVisible = false;
