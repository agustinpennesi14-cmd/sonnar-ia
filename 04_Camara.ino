// Modulo separado de Sonnar IA v1.2. No cambiar logica.

bool iniciarCamara() {
  Serial.println();
  Serial.println("================================");
  Serial.println("        INICIALIZANDO CAMARA");
  Serial.println("================================");
  mostrarMemoria(
    "ANTES DE CAMARA"
  );
  if (
    psramFound()
  ) {
    Serial.println(
      "PSRAM DETECTADA."
    );
  } else {
    Serial.println(
      "PSRAM NO DETECTADA."
    );
  }
  camera_config_t config = {};
  config.ledc_channel =
    LEDC_CHANNEL_0;
  config.ledc_timer =
    LEDC_TIMER_0;
  config.pin_d0 =
    Y2_GPIO_NUM;
  config.pin_d1 =
    Y3_GPIO_NUM;
  config.pin_d2 =
    Y4_GPIO_NUM;
  config.pin_d3 =
    Y5_GPIO_NUM;
  config.pin_d4 =
    Y6_GPIO_NUM;
  config.pin_d5 =
    Y7_GPIO_NUM;
  config.pin_d6 =
    Y8_GPIO_NUM;
  config.pin_d7 =
    Y9_GPIO_NUM;
  config.pin_xclk =
    XCLK_GPIO_NUM;
  config.pin_pclk =
    PCLK_GPIO_NUM;
  config.pin_vsync =
    VSYNC_GPIO_NUM;
  config.pin_href =
    HREF_GPIO_NUM;
  config.pin_sccb_sda =
    SIOD_GPIO_NUM;
  config.pin_sccb_scl =
    SIOC_GPIO_NUM;
  config.pin_pwdn =
    PWDN_GPIO_NUM;
  config.pin_reset =
    RESET_GPIO_NUM;
  config.xclk_freq_hz =
    20000000;
  config.pixel_format =
    PIXFORMAT_JPEG;
  // ===================================================
  // RESOLUCION
  // ===================================================
  config.frame_size =
    FRAMESIZE_QVGA;
  // ===================================================
  // CALIDAD JPEG
  // ===================================================
  config.jpeg_quality =
    15;
  // ===================================================
  // UN SOLO FRAMEBUFFER
  // ===================================================

if (psramFound()) {
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.fb_count = 2;
  config.grab_mode = CAMERA_GRAB_LATEST;

  Serial.println("Framebuffer: PSRAM");
  Serial.println("Búferes: 2");
  Serial.println("Modo: último fotograma disponible");
} else {
  config.fb_location = CAMERA_FB_IN_DRAM;
  config.fb_count = 1;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

  Serial.println("Framebuffer: DRAM");
  Serial.println("Búferes: 1");
  Serial.println("Modo: búfer vacío");
}

  Serial.println();
  Serial.println(
    "Configuracion de camara:"
  );
  Serial.println(
    "Resolucion: QVGA 320x240"
  );
  Serial.println(
    "Formato: JPEG"
  );
  Serial.println(
    "Calidad JPEG: 15"
  );
  Serial.println(
    "Framebuffers: 1"
  );
  Serial.println();
  Serial.println(
    "Ejecutando esp_camera_init()..."
  );
  esp_err_t err =
    esp_camera_init(
      &config
    );
  if (
    err != ESP_OK
  ) {
    Serial.print(
      "ERROR CAMARA. Codigo: 0x"
    );
    Serial.println(
      err,
      HEX
    );
    mostrarMemoria(
      "DESPUES DEL ERROR"
    );
    return false;
  }
  Serial.println();
  Serial.println(
    "CAMARA INICIALIZADA CORRECTAMENTE."
  );
  sensor_t *sensor =
    esp_camera_sensor_get();
  if (
    sensor != NULL
  ) {
    sensor->set_framesize(
      sensor,
      FRAMESIZE_QVGA
    );
    Serial.println(
      "Sensor configurado en QVGA."
    );
  }
  mostrarMemoria(
    "DESPUES DE CAMARA"
  );
  return true;
}
// =====================================================
// INICIALIZAR AUDIO
// =====================================================

bool capturarImagen(
  uint8_t **imagen,
  size_t *tamano
) {
  Serial.println();
  Serial.println(
    "Capturando imagen..."
  );
  camera_fb_t *fb =
    esp_camera_fb_get();
  if (
    fb == NULL
  ) {
    Serial.println(
      "ERROR: No se pudo capturar imagen."
    );
    return false;
  }
  Serial.println(
    "CAPTURA EXITOSA"
  );
  Serial.print(
    "Ancho: "
  );
  Serial.println(
    fb->width
  );
  Serial.print(
    "Alto: "
  );
  Serial.println(
    fb->height
  );
  Serial.print(
    "JPEG: "
  );
  Serial.print(
    fb->len
  );
  Serial.println(
    " bytes"
  );
  // Copiamos el frame antes de devolver
  // el framebuffer a la cámara.
  *imagen =
    (uint8_t*)malloc(
      fb->len
    );
  if (
    *imagen == NULL
  ) {
    Serial.println(
      "ERROR: No hay memoria para copiar imagen."
    );
    esp_camera_fb_return(
      fb
    );
    return false;
  }
  *tamano =
    fb->len;
  memcpy(
    *imagen,
    fb->buf,
    fb->len
  );
  esp_camera_fb_return(
    fb
  );
  Serial.println(
    "Frame de camara liberado."
  );
  return true;
}
// =====================================================
// BASE64
// =====================================================
