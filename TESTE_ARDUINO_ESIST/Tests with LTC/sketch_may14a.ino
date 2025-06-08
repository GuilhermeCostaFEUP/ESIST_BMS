#include <SPI.h>
#include "LTC6811.h"
#include "LTC681x.h"
#include "LT_SPI.h"

const int TOTAL_IC = 1;
cell_asic bms_ic[TOTAL_IC];

const uint8_t ADC_CONVERSION_MODE = 0; // MD_7KHZ_1K
const uint8_t DCP = 0;
const uint8_t CELL_CH = 0; // All cells

void setup() {
  Serial.begin(9600);
  delay(100);

  spi_enable(SPI_CLOCK_DIV16); // SPI 1 MHz
  LTC6811_init_cfg(TOTAL_IC, bms_ic);

  wakeup_idle(TOTAL_IC);
  LTC6811_reset_crc_count(TOTAL_IC, bms_ic);

  wakeup_idle(TOTAL_IC);
  LTC6811_adcv(ADC_CONVERSION_MODE, DCP, CELL_CH);

  delay(10); // 10ms para garantir conversão completa
}

void loop() {
  wakeup_idle(TOTAL_IC);

  // Ler todos os grupos de células: 0=A,1=B,2=C,3=D
  for (int group = 0; group < 4; group++) {
    int8_t error = LTC6811_rdcv(group, TOTAL_IC, bms_ic);
    if (error < 0) {
      Serial.print("Erro na leitura do grupo ");
      Serial.println(group);
      return;
    }
  }

  // Agora as tensões de todas as 12 células estão em bms_ic[0].cells.c_codes[0..11]
  // Imprime células 0 a 6 (ou seja, 7 células)
  for (int i = 0; i <= 6; i++) {
    float voltage = bms_ic[0].cells.c_codes[i] * 0.0001;
    Serial.print("Célula ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(voltage, 4);
    Serial.println(" V");
  }

  delay(5000);
}
