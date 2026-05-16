/* ============================================================
   TABLERO ASCENSOR  -  Arduino Nano
   ------------------------------------------------------------
   3 digitos 7-segmentos:  A  B  C   (tiras LED rojas, 12V)

   8 canales de luz:
     Canal 1  -> compuesto "3+1": prende a la vez el "3" en A,
                 el "+" en B y el "1" en C.
     Canal 2..8 -> los 7 segmentos del digito B (el unico que
                 se controla segmento por segmento).

   Secuencia que recorre el boton: [ 1, 2, 3, 3+1, 5 ]
   Un toque del boton avanza al siguiente. Despues del 5
   vuelve al 1 (loop infinito).

   Cada canal va a un MOSFET canal-N (o modulo rele) que
   conmuta los 12V de la tira. Pin en HIGH = canal encendido.
   ============================================================ */

/* ---------- MAPEO DE CANALES A PINES DEL NANO ----------
   Cambia estos numeros si tu cableado es distinto.
   Canal 1 = compuesto "3+1".
   Canales 2..8 = segmentos a,b,c,d,e,f,g del digito B.

       seg del digito B:
            aaa
           f   b
           f   b
            ggg
           e   c
           e   c
            ddd
*/
const uint8_t PIN_CH1_MAS  = 2;   // Canal 1: compuesto "3 + 1"  (IN1)
const uint8_t PIN_B_SEG_A  = 3;   // Canal 2: segmento a de B    (IN2)
const uint8_t PIN_B_SEG_B  = 4;   // Canal 3: segmento b de B    (IN3)
const uint8_t PIN_B_SEG_C  = 5;   // Canal 4: segmento c de B    (IN4)
const uint8_t PIN_B_SEG_D  = 6;   // Canal 5: segmento d de B    (IN5)
const uint8_t PIN_B_SEG_E  = 7;   // Canal 6: segmento e de B    (IN6)
const uint8_t PIN_B_SEG_F  = 8;   // Canal 7: segmento f de B    (IN7)
const uint8_t PIN_B_SEG_G  = 9;   // Canal 8: segmento g de B    (IN8)

const uint8_t PIN_BOTON    = 10;  // Boton entre este pin y GND

// Orden fijo de los 8 canales para las tablas de abajo:
// indice 0 = canal 1 (compuesto), 1..7 = segmentos a..g de B
const uint8_t PINES[8] = {
  PIN_CH1_MAS,
  PIN_B_SEG_A, PIN_B_SEG_B, PIN_B_SEG_C, PIN_B_SEG_D,
  PIN_B_SEG_E, PIN_B_SEG_F, PIN_B_SEG_G
};

/* ---------- SI TUS MOSFET/RELE SON INVERTIDOS ----------
   MOSFET canal-N comun: pin HIGH = encendido  -> deja false.
   Muchos modulos rele son "active LOW": pin LOW = encendido
   -> pone esto en true.                                      */
const bool CANAL_INVERTIDO = true;   // modulo rele 8 canales = active LOW

/* ---------- TABLA DE LA SECUENCIA ----------
   Cada fila son los 8 canales (1 = encendido, 0 = apagado),
   en el orden:  {3+1 , a , b , c , d , e , f , g}

   Digito B segun cada numero:
     "1" -> b,c
     "2" -> a,b,g,e,d
     "3" -> a,b,g,c,d
     "5" -> a,f,g,c,d
   "3+1" -> solo canal compuesto, B en blanco.                */
const uint8_t SECUENCIA[][8] = {
  /* 3+1  a  b  c  d  e  f  g  */
  {  0,   0, 1, 1, 0, 0, 0, 0 },  // paso 0:  "1"
  {  0,   1, 1, 0, 1, 1, 0, 1 },  // paso 1:  "2"
  {  0,   1, 1, 1, 1, 0, 0, 1 },  // paso 2:  "3"
  {  1,   0, 0, 0, 0, 0, 0, 0 },  // paso 3:  "3+1"
  {  0,   1, 0, 1, 1, 0, 1, 1 },  // paso 4:  "5"
};
const uint8_t TOTAL_PASOS = sizeof(SECUENCIA) / sizeof(SECUENCIA[0]);

// ---------- ESTADO ----------
uint8_t pasoActual = 0;

// Debounce del boton
const unsigned long DEBOUNCE_MS = 30;
int  lecturaEstable   = HIGH;   // con INPUT_PULLUP, suelto = HIGH
int  lecturaPrevia    = HIGH;
unsigned long tCambio = 0;

void mostrarPaso(uint8_t paso) {
  for (uint8_t c = 0; c < 8; c++) {
    bool encendido = SECUENCIA[paso][c];
    if (CANAL_INVERTIDO) encendido = !encendido;
    digitalWrite(PINES[c], encendido ? HIGH : LOW);
  }
}

void setup() {
  // Dejar cada canal en "apagado" ANTES de habilitarlo como salida,
  // asi los reles no se activan todos juntos durante el arranque.
  const uint8_t NIVEL_APAGADO = CANAL_INVERTIDO ? HIGH : LOW;
  for (uint8_t c = 0; c < 8; c++) {
    digitalWrite(PINES[c], NIVEL_APAGADO);
    pinMode(PINES[c], OUTPUT);
  }
  pinMode(PIN_BOTON, INPUT_PULLUP);

  mostrarPaso(pasoActual);   // arranca mostrando el "1"
}

void loop() {
  int lectura = digitalRead(PIN_BOTON);

  // Reinicia el contador de debounce si la lectura cambio
  if (lectura != lecturaPrevia) {
    tCambio = millis();
  }

  // Si la lectura se mantuvo estable el tiempo suficiente...
  if ((millis() - tCambio) > DEBOUNCE_MS) {
    if (lectura != lecturaEstable) {
      lecturaEstable = lectura;

      // Flanco de bajada = boton recien apretado
      if (lecturaEstable == LOW) {
        pasoActual = (pasoActual + 1) % TOTAL_PASOS;
        mostrarPaso(pasoActual);
      }
    }
  }

  lecturaPrevia = lectura;
}
