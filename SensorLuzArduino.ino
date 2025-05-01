/*
 * Controle de Relé com Sensor de Luz e Alternância de Modo (Automático/Manual)
 *
 * Este código permite:
 * - Acionar um relé conforme a luminosidade ambiente (modo automático)
 * - Alternar entre modo automático e manual por meio de um botão
 * - No modo manual, o botão alterna o estado do relé a cada clique
 * - No modo automático, o relé liga ou desliga conforme o valor lido do sensor de luz (LDR)
 * - Sistema de debounce implementado via millis() para estabilidade na leitura do botão
 *
 * Conexões:
 * - Sensor de Luz (LDR): Ligado ao pino analógico A0
 * - Relé: Pino digital 8
 * - Botão: Entre pino digital 2 e GND (com INPUT_PULLUP)
 *
 * Ajuste:
 * - LIMIAR_LUZ: valor de luminosidade para acionar o relé no modo automático
 */

// Definições dos pinos
const int PIN_SENSOR_LUZ = A0;
const int PIN_RELE       = 8;
const int PIN_BOTAO      = 2;

// Configuração do limiar de luz (ajuste conforme necessário)
const int LIMIAR_LUZ = 700;

// Controle de tempo para debounce
const unsigned long TEMPO_DEBOUNCE = 200;
unsigned long ultimoTempoBotao = 0;

// Variáveis de controle
bool modoAutomatico     = true;
bool estadoRele         = false;
bool ultimoEstadoBotao  = HIGH;

void setup() {
  pinMode(PIN_SENSOR_LUZ, INPUT);
  pinMode(PIN_RELE, OUTPUT);
  pinMode(PIN_BOTAO, INPUT_PULLUP);

  digitalWrite(PIN_RELE, LOW);  // Relé começa desligado

  Serial.begin(9600);
  Serial.println("Sistema iniciado.");
}

void loop() {
  // Leitura do estado atual do botão
  bool estadoAtualBotao = digitalRead(PIN_BOTAO);
  unsigned long tempoAtual = millis();

  // Detecta borda de descida (pressionado) com debounce
  if (estadoAtualBotao == LOW && ultimoEstadoBotao == HIGH && (tempoAtual - ultimoTempoBotao) > TEMPO_DEBOUNCE) {
    ultimoTempoBotao = tempoAtual;
    modoAutomatico = !modoAutomatico;

    if (modoAutomatico) {
      Serial.println("Modo AUTOMÁTICO ativado.");
    } else {
      // No modo manual, alterna o estado do relé ao mudar para manual
      estadoRele = !estadoRele;
      digitalWrite(PIN_RELE, estadoRele ? HIGH : LOW);
      Serial.print("Relé ");
      Serial.println(estadoRele ? "ligado (manual)." : "desligado (manual).");
    }
  }

  // Atualiza último estado do botão
  ultimoEstadoBotao = estadoAtualBotao;

  // Controle automático
  if (modoAutomatico) {
    int valorLuz = analogRead(PIN_SENSOR_LUZ);
    Serial.print("Valor da luz: ");
    Serial.println(valorLuz);

    if (valorLuz > LIMIAR_LUZ) {
      digitalWrite(PIN_RELE, LOW);  // Desliga relé
      Serial.println("Relé desligado por luz suficiente (automático).");
    } else {
      digitalWrite(PIN_RELE, HIGH); // Liga relé
      Serial.println("Relé ligado por pouca luz (automático).");
    }
  }

  // Pequeno atraso só para estabilidade de leitura Serial
  delay(100);
}