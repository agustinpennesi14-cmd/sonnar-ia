// Modulo separado de Sonnar IA v1.2. No cambiar logica.

void imprimirLatenciaFinal() {
  Serial.println();
  Serial.println("================================");
  Serial.println("           LATENCIA");
  Serial.println("================================");
  if (
    tiempoInicioEscaneo &&
    tiempoFinCaptura
  ) {
    Serial.print(
      "Gatillo -> captura:          "
    );
    Serial.print(
      tiempoFinCaptura -
      tiempoInicioEscaneo
    );
    Serial.println(" ms");
  }
  if (
    tiempoFinCaptura &&
    tiempoFinBase64
  ) {
    Serial.print(
      "Captura -> Base64:           "
    );
    Serial.print(
      tiempoFinBase64 -
      tiempoFinCaptura
    );
    Serial.println(" ms");
  }
  if (
    tiempoFinBase64 &&
    tiempoFinGemini
  ) {
    Serial.print(
      "Base64 -> Gemini:            "
    );
    Serial.print(
      tiempoFinGemini -
      tiempoFinBase64
    );
    Serial.println(" ms");
  }
  if (
    tiempoInicioEscaneo &&
    tiempoFinGemini
  ) {
    Serial.print(
      "Gatillo -> respuesta Gemini: "
    );
    Serial.print(
      tiempoFinGemini -
      tiempoInicioEscaneo
    );
    Serial.println(" ms");
  }
  if (
    tiempoFinGemini &&
    tiempoInicioTTS
  ) {
    Serial.print(
      "Respuesta -> TTS:            "
    );
    Serial.print(
      tiempoInicioTTS -
      tiempoFinGemini
    );
    Serial.println(" ms");
  }
  if (
    tiempoInicioTTS &&
    tiempoFinTTSConexion
  ) {
    Serial.print(
      "Conexion TTS:                "
    );
    Serial.print(
      tiempoFinTTSConexion -
      tiempoInicioTTS
    );
    Serial.println(" ms");
  }
  if (
    tiempoInicioEscaneo &&
    tiempoFinTTSConexion
  ) {
    Serial.print(
      "Gatillo -> TTS:              "
    );
    Serial.print(
      tiempoFinTTSConexion -
      tiempoInicioEscaneo
    );
    Serial.println(" ms");
  }
  Serial.println(
    "================================"
  );
  Serial.println();
}
// =====================================================
// MEMORIA
// =====================================================
