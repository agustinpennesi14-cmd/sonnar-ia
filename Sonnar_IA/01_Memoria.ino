// Modulo separado de Sonnar IA v1.2. No cambiar logica.

void mostrarMemoria(
  const char* momento
) {
  Serial.println();
  Serial.print("[MEMORIA] ");
  Serial.println(momento);
  Serial.print(
    "Heap interno libre: "
  );
  Serial.print(
    heap_caps_get_free_size(
      MALLOC_CAP_INTERNAL
    )
  );
  Serial.println(" bytes");
  Serial.print(
    "PSRAM libre: "
  );
  Serial.print(
    heap_caps_get_free_size(
      MALLOC_CAP_SPIRAM
    )
  );
  Serial.println(" bytes");
  Serial.print(
    "Heap total libre: "
  );
  Serial.print(
    ESP.getFreeHeap()
  );
  Serial.println(" bytes");
  Serial.println();
}
// =====================================================
// TTS
// =====================================================
