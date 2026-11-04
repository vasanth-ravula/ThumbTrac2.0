#include "relgobluetooth.h"

BLECharacteristic lockCharacteristic;
BLECharacteristic unlockCharacteristic;
BLEDevice peripheral;

void initBLE() {
  BLE.begin();
  // Door Lock Service
  BLE.scanForUuid("00000000-0000-0000-0000-000000000000");
  while (1) {
    // check if a peripheral has been discovered
    peripheral = BLE.available();
    if (peripheral) {
      // discovered a peripheral, print out address, local name, and advertised service
      Serial.print("Found ");
      Serial.print(peripheral.address());
      Serial.print(" '");
      Serial.print(peripheral.localName());
      Serial.print("' ");
      Serial.print(peripheral.advertisedServiceUuid());
      Serial.println();

      if (peripheral.localName() != "DoorLockService") {
        continue;
      }

      // stop scanning
      BLE.stopScan();

      if (peripheral.connect()) {
        Serial.println("Connected");
      } else {
        Serial.println("Failed to connect!");
        return;
      }
      // discover peripheral attributes
      Serial.println("Discovering attributes ...");
      if (peripheral.discoverAttributes()) {
        Serial.println("Attributes discovered");
      } else {
        Serial.println("Attribute discovery failed!");
        peripheral.disconnect();
        return;
      }
      // retrieve the Lock characteristic
      lockCharacteristic = peripheral.characteristic("00000000-0000-0000-0000-000000000000");
      unlockCharacteristic = peripheral.characteristic("00000000-0000-0000-0000-000000000000");
      if (!lockCharacteristic) {
        Serial.println("Peripheral does not have lock characteristic!");
        peripheral.disconnect();
        return;
      } else if (!lockCharacteristic.canWrite()) {
        Serial.println("Peripheral does not have a writable characteristic!");
        peripheral.disconnect();
        return;
      }

      return;
    }
  }
}
void lock() {
  if (peripheral.connected()) {
    lockCharacteristic.writeValue((byte)0x01);
    delay(5000);
  } else
    Serial.println("Device not connected");
}

void unlock() {

  if (peripheral.connected()) {
    unlockCharacteristic.writeValue((byte)0x01);
    delay(5000);
  } else
    Serial.println("Device not connected");

}