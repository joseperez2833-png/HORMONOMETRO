// =========================================================================
// PROYECTO: HORMONOMETRO SIMULADOR (1-20 mUI/ml) con OLED y CONTROL DE FLUJO
// FUNCIONALIDAD: Animación de Bienvenida -> Botón (Iniciar/Pausar/Reiniciar)
// =========================================================================

// --- 1. LIBRERÍAS ---
#include <Wire.h>          
#include <Adafruit_GFX.h>  
#include <Adafruit_SSD1306.h> 

// --- 2. DEFINICIONES DE HARDWARE Y DIRECCIÓN I2C ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1      
#define SCREEN_ADDRESS 0x3C // Dirección I2C típica

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- 3. CONFIGURACIÓN DEL BOTÓN Y PINES ---
#define PIN_BOTON 2 // Pin digital D2 para el botón de control

// --- 4. VARIABLES DE ESTADO Y LÓGICA DE CONTROL ---
enum Estado {
  INICIO,    // Estado inicial de bienvenida
  APAGADO,   // Contador en 0 o valor inicial, esperando para iniciar
  CONTANDO,  // Contador en marcha (simulando medición)
  PAUSADO    // Contador detenido
};

Estado estadoActual = INICIO; // Empezamos en el nuevo estado de INICIO
int valor_medicion = 0;       // Inicia en 0 (esperando medición)
const int VALOR_MAXIMO = 20;  // El conteo va de 1 a 20

unsigned long tiempoAnterior = 0;
const long intervalo = 1000; // Intervalo de 1 segundo (1000 ms)

// Variables para manejo del botón (debounce)
int estadoBotonActual = 0;
int estadoBotonAnterior = 0;

// Prototipo de la función de actualización
void actualizarPantalla(); 
void mostrarAnimacionInicial(); // Prototipo de la nueva función

// =========================================================================
// FUNCIÓN SETUP: Inicialización
// =========================================================================
void setup() {
  Serial.begin(9600);

  // Configurar el pin del botón como entrada con pullup interno
  pinMode(PIN_BOTON, INPUT_PULLUP);

  // Inicializar la pantalla OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("Error: No se encontró la pantalla OLED."));
    for (;;);
  }

  // Ejecutar la animación inicial
  mostrarAnimacionInicial(); 
}

// =========================================================================
// FUNCIÓN LOOP: Bucle Principal
// =========================================================================
void loop() {
  // --- A. Lectura del botón y cambio de estado (Máquina de estados) ---
  estadoBotonActual = digitalRead(PIN_BOTON);

  // Detectar flanco de bajada (de HIGH a LOW)
  if (estadoBotonActual != estadoBotonAnterior && estadoBotonActual == LOW) {
    
    // Cambiar estado en cada pulsación
    switch (estadoActual) {
      case APAGADO:
        // Si estaba APAGADO, INICIAR la medición
        estadoActual = CONTANDO;
        valor_medicion = 1; // La medición/conteo inicia en 1
        tiempoAnterior = millis(); // Reinicia el tiempo de referencia
        break;
        
      case CONTANDO:
        // Si estaba CONTANDO, PAUSAR
        estadoActual = PAUSADO;
        break;
        
      case PAUSADO:
        // Si estaba PAUSADO, REANUDAR
        estadoActual = CONTANDO;
        tiempoAnterior = millis(); // Ajustar el tiempo para el próximo tic
        break;
        
      // El estado INICIO se maneja solo en la función mostrarAnimacionInicial
      case INICIO:
        // Si por alguna razón se presiona durante INICIO, forzar el paso a APAGADO
        estadoActual = APAGADO;
        valor_medicion = 0;
        break;
    }
    actualizarPantalla(); // Actualizar inmediatamente después del cambio de estado
    delay(200); // Antirrebote (Debounce)
  }
  estadoBotonAnterior = estadoBotonActual;


  // --- B. Lógica de conteo (solo si el estado es CONTANDO) ---
  if (estadoActual == CONTANDO) {
    if (millis() - tiempoAnterior >= intervalo) {
      tiempoAnterior = millis();
      
      // Incrementar la medición/contador
      valor_medicion++;
      
      // Verificar si la medición ha llegado al límite (20)
      if (valor_medicion > VALOR_MAXIMO) {
        // Al terminar, reiniciar todo con la animación de bienvenida
        mostrarAnimacionInicial(); 
        return; // Salir del loop actual para no ejecutar más código
      }
      
      actualizarPantalla();
    }
  }
}

// =========================================================================
// FUNCIÓN DE ANIMACIÓN INICIAL Y REINICIO
// =========================================================================
void mostrarAnimacionInicial() {
  // Establecer el estado a INICIO para el mensaje
  estadoActual = INICIO;
  valor_medicion = 0;
  
  // 1. Mostrar "BIENVENIDO"
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5, 10); // Posicionar más o menos centrado
  display.println(F("BIENVENIDO"));
  display.display();
  delay(2000); // Mostrar por 2 segundos
  
  // 2. Mostrar "HORMOMETRO"
  display.clearDisplay();
  display.setTextSize(3);
  display.setCursor(0, 10);
  display.println(F("HORMONOMETRO"));
  display.display();
  delay(2000); // Mostrar por 2 segundos
  
  // 3. Transicionar al estado de espera y mostrar la pantalla inicial
  estadoActual = APAGADO;
  actualizarPantalla();
}

// =========================================================================
// FUNCIÓN AUXILIAR: Actualiza el contenido de la pantalla OLED
// =========================================================================
void actualizarPantalla() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // --- SECCIÓN 1: TÍTULO Y ESTADO ---
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(F("MEDICION (mUI/ml):"));
  
  // Mostrar el estado del flujo
  display.setCursor(90, 0);
  switch (estadoActual) {
      case CONTANDO:
        display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Invertir colores para ACTIVO
        display.print("ACTIVO");
        break;
      case PAUSADO:
        display.print("PAUSADO");
        break;
      case APAGADO:
        if (valor_medicion == VALOR_MAXIMO) {
          display.print("FINALIZADO");
        } else {
          display.print("ESPERA");
        }
        break;
      case INICIO:
        display.print("INICIO"); // Aunque en este estado nunca se llama a actualizarPantalla()
        break;
  }
  display.setTextColor(SSD1306_WHITE); // Restaurar color normal


  // --- SECCIÓN 2: VALOR ACTUAL Y UNIDAD ---
  display.setTextSize(3); 
  display.setCursor(0, 16);
  display.print(valor_medicion); 
  
  display.setTextSize(1);
  display.setCursor(55, 30); 
  display.print("mUI/ml");

  // Línea separadora
  display.drawFastHLine(0, 42, SCREEN_WIDTH, SSD1306_WHITE);


  // --- SECCIÓN 3: INTERPRETACIÓN Y MENSAJE DE BOTÓN ---
  display.setTextSize(2); 
  display.setCursor(0, 48); // Fila de mensajes

  // Lógica de Rango (similar a tu código original)
  if (valor_medicion >= 1 && valor_medicion <= 10) {
    display.println("NORMAL");
  } else if (valor_medicion >= 11 && valor_medicion <= VALOR_MAXIMO) {
    display.println("ANORMAL"); 
  } else {
    // Si valor_medicion es 0 (solo al inicio)
    display.setTextSize(1);
    display.setCursor(0, 48);
    display.println("Presiona Btn para INICIAR"); 
  }

  // Mostrar los cambios en la pantalla
  display.display();
}