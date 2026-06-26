#include <SPI.h>
#include <PN532_SPI.h>
#include <PN532.h>
#include <sha256.h>

#define SS_PIN 10
#define LED_PIN 4
#define RELAY_PIN 7

PN532_SPI pn532spi(SPI, SS_PIN);
PN532 nfc(pn532spi);

// Chave secreta compartilhada (32 bytes) — deve ser igual no app Android
const uint8_t SECRET_KEY[] = {
  0x49, 0x46, 0x54, 0x4D, 0x4E, 0x46, 0x43, 0x4C,
  0x4F, 0x43, 0x4B, 0x32, 0x30, 0x32, 0x35, 0x00,
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
  0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10
};

// AID do projeto
uint8_t SELECT_AID[] = {
  0x00, 0xA4, 0x04, 0x00,
  0x07,
  0xF0, 0x49, 0x46, 0x54, 0x4D, 0x01, 0x01,
  0x00
};

// APDU customizado: INS=0x20 = CHALLENGE
// INS=0x30 = VERIFY HMAC
#define INS_CHALLENGE 0x20
#define INS_VERIFY 0x30

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);  // relé inativo (ativo em LOW)
  Serial.begin(115200);
  // pinMode(LED_PIN, OUTPUT);
  // digitalWrite(LED_PIN, LOW);

  // Seed do gerador aleatório com pino flutuante
  randomSeed(analogRead(A0));

  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.println("PN532 nao encontrado!");
    while (1)
      ;
  }
  nfc.SAMConfig();
  Serial.println("Sistema pronto. Aguardando celular...");
  Serial.println("---");
}

// Onde antes estava digitalWrite(LED_BUILTIN, HIGH):
void unlockDoor() {
  Serial.println(">> RELAY LOW");
  digitalWrite(RELAY_PIN, LOW);
  delay(3000);
  digitalWrite(RELAY_PIN, HIGH);
  Serial.println(">> RELAY HIGH");
}

void piscarLED(int vezes, int intervalo) {
  for (int i = 0; i < vezes; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(intervalo);
    digitalWrite(LED_PIN, LOW);
    delay(intervalo);
  }
}

bool comparaHMAC(uint8_t *a, uint8_t *b, uint8_t len) {
  uint8_t diff = 0;
  for (uint8_t i = 0; i < len; i++) diff |= (a[i] ^ b[i]);
  return diff == 0;
}

void loop() {
  bool success;
  uint8_t response[64];
  uint8_t responseLength = 64;

  Serial.println("Aguardando dispositivo...");

  success = nfc.inListPassiveTarget();
  if (!success) {
    delay(500);
    return;
  }

  Serial.println("Dispositivo detectado!");

  // Passo 1: SELECT AID
  responseLength = 64;
  success = nfc.inDataExchange(
    SELECT_AID, sizeof(SELECT_AID),
    response, &responseLength);

  if (!success || responseLength < 2 || response[responseLength - 2] != 0x90 || response[responseLength - 1] != 0x00) {
    Serial.println("SELECT AID falhou - nao e HCE IFTM.");
    piscarLED(1, 200);
    delay(1000);
    return;
  }
  Serial.println("SELECT AID OK");

  // Passo 2: Gera nonce de 8 bytes
  uint8_t nonce[8];
  for (uint8_t i = 0; i < 8; i++) nonce[i] = random(0, 256);

  Serial.print("Nonce: ");
  for (uint8_t i = 0; i < 8; i++) {
    if (nonce[i] < 0x10) Serial.print("0");
    Serial.print(nonce[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  // Passo 3: Envia CHALLENGE com o nonce
  uint8_t challengeApdu[13];
  challengeApdu[0] = 0x00;           // CLA
  challengeApdu[1] = INS_CHALLENGE;  // INS
  challengeApdu[2] = 0x00;           // P1
  challengeApdu[3] = 0x00;           // P2
  challengeApdu[4] = 0x08;           // Lc = 8 bytes
  memcpy(&challengeApdu[5], nonce, 8);
  challengeApdu[13 - 1] = 0x00;  // Le

  // Aguarda o celular — 8 + Le + headers = 13 bytes mas Le no final
  uint8_t challengeFull[] = {
    0x00, INS_CHALLENGE, 0x00, 0x00,
    0x08,
    nonce[0], nonce[1], nonce[2], nonce[3],
    nonce[4], nonce[5], nonce[6], nonce[7],
    0x20  // Le = 32 bytes (tamanho do HMAC)
  };

  responseLength = 64;
  success = nfc.inDataExchange(
    challengeFull, sizeof(challengeFull),
    response, &responseLength);

  if (!success || responseLength < 34) {
    Serial.println("CHALLENGE falhou - celular nao respondeu.");
    piscarLED(2, 200);
    delay(1000);
    return;
  }

  // Passo 4: Calcula HMAC esperado localmente
  uint8_t *hmacRecebido = response;  // primeiros 32 bytes
  // SW1 SW2 = response[32] e response[33]

  if (response[responseLength - 2] != 0x90 || response[responseLength - 1] != 0x00) {
    Serial.println("Celular retornou erro no CHALLENGE.");
    piscarLED(2, 200);
    delay(1000);
    return;
  }

  // Calcula HMAC esperado
  Sha256.initHmac(SECRET_KEY, sizeof(SECRET_KEY));
  Sha256.write(nonce, 8);
  uint8_t *hmacEsperado = Sha256.resultHmac();

  Serial.print("HMAC recebido:  ");
  for (uint8_t i = 0; i < 32; i++) {
    if (response[i] < 0x10) Serial.print("0");
    Serial.print(response[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  Serial.print("HMAC esperado:  ");
  for (uint8_t i = 0; i < 32; i++) {
    if (hmacEsperado[i] < 0x10) Serial.print("0");
    Serial.print(hmacEsperado[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  // Passo 5: Compara HMACs
  if (comparaHMAC(response, hmacEsperado, 32)) {
    Serial.println(">> AUTENTICADO! Abrindo fechadura...");
    unlockDoor();
    // piscarLED(3, 100);
    // digitalWrite(LED_PIN, HIGH);
    // delay(3000);
    // digitalWrite(LED_PIN, LOW);
  } else {
    Serial.println(">> HMAC INVALIDO - Acesso negado!");
    piscarLED(5, 80);
  }

  Serial.println("---");
  delay(1000);
}