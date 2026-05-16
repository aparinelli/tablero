/* ============================================================
   TEST DE CANALES  -  Mapeo rele -> segmentos
   ------------------------------------------------------------
   Sketch auxiliar (NO es el tablero final).

   Enciende UN canal del rele por vez para que puedas ver, en
   el tablero, que segmento o luz corresponde a cada canal.

   Como usarlo:
     1. Carga este sketch en el Nano.
     2. Abri el Monitor Serie (9600 baudios).
     3. Mira el tablero: cada vez que cambia el canal, anota
        que segmento se prendio.
     4. Volca lo que anotaste en la tabla SECUENCIA del sketch
        principal (tablero_ascensor.ino).

   Avanza solo cada 4 segundos. Tambien podes apretar el boton
   (D10) para avanzar al instante.

   Cableado (igual que el sketch principal):
     Rele IN1..IN8 -> D2..D9
     Boton         -> D10 a GND
   ============================================================ */

const uint8_t PINES[8] = { 2, 3, 4, 5, 6, 7, 8, 9 };  // IN1..IN8
const uint8_t PIN_BOTON = 10;

const bool CANAL_INVERTIDO = true;   // modulo rele 8 canales = active LOW

const unsigned long AUTO_MS = 4000;  // avance automatico cada 4 s

uint8_t canalActual = 0;
unsigned long tUltimoCambio = 0;

// Debounce del boton
const unsigned long DEBOUNCE_MS = 30;
int lecturaEstable = HIGH;
int lecturaPrevia  = HIGH;
unsigned long tCambioBoton = 0;

void aplicarCanal(uint8_t canal) {
  for (uint8_t c = 0; c < 8; c++) {
    bool encendido = (c == canal);
    if (CANAL_INVERTIDO) encendido = !encendido;
    digitalWrite(PINES[c], encendido ? HIGH : LOW);
  }
}

void anunciarCanal(uint8_t canal) {
  Serial.print(">> CANAL ");
  Serial.print(canal + 1);
  Serial.print("   (pin D");
  Serial.print(PINES[canal]);
  Serial.println(")   ENCENDIDO  --  mira el tablero y anota que es");
}

void avanzar() {
  canalActual = (canalActual + 1) % 8;
  aplicarCanal(canalActual);
  anunciarCanal(canalActual);
  tUltimoCambio = millis();
}

void setup() {
  Serial.begin(9600);

  // Dejar todo apagado ANTES de habilitar las salidas.
  const uint8_t NIVEL_APAGADO = CANAL_INVERTIDO ? HIGH : LOW;
  for (uint8_t c = 0; c < 8; c++) {
    digitalWrite(PINES[c], NIVEL_APAGADO);
    pinMode(PINES[c], OUTPUT);
  }
  pinMode(PIN_BOTON, INPUT_PULLUP);

  Serial.println();
  Serial.println("=== TEST DE CANALES DEL RELE ===");
  Serial.println("Cada canal se enciende de a uno. Anota que ves.");
  Serial.println("Avanza solo cada 4 s, o apreta el boton para avanzar ya.");
  Serial.println();
  Serial.println("Segmentos del digito B (para nombrarlos):");
  Serial.println("    aaa");
  Serial.println("   f   b");
  Serial.println("   f   b");
  Serial.println("    ggg");
  Serial.println("   e   c");
  Serial.println("   e   c");
  Serial.println("    ddd");
  Serial.println();

  aplicarCanal(canalActual);
  anunciarCanal(canalActual);
  tUltimoCambio = millis();
}

void loop() {
  // Avance automatico
  if (millis() - tUltimoCambio > AUTO_MS) {
    avanzar();
  }

  // Avance por boton (con debounce)
  int lectura = digitalRead(PIN_BOTON);
  if (lectura != lecturaPrevia) {
    tCambioBoton = millis();
  }
  if ((millis() - tCambioBoton) > DEBOUNCE_MS) {
    if (lectura != lecturaEstable) {
      lecturaEstable = lectura;
      if (lecturaEstable == LOW) {   // flanco de bajada = boton apretado
        avanzar();
      }
    }
  }
  lecturaPrevia = lectura;
}
