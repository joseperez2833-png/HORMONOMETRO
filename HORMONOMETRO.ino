// ====================================================================
// PROYECTO: HORMONOMETRO SIMULADOR (1-20 mUI/ml) con OLED y BOTÓN
// ====================================================================

// 1. INCLUSIÓN DE LIBRERÍAS
#include <Wire.h>          // Para el protocolo I2C
#include <Adafruit_GFX.h>  // Librería de gráficos base (NECESITA INSTALARSE)
#include <Adafruit_SSD1306.h> // Librería para el controlador SSD1306 (NECESITA INSTALARSE)

// 2. DEFINICIONES DE HARDWARE Y DIRECCIÓN I2C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1       // Pin de Reset no usado (-1)
#define SCREEN_ADDRESS 0x3C // Dirección I2C típica (PRUEBA 0x3D si 0x3C no funciona)
#define BUTTON_PIN 2        // Pin digital D2 para el botón de reinicio

// 3. CREACIÓN DEL OBJETO DE LA PANTALLA
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// 4. VARIABLES DE CONTROL
int valor_medicion = 1;       // El contador inicia en 1 mUI/ml
unsigned long tiempoAnterior = 0;
const long intervalo = 1000;  // Intervalo de 1 segundo (1000 ms)

// Prototipo de la función de actualización
void actualizarContador(bool inicial); 


void setup() {
  Serial.begin(9600);

  // >> CONFIGURACIÓN DEL BOTÓN DE REINICIO
  // Usamos INPUT_PULLUP para evitar resistencias externas; el botón lee LOW al ser presionado.
  pinMode(BUTTON_PIN, INPUT_PULLUP); 

  // >> INICIALIZACIÓN DE LA PANTALLA OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("Error: No se encontró la pantalla OLED."));
    for (;;);
  }

  // Configuración de texto inicial (título fijo)
  display.clearDisplay(); 
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("MEDICION (mUI/ml):");
  
  // Muestra el marco inicial con mensaje de INICIANDO...
  actualizarContador(true); 
}


void loop() {
  unsigned long tiempoActual = millis();

  // 1. LÓGICA DE TEMPORIZACIÓN (Incremento automático cada 1 segundo)
  if (tiempoActual - tiempoAnterior >= intervalo) {
    tiempoAnterior = tiempoActual;
    
    valor_medicion++; 

    // Lógica de Reinicio por Límite (si el contador pasa de 20, vuelve a 1)
    if (valor_medicion > 20) {
        valor_medicion = 1; 
    }
    
    actualizarContador(false);
  }

  // 2. LÓGICA DEL BOTÓN (Reinicio Manual)
  // Si el botón está presionado (Pin D2 lee LOW)
  if (digitalRead(BUTTON_PIN) == LOW) {
      Serial.println("Reinicio Manual Activado.");
      
      valor_medicion = 1;      // Forzar el reinicio a 1
      actualizarContador(false); // Actualizar pantalla
      
      // Antirrebote (Debounce) para un solo reinicio por pulsación
      delay(200); 
  }
}


// FUNCIÓN PARA ACTUALIZAR LA PANTALLA
void actualizarContador(bool inicial) {
  
  // Limpiar solo la zona del valor y mensaje (para evitar parpadeo en el título)
  display.fillRect(0, 16, 128, 48, SSD1306_BLACK); 
  
  // --- A. MOSTRAR EL VALOR ACTUAL ---
  display.setTextSize(3); 
  display.setCursor(0, 16);
  display.print(valor_medicion); 
  
  // Imprimir la unidad (mUI/ml) en texto más pequeño al lado
  display.setTextSize(1);
  display.setCursor(55, 30); 
  display.print("mUI/ml");

  // --- B. MOSTRAR EL MENSAJE DE RANGO ---
  display.setTextSize(2); 
  display.setCursor(0, 48); // Fila de mensajes
  
  if (valor_medicion >= 1 && valor_medicion <= 10) {
    // Rango 1: 1 - 10 mUI/ml
    display.setTextColor(SSD1306_WHITE);
    display.println("NORMAL");
    Serial.println("Valor Normal");

  } else if (valor_medicion >= 11 && valor_medicion <= 20) {
    // Rango 2: 11 - 20 mUI/ml
    display.setTextColor(SSD1306_WHITE);
    display.println("ANORMAL"); 
    Serial.println("Valor Anormal");

  } else if (inicial) {
    // Mensaje que solo se muestra al inicio del setup
     display.setTextColor(SSD1306_WHITE);
     display.println("INICIANDO..."); 
  }

  // ¡IMPORTANTE! Mostrar el contenido del buffer en la pantalla física
  display.display();
}