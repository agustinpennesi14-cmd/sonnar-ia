// Modulo separado de Sonnar IA v1.2. No cambiar logica.

String convertirBase64(
  uint8_t *imagen,
  size_t tamano
) {
  size_t base64Len =
    4 * (
      (tamano + 2) / 3
    );
  Serial.print(
    "Base64 necesario: "
  );
  Serial.println(
    base64Len
  );
  unsigned char *base64Buffer =
    (unsigned char*)malloc(
      base64Len + 1
    );
  if (
    base64Buffer == NULL
  ) {
    Serial.println(
      "ERROR: No hay memoria para Base64."
    );
    return "";
  }
  size_t salidaLen =
    0;
  int resultado =
    mbedtls_base64_encode(
      base64Buffer,
      base64Len + 1,
      &salidaLen,
      imagen,
      tamano
    );
  if (
    resultado != 0
  ) {
    Serial.println(
      "ERROR: Fallo Base64."
    );
    free(
      base64Buffer
    );
    return "";
  }
  base64Buffer[
    salidaLen
  ] = '\0';
  String resultadoBase64 =
    String(
      (char*)base64Buffer
    );
  free(
    base64Buffer
  );
  Serial.println(
    "JPEG -> BASE64: OK"
  );
  Serial.print(
    "Caracteres Base64: "
  );
  Serial.println(
    resultadoBase64.length()
  );
  return resultadoBase64;
}
// =====================================================
// GEMINI
// =====================================================

String analizarConGemini(
  const String &imagenBase64
) {
  Serial.println();
  Serial.println("================================");
  Serial.println("         ESCANEO CON GEMINI");
  Serial.println("================================");
  // ===================================================
  // PROMPT
  // ===================================================
  String prompt =
    "Identifica el objeto principal de la imagen. "
    "Responde solamente con una frase en español "
    "de 5 a 12 palabras. "
    "Se preciso, directo y no inventes informacion.";
  // ===================================================
  // JSON
  // ===================================================
  DynamicJsonDocument requestDoc(
    12000
  );
  JsonArray contents =
    requestDoc.createNestedArray(
      "contents"
    );
  JsonObject content =
    contents.createNestedObject();
  JsonArray parts =
    content.createNestedArray(
      "parts"
    );
  JsonObject textPart =
    parts.createNestedObject();
  textPart["text"] =
    prompt;
  JsonObject imagePart =
    parts.createNestedObject();
  JsonObject inlineData =
    imagePart.createNestedObject(
      "inline_data"
    );
  inlineData["mime_type"] =
    "image/jpeg";
  inlineData["data"] =
    imagenBase64;
  String json;
  serializeJson(
    requestDoc,
    json
  );
  Serial.println();
  Serial.println(
    "JSON preparado."
  );
  Serial.print(
    "Tamano de solicitud: "
  );
  Serial.print(
    json.length()
  );
  Serial.println(
    " bytes"
  );
  mostrarMemoria(
    "ANTES DE GEMINI"
  );
  // ===================================================
  // URL
  // ===================================================
  String url =
    "https://generativelanguage.googleapis.com/"
    "v1beta/models/" +
    String(GEMINI_MODEL) +
    ":generateContent?key=" +
    String(API_KEY);
  // ===================================================
  // INICIO DE MEDICION
  // ===================================================
  tiempoInicioGemini =
    millis();
  // ===================================================
  // REINTENTOS
  // ===================================================
  for (
    int intento = 1;
    intento <= MAX_INTENTOS_GEMINI;
    intento++
  ) {
    Serial.println();
    Serial.println(
      "--------------------------------"
    );
    Serial.print(
      "GEMINI - INTENTO "
    );
    Serial.print(
      intento
    );
    Serial.println(
      " / 3"
    );
    Serial.println(
      "--------------------------------"
    );
    // -------------------------------------------------
    // Cliente HTTPS nuevo para cada intento
    // -------------------------------------------------
    WiFiClientSecure cliente;
    cliente.setInsecure();
    HTTPClient https;
    Serial.println(
      "Conectando con Gemini..."
    );
    if (
      !https.begin(
        cliente,
        url
      )
    ) {
      Serial.println(
        "ERROR: No se pudo iniciar HTTPS."
      );
      if (
        intento < MAX_INTENTOS_GEMINI
      ) {
        unsigned long espera =
          ESPERA_REINTENTO_MS *
          intento;
        Serial.print(
          "Reintentando en "
        );
        Serial.print(
          espera
        );
        Serial.println(
          " ms..."
        );
        delay(
          espera
        );
        continue;
      }
      tiempoFinGemini =
        millis();
      return "";
    }
    // -------------------------------------------------
    // TIMEOUTS
    // -------------------------------------------------
    https.setConnectTimeout(
      8000
    );
    https.setTimeout(
      12000
    );
    https.addHeader(
      "Content-Type",
      "application/json"
    );
    // -------------------------------------------------
    // POST
    // -------------------------------------------------
    Serial.println(
      "Enviando imagen a Gemini..."
    );
    unsigned long inicioRequest =
      millis();
    int httpCode =
      https.POST(
        json
      );
    unsigned long tiempoRequest =
      millis() -
      inicioRequest;
    Serial.println();
    Serial.print(
      "Tiempo de respuesta: "
    );
    Serial.print(
      tiempoRequest
    );
    Serial.println(
      " ms"
    );
    Serial.print(
      "HTTP: "
    );
    Serial.println(
      httpCode
    );
    String respuesta =
      https.getString();
    // -------------------------------------------------
    // 200 OK
    // -------------------------------------------------
    if (
      httpCode == 200
    ) {
      Serial.println();
      Serial.println(
        "GEMINI RESPONDIO CORRECTAMENTE."
      );
      Serial.println();
      Serial.println(
        "RESPUESTA DE GEMINI:"
      );
      Serial.println(
        respuesta
      );
      https.end();
      // ===============================================
      // PARSEAR RESPUESTA
      // ===============================================
      DynamicJsonDocument responseDoc(
        12000
      );
      DeserializationError error =
        deserializeJson(
          responseDoc,
          respuesta
        );
      if (
        error
      ) {
        Serial.print(
          "ERROR PARSEANDO GEMINI: "
        );
        Serial.println(
          error.c_str()
        );
        tiempoFinGemini =
          millis();
        return "";
      }
      const char* textoRespuesta =
        responseDoc
          ["candidates"]
          [0]
          ["content"]
          ["parts"]
          [0]
          ["text"];
      if (
        textoRespuesta == nullptr
      ) {
        Serial.println(
          "ERROR: Gemini no devolvio texto."
        );
        tiempoFinGemini =
          millis();
        return "";
      }
      String descripcion =
        String(
          textoRespuesta
        );
      descripcion.trim();
      Serial.println();
      Serial.println(
        "================================"
      );
      Serial.println(
        "DESCRIPCION DETECTADA"
      );
      Serial.println(
        descripcion
      );
      Serial.println(
        "================================"
      );
      tiempoFinGemini =
        millis();
      return descripcion;
    }
    // -------------------------------------------------
    // ERROR 503
    // -------------------------------------------------
    if (
      httpCode == 503
    ) {
      Serial.println();
      Serial.println(
        "GEMINI: HTTP 503 - SERVICIO NO DISPONIBLE."
      );
      Serial.println(
        "El modelo esta temporalmente ocupado."
      );
      https.end();
      if (
        intento <
        MAX_INTENTOS_GEMINI
      ) {
        unsigned long espera =
          ESPERA_REINTENTO_MS *
          intento;
        Serial.print(
          "Reintentando en "
        );
        Serial.print(
          espera
        );
        Serial.println(
          " ms..."
        );
        delay(
          espera
        );
        continue;
      }
      Serial.println();
      Serial.println(
        "Se agotaron los reintentos."
      );
      tiempoFinGemini =
        millis();
      return "";
    }
    // -------------------------------------------------
    // ERROR 429
    // -------------------------------------------------
    if (
      httpCode == 429
    ) {
      Serial.println();
      Serial.println(
        "GEMINI: HTTP 429 - DEMASIADAS SOLICITUDES."
      );
      https.end();
      if (
        intento <
        MAX_INTENTOS_GEMINI
      ) {
        unsigned long espera =
          1000UL *
          intento;
        Serial.print(
          "Reintentando en "
        );
        Serial.print(
          espera
        );
        Serial.println(
          " ms..."
        );
        delay(
          espera
        );
        continue;
      }
      Serial.println(
        "Se agotaron los reintentos."
      );
      tiempoFinGemini =
        millis();
      return "";
    }
    // -------------------------------------------------
    // OTROS ERRORES
    // -------------------------------------------------
    Serial.println();
    Serial.println(
      "GEMINI DEVOLVIO UN ERROR."
    );
    Serial.println(
      "Respuesta:"
    );
    Serial.println(
      respuesta
    );
    https.end();
    // Errores como 400, 401, 403, etc.
    // no tiene sentido repetirlos automáticamente.
    tiempoFinGemini =
      millis();
    return "";
  }
  tiempoFinGemini =
    millis();
  return "";
}
// =====================================================
// ESCANEO COMPLETO
// =====================================================
