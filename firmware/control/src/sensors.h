// =============================================================================
//  Control — Lectura de los sensores
// =============================================================================
//  Tres funciones, una por tipo de sensor. Todas devuelven false si la lectura
//  no es utilizable. Las de I2C esperan a que el sensor termine de medir, así
//  que deben llamarse desde una tarea (nunca desde la tarea del ciclo).
//
//    readThermocouple()  MAX6675 por SPI   temperatura en décimas de grado
//    readSht31()         SHT31, I2C 0x44   temperatura y humedad
//    readSgp40()         SGP40, I2C 0x59   señal cruda de compuestos orgánicos (olor)
// =============================================================================
#pragma once
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

// Los sensores de Sensirion protegen cada par de bytes con este CRC-8.
inline uint8_t sensirionCrc(const uint8_t* data) {
  uint8_t crc = 0xFF;
  for (int i = 0; i < 2; i++) {
    crc ^= data[i];
    for (int bit = 0; bit < 8; bit++) crc = (crc & 0x80) ? (crc << 1) ^ 0x31 : (crc << 1);
  }
  return crc;
}

// Lee `count` palabras de 16 bits (cada una seguida de su CRC) de un sensor I2C.
inline bool readI2cWords(uint8_t address, uint16_t* words, int count) {
  if (Wire.requestFrom(address, static_cast<uint8_t>(count * 3)) != count * 3) return false;
  for (int i = 0; i < count; i++) {
    uint8_t bytes[3];
    for (uint8_t& b : bytes) b = Wire.read();
    if (sensirionCrc(bytes) != bytes[2]) return false;
    words[i] = (bytes[0] << 8) | bytes[1];
  }
  return true;
}

// -----------------------------------------------------------------------------
//  MAX6675 (termopar tipo K)
// -----------------------------------------------------------------------------
// Los 16 bits: D15 = 0, D14–D3 = temperatura × 4, D2 = 1 si el termopar está abierto.
// No leer el mismo módulo más de una vez cada 250 ms (tarda ≈ 220 ms en convertir).
inline bool readThermocouple(int csPin, int16_t& tempX10) {
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(csPin, LOW);
  const uint16_t raw = SPI.transfer16(0);
  digitalWrite(csPin, HIGH);
  SPI.endTransaction();

  if (raw == 0x0000) return false;  // módulo sin alimentar o salida en corto a GND
  if (raw & 0x8006) return false;   // bits fijos incorrectos o termopar abierto

  const int16_t quarters = raw >> 3;  // cuartos de grado
  tempX10 = quarters * 10 / 4;
  return tempX10 <= 1500;             // más de 150 °C no es creíble en este equipo
}

// -----------------------------------------------------------------------------
//  SHT31 (temperatura y humedad)
// -----------------------------------------------------------------------------
inline bool readSht31(int16_t& tempX10, uint16_t& humidityX10) {
  constexpr uint8_t ADDRESS = 0x44;

  Wire.beginTransmission(ADDRESS);
  Wire.write(0x24);  // medición única, alta repetibilidad
  Wire.write(0x00);
  if (Wire.endTransmission() != 0) return false;
  delay(20);  // la medición tarda ≈ 15 ms

  uint16_t words[2];
  if (!readI2cWords(ADDRESS, words, 2)) return false;
  tempX10 = static_cast<int32_t>(words[0]) * 1750 / 65535 - 450;  // −45 … +130 °C
  humidityX10 = static_cast<uint32_t>(words[1]) * 1000 / 65535;   // 0 … 100 %
  return true;
}

// -----------------------------------------------------------------------------
//  SGP40 (compuestos orgánicos volátiles)
// -----------------------------------------------------------------------------
// Autotest del sensor (≈ 320 ms). Llamar una vez al arrancar.
inline bool selfTestSgp40() {
  constexpr uint8_t ADDRESS = 0x59;
  Wire.beginTransmission(ADDRESS);
  Wire.write(0x28);
  Wire.write(0x0E);
  if (Wire.endTransmission() != 0) return false;
  delay(330);
  uint16_t result;
  return readI2cWords(ADDRESS, &result, 1) && result == 0xD400;
}

// Mide la señal cruda, compensada con la temperatura y la humedad del SHT31.
// Debe llamarse una vez por segundo: el algoritmo del índice de olor lo exige.
inline bool readSgp40(int16_t tempX10, uint16_t humidityX10, uint16_t& raw) {
  constexpr uint8_t ADDRESS = 0x59;
  const uint16_t humidityTicks = static_cast<uint32_t>(constrain(humidityX10, 0, 1000)) * 65535 / 1000;
  const uint16_t tempTicks = static_cast<uint32_t>(constrain(tempX10, -450, 1300) + 450) * 65535 / 1750;

  uint8_t command[8] = {0x26, 0x0F,
                        highByte(humidityTicks), lowByte(humidityTicks), 0,
                        highByte(tempTicks), lowByte(tempTicks), 0};
  command[4] = sensirionCrc(&command[2]);
  command[7] = sensirionCrc(&command[5]);

  Wire.beginTransmission(ADDRESS);
  Wire.write(command, sizeof(command));
  if (Wire.endTransmission() != 0) return false;
  delay(30);  // la medición tarda ≈ 30 ms

  return readI2cWords(ADDRESS, &raw, 1);
}
