// Para o projeto, utilizaremos a comunicação I²C. Logo:
#include <Wire.h>

// Importando a biblioteca do BMP:
#include <Adafruit_BMP280.h>

// Declaro nosso bmp  
Adafruit_BMP280 BMP;
const uint8_t Endereco_BMP = 0x76; // Depende do BMP, mudar pra Ox77 se necessário

float Altitude_Inicial;
float Altitude = 0;
float Altitude_Anterior = 0;
float Apogeu;
int contador = 0;

bool Apogeu_Detectado = false;

const int Porta_SDA = 21, Porta_SCL = 22; 
const int Mosfet_Pin = 18;
const int LED = 19;

void setup() {
  Serial.begin(115200);
  // Iniciando o I²C:
  Wire.begin(Porta_SDA, Porta_SCL);

  if (!BMP.begin(Endereco_BMP)) {
    Serial.print("BMP280 não encontrado no endereço 0x");
    Serial.println(Endereco_BMP, HEX);

    // Para nosso código por completo
    while (true) {
      delay(1000);
    }
  }

  Serial.println("BMP inicializado.");

  // Mede a altura inicial
  Altitude_Inicial = BMP.readAltitude(1015.8);

  // Define o Mosfet como OUTPUT e desliga ele no início
  pinMode(Mosfet_Pin, OUTPUT);
  digitalWrite(Mosfet_Pin, LOW);

  // Define o LED como OUTPUT
  pinMode(LED, OUTPUT);
}

void loop() {
  Altitude_Anterior = Altitude;
  Altitude = BMP.readAltitude(1015.8) - Altitude_Inicial;

  // Removível: Imprimir a altitude para verificar o funcionamento
  Serial.print("Altitude: ");
  Serial.print(Altitude);
  Serial.println(" m");

  // Aqui vamos registrar 5 quedas consecutivas para evitar falsos apogeus por ruídos do BMP
  // Além disso, vamos exigir uma altura mínima de 10 metros para isso acontecer
  if(!Apogeu_Detectado && Altitude > 10){
    if(Altitude < Altitude_Anterior){
      contador++;
    }else{
      contador = 0;
    }
  
    // Condição de parada atingida: Apogeu detectado
    if(contador >= 5){
      Apogeu_Detectado = true;
      Apogeu = Altitude_Anterior;
      digitalWrite(LED, HIGH);
      digitalWrite(Mosfet_Pin, HIGH);
    }
  }

  // Delay de 50ms entre leituras
  delay(50);
}
