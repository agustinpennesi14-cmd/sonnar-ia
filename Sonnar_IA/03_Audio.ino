// Modulo separado de Sonnar IA v1.2. No cambiar logica.

void hablarTexto(
  const String &texto
) {
  if (
    texto.length() == 0
  ) {
    Serial.println(
      "TTS: texto vacio."
    );
    return;
  }
  Serial.println();
  Serial.println("================================");
  Serial.println("             TTS");
  Serial.println("================================");
  textoTTSActual =
    texto;
  Serial.print(
    "Texto: "
  );
  Serial.println(
    textoTTSActual
  );
  // Si hubiera audio anterior,
  // lo detenemos inmediatamente.
  if (
    audio.isRunning()
  ) {
    Serial.println(
      "Deteniendo audio anterior..."
    );
    audio.stopSong();
  }
  Serial.println(
    "Conectando con Google TTS..."
  );
  tiempoInicioTTS =
    millis();
  bool resultado =
    audio.connecttospeech(
      textoTTSActual.c_str(),
      "es"
    );
  tiempoFinTTSConexion =
    millis();
  Serial.print(
    "Tiempo conexion TTS: "
  );
  Serial.print(
    tiempoFinTTSConexion -
    tiempoInicioTTS
  );
  Serial.println(" ms");
  if (resultado) {
    Serial.println(
      "Google TTS conectado."
    );
    Serial.println(
      "Reproduciendo..."
    );
  } else {
    Serial.println(
      "ERROR: No se pudo conectar con Google TTS."
    );
  }
}
// =====================================================
// AVISO
// =====================================================

void anunciarEspera() {
  if (
    sistemaEscaneando
  ) {
    return;
  }
  if (
    audio.isRunning()
  ) {
    return;
  }
  hablarTexto(
    "Presione el boton para comenzar."
  );
}
// =====================================================
// INICIALIZAR CAMARA
// =====================================================

void iniciarAudio() {
  Serial.println();
  Serial.println("================================");
  Serial.println("         INICIALIZANDO AUDIO");
  Serial.println("================================");
  mostrarMemoria(
    "ANTES DE AUDIO"
  );
  audio.setPinout(
    I2S_BCLK,
    I2S_LRC,
    I2S_DOUT
  );
  audio.setVolume(
    20
  );
  Serial.println(
    "MAX98357A listo."
  );
  mostrarMemoria(
    "DESPUES DE AUDIO"
  );
}
// =====================================================
// WIFI
// =====================================================
