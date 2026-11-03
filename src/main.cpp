// Para o projeto, utilizaremos a comunicação I²C. Logo:
#include <Wire.h>

// Importando a biblioteca do BMP:
#include <Adafruit_BMP280.h>

// Inclui a biblioteca nativa para a memória flash  do esp32
#include <Preferences.h>

#define BMP_ADDRESS_1 0x76
#define BMP_ADDRESS_2 0x77

// Declaro nosso bmp  
Adafruit_BMP280 BMP;
// Declara o objeto de memória
Preferences memoria;

float Altitude_Inicial;
float Altitude = 0.0;
float Altitude_Anterior = 0.0;
float Apogeu = 0.0;
int contador = 0;
int Contador_Pouso = 0;

bool Apogeu_Detectado = false;

const int Porta_SDA = 21, Porta_SCL = 22; 
const int Mosfet_Pin = 13;
const int LED = 16;

void setup() {
  Serial.begin(9600);

  // Define o Mosfet como OUTPUT e desliga ele no início
  pinMode(Mosfet_Pin, OUTPUT);
  digitalWrite(Mosfet_Pin, LOW);
  // Define o LED como OUTPUT
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  // --- LEITURA DO VOO ANTERIOR ---
  // Abre a pasta "foguete" em modo de apenas leitura (true)
  memoria.begin("foguete", true); 
  float ultimo_apogeu = memoria.getFloat("apogeu", 0.0);
  memoria.end();

  Serial.println("=========================================");
  Serial.print("Apogeu registrado no ultimo voo: ");
  Serial.print(ultimo_apogeu);
  Serial.println(" m");
  Serial.println("=========================================");

  // Iniciando o I²C:
  Wire.begin(Porta_SDA, Porta_SCL);

  if (BMP.begin(BMP_ADDRESS_1)) {
    Serial.println("BMP280 encontrado no endereço 0x76!");
  }
  else if (BMP.begin(BMP_ADDRESS_2)) {
    Serial.println("BMP280 encontrado no endereço 0x77!");
  }
  else {
    Serial.println("Erro: BMP280 não encontrado em 0x76 nem em 0x77!");
    while (true); // Trava o código se falhar nos dois
  }

  Serial.println("BMP inicializado.");

  // Descarta a primeira leitura e tira a média de 10 leituras para zerar a altitude
  BMP.readAltitude(1015.8);
  delay(50);
  float soma = 0;
  for (int i = 0; i < 10; i++) {
    soma += BMP.readAltitude(1015.8);
    delay(50);
  }
  Altitude_Inicial = soma / 10.0;

  Serial.print("Altitude Inicial: ");
  Serial.print(Altitude_Inicial);
  Serial.println(" m");
}

void loop() {
  Altitude_Anterior = Altitude;
  Altitude = BMP.readAltitude(1015.8) - Altitude_Inicial;

  // Registra sempre a maior altitude atingida
  if (Altitude > Apogeu) {
    Apogeu = Altitude;
  }

  // Removível: Imprimir a altitude para verificar o funcionamento
  Serial.print("Altitude: ");
  Serial.print(Altitude);
  Serial.println(" m");

  // Detecta o apogeu
  if (!Apogeu_Detectado && Altitude > 10.0) {

    if (Altitude < Altitude_Anterior) {
      contador++;
    } else {
      contador = 0;
    }

    // Condição de parada atingida: Apogeu detectado
    if (contador >= 5) {
      Apogeu_Detectado = true;

      // Aciona o paraquedas e o LED de controle
      digitalWrite(LED, HIGH);
      digitalWrite(Mosfet_Pin, HIGH);

      // Salva o Apogeu na memória Flash (modo leitura/escrita = false)
      memoria.begin("foguete", false);
      memoria.putFloat("apogeu", Apogeu);
      memoria.end();

      Serial.print("APOGEU SALVO NA FLASH: ");
      Serial.print(Apogeu);
      Serial.println(" m");

      // Mantém o MOSFET ligado por 1 segundo
      delay(1000);
      digitalWrite(Mosfet_Pin, LOW);
    }
  }

  // Delay de 50ms entre leituras
  delay(50);
}