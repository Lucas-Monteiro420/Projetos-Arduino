/*
 * Cronômetro Arduino com Buzzer Ativo (5V) e Relé Oscilante
 * 
 * Este código considera que:
 * - O buzzer é do tipo ativo com conexão ao 5V
 * - O pino digital apenas controla o acionamento do buzzer
 * - O relé oscila (liga/desliga) após o término do tempo até que o sistema seja reiniciado
 * 
 * Conexões:
 * - Buzzer: Positivo ao 5V, Negativo ao pino 9 via transistor ou ao GND
 * - Botão: Entre pino 2 e GND
 * - Relé: Pino 8
 */

// Definição dos pinos
const int RELE_PIN = 8;      // Pino do relé
const int BOTAO_PIN = 2;     // Pino do botão
const int BUZZER_PIN = 9;    // Pino do buzzer (controle)

// Variáveis para controle do tempo
const unsigned long TEMPO_CRONOMETRO = 10000; // 10 segundos (use 60000 para 1 minuto)
unsigned long tempoInicio = 0;

// Estados do sistema
boolean contando = false;
boolean releAtivado = false;
boolean releOscilando = false;

// Configuração para oscilação do relé
const unsigned long TEMPO_OSCILACAO = 500; // 500ms (meio segundo) para cada ciclo liga/desliga
unsigned long ultimaMudancaRele = 0;

void setup() {
  // Configuração dos pinos
  pinMode(RELE_PIN, OUTPUT);
  pinMode(BOTAO_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Configura estados iniciais
  digitalWrite(RELE_PIN, LOW);     // Relé desligado
  digitalWrite(BUZZER_PIN, HIGH);  // Buzzer desligado (lógica invertida para buzzer ativo)
  
  // Comunicação serial
  Serial.begin(9600);
  Serial.println("Sistema iniciado. Pressione o botão para comecar.");
  
  // Bip inicial
  bipCurto();
}

void loop() {
  // Verifica o botão
  if (botaoPressionado()) {
    if (releAtivado || releOscilando) {
      // Desativa o relé se estiver ativo ou oscilando
      digitalWrite(RELE_PIN, LOW);
      releAtivado = false;
      releOscilando = false;
      contando = false;
      bipCurto();
      Serial.println("Sistema reiniciado.");
      Serial.println("Pressione o botão novamente para iniciar o cronômetro.");
    } 
    else if (!contando) {
      // Inicia o cronômetro
      tempoInicio = millis();
      contando = true;
      bipCurto();
      Serial.println("Cronômetro iniciado!");
    }
  }
  
  // Atualiza o cronômetro
  if (contando) {
    unsigned long tempoAtual = millis();
    unsigned long tempoPassado = tempoAtual - tempoInicio;
    
    // Mostra tempo restante
    static unsigned long ultimaExibicao = 0;
    if (tempoAtual - ultimaExibicao >= 1000) {
      ultimaExibicao = tempoAtual;
      int segundosRestantes = (TEMPO_CRONOMETRO - tempoPassado) / 1000;
      Serial.print("Tempo restante: ");
      Serial.print(segundosRestantes);
      Serial.println(" segundos");
      
      // Bip a cada segundo nos últimos 5 segundos
      if (segundosRestantes <= 5 && segundosRestantes > 0) {
        bipCurto();
      }
    }
    
    // Verifica se o tempo acabou
    if (tempoPassado >= TEMPO_CRONOMETRO) {
      // Inicia a oscilação do relé
      releOscilando = true;
      releAtivado = false;
      contando = false;
      
      // Inicializa o estado do relé (começa ligado)
      digitalWrite(RELE_PIN, HIGH);
      ultimaMudancaRele = millis();
      
      // Sinaliza com o buzzer
      bipAlarme();
      Serial.println("Tempo finalizado! Reinicie o sistema!");
    }
  }
  
  // Controla a oscilação do relé
  if (releOscilando) {
    unsigned long tempoAtual = millis();
    if (tempoAtual - ultimaMudancaRele >= TEMPO_OSCILACAO) {
      ultimaMudancaRele = tempoAtual;
      // Inverte o estado do relé
      if (digitalRead(RELE_PIN) == HIGH) {
        digitalWrite(RELE_PIN, LOW);
        Serial.println("Relé: DESLIGADO");
      } else {
        digitalWrite(RELE_PIN, HIGH);
        Serial.println("Relé: LIGADO");
      }
    }
  }
  
  // Se o relé estiver ativado ou oscilando, emite bips periódicos
  static unsigned long ultimoBip = 0;
  if (releAtivado || releOscilando) {
    unsigned long tempoAtual = millis();
    if (tempoAtual - ultimoBip >= 1000) { // Bip a cada 2 segundos
      ultimoBip = tempoAtual;
      bipCurto();
    }
  }
  
  // Garante que o buzzer está na posição "desligado" ao final do loop
  digitalWrite(BUZZER_PIN, HIGH);  // HIGH para buzzer ativo com lógica invertida
}

// Verifica se o botão foi pressionado com debounce
boolean botaoPressionado() {
  // Lê o estado do botão (com pull-up, LOW = pressionado)
  if (digitalRead(BOTAO_PIN) == LOW) {
    delay(50); // Debounce
    
    if (digitalRead(BOTAO_PIN) == LOW) {
      // Aguarda soltar o botão
      while (digitalRead(BOTAO_PIN) == LOW) {
        delay(10);
      }
      return true;
    }
  }
  return false;
}

// Função para emitir um bip curto
void bipCurto() {
  // NOTA: A lógica aqui é invertida para buzzer ativo
  digitalWrite(BUZZER_PIN, LOW);  // Liga o buzzer
  delay(100);                    // Duração curta
  digitalWrite(BUZZER_PIN, HIGH); // Desliga o buzzer
}

// Função para emitir o alarme
void bipAlarme() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(BUZZER_PIN, LOW);  // Liga
    delay(100);
    digitalWrite(BUZZER_PIN, HIGH); // Desliga
    delay(100);
  }
  
  // Som mais longo para finalizar
  digitalWrite(BUZZER_PIN, LOW);  // Liga
  delay(300);
  digitalWrite(BUZZER_PIN, HIGH); // Desliga
}