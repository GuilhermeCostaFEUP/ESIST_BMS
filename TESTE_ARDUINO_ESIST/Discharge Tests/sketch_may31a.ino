const int pinSaida = 5;
const int analogPin = A3;
const float Vref = 4.38;
const int resolution = 1023;

unsigned long ultimoTempoLeitura = 0;
const unsigned long intervaloLeitura = 2000;  // 2 segundos

void setup() {
  pinMode(pinSaida, OUTPUT);
  Serial.begin(9600);
  
  Serial.println("Escreve 1 para ligar o pino 5, 0 para desligar.");
}

void loop() {
  // Verifica se há dados disponíveis na porta serial
  if (Serial.available() > 0) {
    char comando = Serial.read();

    if (comando == '1') {
      digitalWrite(pinSaida, HIGH);
      Serial.println("Pino 5 ligado (HIGH).");
    } 
    else if (comando == '0') {
      digitalWrite(pinSaida, LOW);
      Serial.println("Pino 5 desligado (LOW).");
    }
  }

  // Leitura da tensão no pino A0 a cada 2 segundos
  unsigned long tempoAtual = millis();
  if (tempoAtual - ultimoTempoLeitura >= intervaloLeitura) {
    ultimoTempoLeitura = tempoAtual;

    int raw = analogRead(analogPin);
    float tensao = (raw * Vref) / resolution;

    Serial.print("Tensão em A0: ");
    Serial.print(tensao, 3);
    Serial.println(" V");
  }
}
