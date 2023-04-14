// nRF24L01
#include <RF24.h>
#include <SPI.h>
#include <nRF24L01.h>
// utils
#include <string.h>

#include "Arduino.h"
// linked list
#include "listenc.h"
// file
#include "FS.h"
#include "SPIFFS.h"
/* VARIABLES */

// version
const uint16_t version = 100;  // 1.0.0

// equipments
#define LENGTH_RADIOADRRESS 5
#define MAX_ADDRESS 255
struct Equipments {
  unsigned char address[MAX_ADDRESS][LENGTH_RADIOADRRESS] = {0};
};
Equipments equipment;

// nRF24L01
#define CHANNEL 108
#define nRF_VCC 22
#define CE_PIN 17
#define CSN_PIN 16
// instantiate an object for the nRF24L01 transceiver
RF24 radio(CE_PIN,
           CSN_PIN);
uint8_t RxAddresses[][6] = {"2Node"};  // gateway address

// data
char myRxData[32] = {0};
char myAckPayload[32] = "OK";

// list
TLista lista;

// FreeRTOS
TaskHandle_t *TaskOnApp;
TaskHandle_t *TaskOnPro;
static SemaphoreHandle_t UpdateMutex;

// Prototypes
void TaskRunningOnProtocolCore(void *arg);
void TaskRunningOnAppCore(void *arg);

void print_AddressEquipments() {
  Serial.println("Debug Address");
  for (int i = 0; i < MAX_ADDRESS; i++) {
    for (int j = 0; j < LENGTH_RADIOADRRESS; j++) {
      Serial.printf("%x", equipment.address[i][j]);
    }
    Serial.println();
  }
}

/**
 * @brief load saved values
 *
 */
void spifss_readAdrress() {
  File file = SPIFFS.open("/address.txt");

  char inputc = 0;
  int id = 0;
  unsigned char store_adrress[LENGTH_RADIOADRRESS] = {0};
  char buffer[10] = {0};
  char buffer2[5] = {0};
  uint8_t i = 0;

  while (file.available()) {
    inputc = file.read();
    Serial.print(inputc);
    if (inputc == '\n') {
      if (sscanf(buffer, "%d,", &id) == 1) {
        Serial.printf("ID: %d", id);
      }
      char *ptr = strchr((char *)buffer, ',') + 1;

      store_adrress[0] = (unsigned char)ptr[0];
      store_adrress[1] = (unsigned char)ptr[1];
      store_adrress[2] = (unsigned char)ptr[2];
      store_adrress[3] = (unsigned char)ptr[3];
      store_adrress[4] = 0;

      memcpy(equipment.address[id - 1], store_adrress, LENGTH_RADIOADRRESS);
      memset(buffer, 0x00, 10);
      i = 0;

    } else
      buffer[i++] = inputc;
  }

  if (strlen(buffer) != 0) {
    if (sscanf(buffer, "%d,", &id) == 1) {
      Serial.printf("ID: %d", id);
    }
    char *ptr = strchr((char *)buffer, ',') + 1;

    store_adrress[0] = (unsigned char)ptr[0];
    store_adrress[1] = (unsigned char)ptr[1];
    store_adrress[2] = (unsigned char)ptr[2];
    store_adrress[3] = (unsigned char)ptr[3];
    store_adrress[4] = 0;

    memcpy(equipment.address[id - 1], store_adrress, LENGTH_RADIOADRRESS);
    memset(buffer, 0x00, 10);
  }

  file.close();
}

bool spifss_updateAdrress(uint8_t id, uint8_t *address) {
  if (id == 0) return false;

  // clear RAM
  for (int l = 0; l < MAX_ADDRESS; l++) {
    if (address[0] == equipment.address[l][0] &&
        address[1] == equipment.address[l][1] &&
        address[2] == equipment.address[l][2] &&
        address[3] == equipment.address[l][3]) {
      equipment.address[l][0] = 0;
      equipment.address[l][1] = 0;
      equipment.address[l][2] = 0;
      equipment.address[l][3] = 0;
      equipment.address[l][4] = 0;
    }
  }

  // save RAM
  equipment.address[id - 1][0] = address[0];
  equipment.address[id - 1][1] = address[1];
  equipment.address[id - 1][2] = address[2];
  equipment.address[id - 1][3] = address[3];

  // overwrite .txt
  File file = SPIFFS.open("/address.txt", FILE_WRITE);

  char id_i[4] = {0};
  sprintf(id_i, "%hhu", id);
  for (int i = 0; i < MAX_ADDRESS; i++) {
    if (equipment.address[i][0] == 0 && equipment.address[i][1] == 0 &&
        equipment.address[i][2] == 0 && equipment.address[i][3] == 0)
      continue;
    file.print(i + 1);
    file.print("\x2C");
    file.printf("%c", equipment.address[i][0]);
    file.printf("%c", equipment.address[i][1]);
    file.printf("%c", equipment.address[i][2]);
    file.printf("%c", equipment.address[i][3]);
    file.println();
  }

  file.close();
  Serial.println("save/update address exit");
  print_AddressEquipments();
  return true;
}

void spifss_INIT() {
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  spifss_readAdrress();
  print_AddressEquipments();
}

void radio_INIT() {
  if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1) {
    }  // hold in infinite loop
  }

  radio.setChannel(CHANNEL);
  radio.setDataRate(RF24_2MBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.enableAckPayload();
  radio.enableDynamicPayloads();

  // set the RX addres of the TX node into a RX pipe
  radio.openReadingPipe(1, RxAddresses[0]);  // using pipe 1
  // radio.openReadingPipe(2, RxAddresses[1]); // using pipe 2

  radio.startListening();  // put radio in RX mode

  // Debug
  // uint16_t used_chars = radio.sprintfPrettyDetails(buffer);
  // Serial.println(buffer);
}

// FREERTOS
// int incomingByte = 0;
bool updateAddress = false;
char addressUpdate[32] = {0};

void TaskRunningOnProtocolCore(void *arg) {
  uint8_t id = 0;
  uint8_t adddresx[5] = {0};

  while (1) {
    // if (Serial.available() > 0){
    //   // read the incoming byte:
    //   incomingByte = Serial.read();
    //   Serial.println(incomingByte, DEC);
    //   if (incomingByte == 49) {
    //     uint8_t adddresx[5] = {"\xFF\xFE\x30\x31"};
    //     spifss_updateAdrress(idx++, adddresx);
    //   } else if (incomingByte == 50) {
    //     uint8_t adddresx2[5] = {"\xFA\xFB\x32\x34"};
    //     spifss_updateAdrress(idx++, adddresx2);
    //   } else if (incomingByte == 51)
    //     ESP.restart();
    // }

    if (updateAddress) {  // change to queue
      if (xSemaphoreTake(UpdateMutex, portMAX_DELAY)) {
        id = addressUpdate[1];
        adddresx[0] = (unsigned char)addressUpdate[2];
        adddresx[1] = (unsigned char)addressUpdate[3];
        adddresx[2] = (unsigned char)addressUpdate[4];
        adddresx[3] = (unsigned char)addressUpdate[5];
        spifss_updateAdrress(id, adddresx);
        updateAddress = false;
        memset(adddresx, 0, 5);
        id = 0;
        xSemaphoreGive(UpdateMutex);
      }
    }

    delay(1);
  }
}

void TaskRunningOnAppCore(void *arg) {
  while (1) {
    uint8_t pipe;

    if (radio.available(&pipe)) {
      if (radio.writeAckPayload(pipe, &myAckPayload, sizeof(myAckPayload)))
        Serial.println("ACK Success");
      else {
        Serial.println("ACK Fail");
        radio.flush_tx();
        radio.writeAckPayload(pipe, &myAckPayload, sizeof(myAckPayload));
      }

      radio.read(&myRxData, sizeof(myRxData));  // get incoming payload

      /* Protocol
        myRxData[0] == '0' -> cancel help request
        myRxData[0] == '1' -> help request
        myRxData[0] == '2' -> set up address/id equipment
            -> myRxData[1] == id (1 - 255)
            -> myRxData[2] == address pt. 1
            -> myRxData[3] == address pt. 2
            -> myRxData[4] == address pt. 3
            -> myRxData[5] == address pt. 4
      */

      // debug
      Serial.printf("dataRX[0]: %x\n\r", myRxData[0]);
      Serial.printf("dataRX[1]: %x\n\r", myRxData[1]);
      Serial.printf("dataRX[2]: %x\n\r", myRxData[2]);
      Serial.printf("dataRX[3]: %x\n\r", myRxData[3]);
      Serial.printf("dataRX[4]: %x\n\r", myRxData[4]);
      Serial.printf("dataRX[5]: %x\n\r", myRxData[5]);

      // update address
      if (myRxData[0] == '2') {
        Serial.println("Setup");
        memcpy(addressUpdate, myRxData,6);
        updateAddress = true;
        continue;
      }

      int id = 0;

      if (xSemaphoreTake(UpdateMutex, portMAX_DELAY)) {
        for (int i = 0; i < 255; i++) {
          if ((unsigned char)myRxData[2] == equipment.address[i][0] &&
              (unsigned char)myRxData[3] == equipment.address[i][1] &&
              (unsigned char)myRxData[4] == equipment.address[i][2] &&
              (unsigned char)myRxData[5] == equipment.address[i][3]) {
            id = i + 1;
            break;
          }
        }
        xSemaphoreGive(UpdateMutex);
      }

      if (id == 0) continue;

      int i = 0;
      // cancel and remove help request
      if (myRxData[0] == '0') {
        Serial.print("Ajuda Máquina (cancelada): ");
        Serial.println(id);
        i = indexLista(lista, id);
        if (i != -1) {
          removeLista(lista, id);
          imprimeLista(lista);
        }

        // help request
      } else if (myRxData[0] == '1') {
        Serial.print("Ajuda Máquina: ");
        Serial.println(id);
        i = indexLista(lista, id);
        if (i == -1) {
          lista = appendLista(lista, id);
        }
        imprimeLista(lista);
      }
    }
    delay(1);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(nRF_VCC, OUTPUT);    // Configure digital pin nRF_VCC as an output (VCC radio)
  digitalWrite(nRF_VCC, HIGH);

  lista = criaLista();
  UpdateMutex = xSemaphoreCreateMutex();

  spifss_INIT();
  radio_INIT();

  Serial.println("Version: " + String(version));

  xTaskCreatePinnedToCore(TaskRunningOnAppCore, "TaskOnApp", 4096, NULL, 4,
                          TaskOnApp, APP_CPU_NUM);

  xTaskCreatePinnedToCore(TaskRunningOnProtocolCore, "TaskOnPro", 4096, NULL, 4,
                          TaskOnPro, PRO_CPU_NUM);
}

/**
 * @brief Disable -> Implemented tasks
 */
void loop() {}
