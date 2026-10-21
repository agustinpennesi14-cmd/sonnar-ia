
#include <DNSServer.h>

// =====================================================
// WIFI PERSISTENTE + PORTAL CAUTIVO
// Sonnar IA v1.2
// =====================================================

static const char* WIFI_AP_NOMBRE = "SonnarIA-Setup";
static const char* WIFI_AP_CLAVE = "SonnarIA2026";
static const char* WIFI_PREF_NAMESPACE = "sonnarwifi";

static const byte DNS_PORT = 53;

Preferences preferenciasWiFi;
WebServer servidorWiFi(80);
DNSServer servidorDNS;

String wifiSSIDGuardado = "";
String wifiPasswordGuardada = "";
String wifiSSIDPendiente = "";
String wifiPasswordPendiente = "";

bool portalWiFiActivo = false;
bool portalIntentandoConectar = false;
bool portalCerrarTrasExito = false;

unsigned long portalInicioIntento = 0;
unsigned long portalCerrarEn = 0;

String estadoPortalWiFi =
  "Conectate a SonnarIA-Setup desde el celular.";

void menuDibujarSistema();

static const unsigned long WIFI_TIMEOUT_CONEXION_MS = 20000;
static const unsigned long WIFI_ESPERA_CIERRE_PORTAL_MS = 5000;

// =====================================================
// CARGAR / GUARDAR CREDENCIALES
// =====================================================

bool cargarCredencialesWiFi() {
  preferenciasWiFi.begin(WIFI_PREF_NAMESPACE, true);

  wifiSSIDGuardado = preferenciasWiFi.getString("ssid", "");
  wifiPasswordGuardada = preferenciasWiFi.getString("pass", "");

  preferenciasWiFi.end();

  return wifiSSIDGuardado.length() > 0;
}

bool guardarCredencialesWiFi(
  const String& ssid,
  const String& password
) {
  preferenciasWiFi.begin(WIFI_PREF_NAMESPACE, false);

  size_t escritoSSID = preferenciasWiFi.putString("ssid", ssid);
  size_t escritoPassword = preferenciasWiFi.putString("pass", password);

  preferenciasWiFi.end();

  if (escritoSSID == 0 || escritoPassword == 0) {
    Serial.println("[WIFI] ERROR guardando credenciales.");
    return false;
  }

  wifiSSIDGuardado = ssid;
  wifiPasswordGuardada = password;

  return true;
}

// =====================================================
// API KEY DE GEMINI: CARGAR / GUARDAR EN LA MISMA NVS
// =====================================================

bool cargarAPIKeyGemini() {
  preferenciasWiFi.begin(WIFI_PREF_NAMESPACE, true);
  API_KEY = preferenciasWiFi.getString("gemini", "");
  preferenciasWiFi.end();

  Serial.println(API_KEY.length() > 0
    ? "[GEMINI] API Key cargada desde memoria."
    : "[GEMINI] No hay API Key configurada.");

  return API_KEY.length() > 0;
}

bool guardarAPIKeyGemini(const String& nuevaClave) {
  if (nuevaClave.length() < 20 || nuevaClave.length() > 200) {
    return false;
  }

  preferenciasWiFi.begin(WIFI_PREF_NAMESPACE, false);
  size_t escrito = preferenciasWiFi.putString("gemini", nuevaClave);
  preferenciasWiFi.end();

  if (escrito == 0) {
    Serial.println("[GEMINI] ERROR guardando API Key.");
    return false;
  }

  API_KEY = nuevaClave;
  Serial.println("[GEMINI] API Key guardada en memoria persistente.");
  return true;
}

// =====================================================
// CONECTAR A LA RED GUARDADA
// =====================================================

bool conectarWiFi() {
  Serial.println();
  Serial.println("[WIFI] Buscando credenciales guardadas...");

  if (!cargarCredencialesWiFi()) {
    Serial.println("[WIFI] No hay una red guardada.");
    WiFi.mode(WIFI_STA);
    return false;
  }

  Serial.print("[WIFI] Conectando a: ");
  Serial.println(wifiSSIDGuardado);

  WiFi.mode(WIFI_STA);
  WiFi.begin(
    wifiSSIDGuardado.c_str(),
    wifiPasswordGuardada.c_str()
  );

  unsigned long inicio = millis();

  while (
    WiFi.status() != WL_CONNECTED &&
    millis() - inicio < WIFI_TIMEOUT_CONEXION_MS
  ) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[WIFI] No se pudo conectar.");
    WiFi.disconnect(false, false);
    return false;
  }

  Serial.println("[WIFI] Conexion establecida.");
  Serial.print("[WIFI] IP: ");
  Serial.println(WiFi.localIP());

  return true;
}

// =====================================================
// ESCAPAR TEXTO HTML
// =====================================================

String wifiEscaparHTML(const String& entrada) {
  String salida;
  salida.reserve(entrada.length() + 8);

  for (size_t i = 0; i < entrada.length(); i++) {
    char c = entrada[i];

    switch (c) {
      case '&':  salida += F("&amp;");  break;
      case '<':  salida += F("&lt;");   break;
      case '>':  salida += F("&gt;");   break;
      case '"':  salida += F("&quot;"); break;
      case '\'': salida += F("&#39;");  break;
      default:   salida += c;           break;
    }
  }

  return salida;
}

// =====================================================
// REDIRECCION AL PORTAL CAUTIVO
// =====================================================

void wifiRedirigirAlPortal() {
  servidorWiFi.sendHeader(
    "Location",
    "http://192.168.4.1/",
    true
  );

  servidorWiFi.sendHeader("Cache-Control", "no-cache");
  servidorWiFi.send(302, "text/plain", "");
}

// =====================================================
// PAGINA DE CONFIGURACION
// =====================================================

void wifiEnviarPaginaPrincipal() {
  String pagina;
  pagina.reserve(5200);

  pagina += F(
    "<!doctype html><html lang='es'><head>"
    "<meta charset='utf-8'>"
    "<meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<meta http-equiv='Cache-Control' content='no-cache'>"
    "<title>Sonnar IA - Wi-Fi</title>"
    "<style>"
    "body{font-family:Arial,sans-serif;background:#10151d;color:#f2f5f8;margin:0;padding:22px}"
    ".caja{max-width:440px;margin:20px auto;background:#1c2633;padding:22px;border-radius:18px}"
    "h1{font-size:25px;margin:0 0 8px}"
    "p{line-height:1.45;color:#c9d3df}"
    "label{display:block;margin:15px 0 6px}"
    "input,select,button{box-sizing:border-box;width:100%;padding:12px;border-radius:9px;border:1px solid #526174;font-size:16px}"
    "input,select{background:#101923;color:#fff}"
    "button{background:#bde9ff;color:#10151d;font-weight:bold;margin-top:14px;border:0}"
    ".sec{background:#354355;color:white}"
    ".estado{padding:12px;background:#101923;border-radius:9px;margin-top:14px;word-break:break-word}"
    "</style></head><body><div class='caja'>"
    "<h1>Sonnar IA</h1>"
    "<p>Configuracion de red Wi-Fi. Elegi una red e ingresa su contrasena.</p>"
    "<button type='button' class='sec' onclick='buscarRedes()'>Buscar redes Wi-Fi</button>"
    "<form id='formulario'>"
    "<label for='lista'>Redes encontradas</label>"
    "<select id='lista'><option value=''>Toca Buscar redes Wi-Fi</option></select>"
    "<label for='ssid'>Nombre de red (SSID)</label>"
    "<input id='ssid' name='ssid' maxlength='32' required autocomplete='off' placeholder='Nombre exacto de la red'>"
    "<label for='password'>Contrasena Wi-Fi</label>"
    "<input id='password' name='password' type='password' maxlength='63' autocomplete='current-password' placeholder='Contrasena de la red'>"
    "<button type='submit'>Guardar y conectar</button>"
    "</form>"
    "<hr style='border:0;border-top:1px solid #526174;margin:24px 0'>"
    "<h2 style='font-size:20px'>API de Gemini</h2>"
    "<p>Ingresa una API Key nueva para cambiar la clave utilizada en los escaneos. La clave actual nunca se muestra.</p>"
    "<form id='formApi'>"
    "<label for='apiKey'>API Key de Gemini</label>"
    "<input id='apiKey' name='apiKey' type='password' maxlength='200' autocomplete='off' placeholder='Pega aquí la nueva API Key' required>"
    "<button type='submit'>Guardar API Key</button>"
    "</form>"
    "<div class='estado' id='estadoApi'>La clave se guarda en la memoria del dispositivo.</div>"
    "<div class='estado' id='estado'>Conectate a esta pagina mientras Sonnar IA configura la red.</div>"
    "<p>Si la conexion funciona, la red quedara guardada para los proximos encendidos.</p>"
    "</div><script>"
    "const lista=document.getElementById('lista');"
    "const ssid=document.getElementById('ssid');"
    "const estado=document.getElementById('estado');"

    "async function buscarRedes(){"
    "estado.textContent='Buscando redes...';"
    "try{let r=await fetch('/scan',{cache:'no-store'});"
    "lista.innerHTML=await r.text();"
    "estado.textContent='Selecciona una red o escribe el nombre manualmente.';"
    "}catch(e){estado.textContent='No se pudieron buscar redes. Intenta otra vez.';}"
    "}"

    "lista.addEventListener('change',()=>{if(lista.value)ssid.value=lista.value;});"

    "document.getElementById('formulario').addEventListener('submit',async e=>{"
    "e.preventDefault();"
    "estado.textContent='Intentando conectar...';"
    "let datos=new URLSearchParams();"
    "datos.set('ssid',ssid.value);"
    "datos.set('password',document.getElementById('password').value);"
    "try{let r=await fetch('/save',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:datos});"
    "estado.textContent=await r.text();"
    "}catch(e){estado.textContent='Solicitud enviada. Espera y revisa el estado de conexion.';}"
    "});"

    "document.getElementById('formApi').addEventListener('submit',async e=>{"
    "e.preventDefault();"
    "const estadoApi=document.getElementById('estadoApi');"
    "const apiKey=document.getElementById('apiKey').value.trim();"
    "if(apiKey.length<20){estadoApi.textContent='La clave parece demasiado corta. Revisa el valor.';return;}"
    "estadoApi.textContent='Guardando API Key...';"
    "let datos=new URLSearchParams();datos.set('apiKey',apiKey);"
    "try{let r=await fetch('/save-api',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:datos});"
    "estadoApi.textContent=await r.text();"
    "if(r.ok)document.getElementById('apiKey').value='';"
    "}catch(e){estadoApi.textContent='No se pudo confirmar el guardado. Revisa la conexion e intenta otra vez.';}"
    "});"

    "setInterval(async()=>{"
    "try{let r=await fetch('/status',{cache:'no-store'});"
    "let t=await r.text();if(t)estado.textContent=t;"
    "}catch(e){}"
    "},1800);"

    "buscarRedes();"
    "</script></body></html>"
  );

  servidorWiFi.sendHeader("Cache-Control", "no-store");
  servidorWiFi.send(200, "text/html; charset=utf-8", pagina);
}

// =====================================================
// INICIAR PORTAL CAUTIVO
// =====================================================

bool iniciarPortalConfiguracionWiFi() {
  if (portalWiFiActivo) {
    return true;
  }

  Serial.println();
  Serial.println("[WIFI] Iniciando portal cautivo...");

  portalIntentandoConectar = false;
  portalCerrarTrasExito = false;

  wifiSSIDPendiente = "";
  wifiPasswordPendiente = "";

  estadoPortalWiFi =
    "Conectate a SonnarIA-Setup desde el celular.";

  WiFi.mode(WIFI_AP_STA);

  // Direccion fija del punto de acceso.
  IPAddress ip(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);

  if (!WiFi.softAPConfig(ip, gateway, subnet)) {
    Serial.println("[WIFI] Error configurando IP del portal.");
  }

  if (!WiFi.softAP(WIFI_AP_NOMBRE, WIFI_AP_CLAVE)) {
    Serial.println("[WIFI] ERROR iniciando punto de acceso.");
    WiFi.mode(WIFI_STA);
    return false;
  }

  // DNS: todas las consultas se dirigen al ESP32.
  servidorDNS.start(
    DNS_PORT,
    "*",
    WiFi.softAPIP()
  );

  // ---------------------------------------------------
  // PAGINA PRINCIPAL
  // ---------------------------------------------------

  servidorWiFi.on("/", HTTP_GET, []() {
    wifiEnviarPaginaPrincipal();
  });

  // ---------------------------------------------------
  // COMPROBACIONES DE PORTAL CAUTIVO
  // Android
  // ---------------------------------------------------

  servidorWiFi.on("/generate_204", HTTP_GET, []() {
    wifiRedirigirAlPortal();
  });

  servidorWiFi.on("/gen_204", HTTP_GET, []() {
    wifiRedirigirAlPortal();
  });

  servidorWiFi.on("/mobile/status.php", HTTP_GET, []() {
    wifiRedirigirAlPortal();
  });

  // Apple / iOS / macOS

  servidorWiFi.on("/hotspot-detect.html", HTTP_GET, []() {
    wifiRedirigirAlPortal();
  });

  servidorWiFi.on("/library/test/success.html", HTTP_GET, []() {
    wifiRedirigirAlPortal();
  });

  servidorWiFi.on("/success.txt", HTTP_GET, []() {
    wifiRedirigirAlPortal();
  });

  // Windows

  servidorWiFi.on("/connecttest.txt", HTTP_GET, []() {
    wifiRedirigirAlPortal();
  });

  servidorWiFi.on("/ncsi.txt", HTTP_GET, []() {
    wifiRedirigirAlPortal();
  });

  servidorWiFi.on("/redirect", HTTP_GET, []() {
    wifiRedirigirAlPortal();
  });

  // ---------------------------------------------------
  // BUSCAR REDES WIFI
  // ---------------------------------------------------

  servidorWiFi.on("/scan", HTTP_GET, []() {
    int cantidad = WiFi.scanNetworks(false, true);

    String opciones;
    opciones.reserve(1600);

    opciones += F("<option value=''>Selecciona una red...</option>");

    if (cantidad <= 0) {
      opciones += F("<option value=''>No se encontraron redes</option>");
    } else {
      for (int i = 0; i < cantidad; i++) {
        String nombre = WiFi.SSID(i);

        if (nombre.length() == 0) {
          continue;
        }

        String nombreSeguro = wifiEscaparHTML(nombre);

        opciones += F("<option value=\"");
        opciones += nombreSeguro;
        opciones += F("\">");
        opciones += nombreSeguro;
        opciones += F(" (");
        opciones += String(WiFi.RSSI(i));
        opciones += F(" dBm)</option>");
      }
    }

    WiFi.scanDelete();

    servidorWiFi.sendHeader("Cache-Control", "no-store");
    servidorWiFi.send(
      200,
      "text/html; charset=utf-8",
      opciones
    );
  });

  // ---------------------------------------------------
  // GUARDAR Y PROBAR CREDENCIALES
  // ---------------------------------------------------

  servidorWiFi.on("/save", HTTP_POST, []() {
    String ssid = servidorWiFi.arg("ssid");
    String password = servidorWiFi.arg("password");

    ssid.trim();

    if (
      ssid.length() == 0 ||
      ssid.length() > 32 ||
      password.length() > 63
    ) {
      servidorWiFi.send(
        400,
        "text/plain; charset=utf-8",
        "El nombre de red o la contrasena no son validos."
      );
      return;
    }

    wifiSSIDPendiente = ssid;
    wifiPasswordPendiente = password;

    portalIntentandoConectar = true;
    portalCerrarTrasExito = false;
    portalInicioIntento = millis();

    estadoPortalWiFi = "Intentando conectar a " + ssid + "...";

    WiFi.mode(WIFI_AP_STA);
    WiFi.disconnect(false, false);
    WiFi.begin(
      wifiSSIDPendiente.c_str(),
      wifiPasswordPendiente.c_str()
    );

    servidorWiFi.send(
      200,
      "text/plain; charset=utf-8",
      "Datos recibidos. Sonnar IA esta intentando conectarse. No cierres la pagina."
    );

    Serial.print("[WIFI] Probando nueva red: ");
    Serial.println(ssid);
  });

  // ---------------------------------------------------
  // GUARDAR API KEY DE GEMINI (INDEPENDIENTE DEL WIFI)
  // ---------------------------------------------------

  servidorWiFi.on("/save-api", HTTP_POST, []() {
    String nuevaClave = servidorWiFi.arg("apiKey");
    nuevaClave.trim();

    if (nuevaClave.length() < 20 || nuevaClave.length() > 200) {
      servidorWiFi.send(
        400,
        "text/plain; charset=utf-8",
        "La API Key no parece valida. Revisa que hayas copiado la clave completa."
      );
      return;
    }

    if (!guardarAPIKeyGemini(nuevaClave)) {
      servidorWiFi.send(
        500,
        "text/plain; charset=utf-8",
        "No se pudo guardar la API Key en memoria. Intenta nuevamente."
      );
      return;
    }

    servidorWiFi.send(
      200,
      "text/plain; charset=utf-8",
      "API Key guardada correctamente. Sonnar IA la usara en los proximos escaneos."
    );
  });

  // ---------------------------------------------------
  // ESTADO DE CONEXION
  // ---------------------------------------------------

  servidorWiFi.on("/status", HTTP_GET, []() {
    servidorWiFi.sendHeader("Cache-Control", "no-store");

    servidorWiFi.send(
      200,
      "text/plain; charset=utf-8",
      estadoPortalWiFi
    );
  });

  // ---------------------------------------------------
  // CUALQUIER OTRA DIRECCION -> PORTAL
  // ---------------------------------------------------

  servidorWiFi.onNotFound([]() {
    wifiRedirigirAlPortal();
  });

  servidorWiFi.begin();

  portalWiFiActivo = true;

  Serial.println("[WIFI] Portal cautivo activo.");

  Serial.print("[WIFI] Red: ");
  Serial.println(WIFI_AP_NOMBRE);

  Serial.print("[WIFI] IP: ");
  Serial.println(WiFi.softAPIP());

  Serial.println("[WIFI] Abrir: http://192.168.4.1");

  return true;
}

// =====================================================
// CERRAR PORTAL
// =====================================================

void detenerPortalConfiguracionWiFi() {
  if (!portalWiFiActivo) {
    return;
  }

  servidorWiFi.stop();
  servidorDNS.stop();

  if (portalIntentandoConectar) {
    WiFi.disconnect(false, false);
  }

  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);

  portalIntentandoConectar = false;
  portalCerrarTrasExito = false;
  portalWiFiActivo = false;

  Serial.println("[WIFI] Portal cautivo cerrado.");
}

bool wifiPortalEstaActivo() {
  return portalWiFiActivo;
}

// =====================================================
// ATENDER PORTAL SIN BLOQUEAR EL LOOP
// =====================================================

void gestionarPortalWiFi() {
  if (!portalWiFiActivo) {
    return;
  }

  // Procesar DNS para la deteccion del portal cautivo.
  servidorDNS.processNextRequest();

  // Atender solicitudes HTTP del celular.
  servidorWiFi.handleClient();

  if (portalIntentandoConectar) {

    if (WiFi.status() == WL_CONNECTED) {

      if (
        guardarCredencialesWiFi(
          wifiSSIDPendiente,
          wifiPasswordPendiente
        )
      ) {
        portalIntentandoConectar = false;
        portalCerrarTrasExito = true;

        portalCerrarEn =
          millis() + WIFI_ESPERA_CIERRE_PORTAL_MS;

        estadoPortalWiFi =
          "Conexion correcta. Red guardada; Sonnar IA se conectara automaticamente.";

        Serial.println("[WIFI] Red guardada en memoria persistente.");

        Serial.print("[WIFI] IP de la red: ");
        Serial.println(WiFi.localIP());

      } else {
        portalIntentandoConectar = false;

        estadoPortalWiFi =
          "No se pudieron guardar los datos. Intenta nuevamente.";
      }

    } else if (
      millis() - portalInicioIntento >= WIFI_TIMEOUT_CONEXION_MS
    ) {
      portalIntentandoConectar = false;

      WiFi.disconnect(false, false);

      // Mantener el punto de acceso y el DNS disponibles.
      WiFi.mode(WIFI_AP_STA);

      estadoPortalWiFi =
        "No conecto. Revisa la contrasena y vuelve a intentar.";

      Serial.println(
        "[WIFI] Tiempo agotado; las credenciales anteriores se conservaron."
      );
    }
  }

  if (
    portalCerrarTrasExito &&
    (long)(millis() - portalCerrarEn) >= 0
  ) {
    servidorWiFi.stop();
    servidorDNS.stop();

    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);

    portalWiFiActivo = false;
    portalCerrarTrasExito = false;

    Serial.println(
      "[WIFI] Portal cerrado; la red guardada ya esta conectada."
    );

    menuDibujarSistema();
  }
}
