// Modulo separado de Sonnar IA v1.2. No cambiar logica.

void capturarYEnviarAGemini() {
  sistemaEscaneando =
    true;
  // ===================================================
  // RESET DE TIEMPOS
  // ===================================================
  tiempoInicioEscaneo =
    millis();
  tiempoFinCaptura =
    0;
  tiempoFinBase64 =
    0;
  tiempoInicioGemini =
    0;
  tiempoFinGemini =
    0;
  tiempoInicioTTS =
    0;
  tiempoFinTTSConexion =
    0;
  Serial.println();
  Serial.println(
    "Iniciando captura..."
  );
  // ===================================================
  // CAPTURA
  // ===================================================
  uint8_t *imagen =
    NULL;
  size_t tamanoImagen =
    0;
  if (
    !capturarImagen(
      &imagen,
      &tamanoImagen
    )
  ) {
    sistemaEscaneando =
      false;
    ultimoAviso =
      millis();
    return;
  }
  tiempoFinCaptura =
    millis();
  // ===================================================
  // BASE64
  // ===================================================
  String imagenBase64 =
    convertirBase64(
      imagen,
      tamanoImagen
    );
  tiempoFinBase64 =
    millis();
  // Ya no necesitamos la imagen original.
  free(
    imagen
  );
  imagen =
    NULL;
  if (
    imagenBase64.length() == 0
  ) {
    sistemaEscaneando =
      false;
    ultimoAviso =
      millis();
    return;
  }
  // ===================================================
  // GEMINI
  // ===================================================
  String descripcion =
    analizarConGemini(
      imagenBase64
    );
  // Liberar inmediatamente la enorme String Base64.
  imagenBase64 = "";
  Serial.println();
  Serial.println(
    "Memoria liberada."
  );
  // ===================================================
  // RESULTADO
  // ===================================================
  if (
    descripcion.length() > 0
  ) {
    Serial.println();
    Serial.println(
      "Respuesta lista para TTS."
    );
    // =================================================
    // TTS INMEDIATO
    // =================================================
    hablarTexto(
      descripcion
    );
  } else {
    Serial.println();
    Serial.println(
      "No se obtuvo descripcion."
    );
  }
  // ===================================================
  // LATENCIA
  // ===================================================
  imprimirLatenciaFinal();
  ultimoAviso =
    millis();
  sistemaEscaneando =
    false;
  Serial.println();
  Serial.println(
    "================================"
  );
  Serial.println(
    "    Esperando nueva pulsacion..."
  );
  Serial.println(
    "================================"
  );
}
// =====================================================
// SETUP
// =====================================================
