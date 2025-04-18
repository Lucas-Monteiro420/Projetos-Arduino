/*
 * Despertador Arduino com Buzzer Ativo (5V) e Relé Oscilante
 * 
 * Este código considera que:
 * - O buzzer é do tipo ativo com conexão ao 5V
 * - O pino digital apenas controla o acionamento do buzzer
 * - O relé oscila (liga/desliga) após o horário programado até que o sistema seja reiniciado
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
unsigned long tempoAtualMillis = 0;
unsigned long ultimoSegundo = 0;

// Variáveis para o relógio e despertador
int horaAtual = 12;
int minutoAtual = 0;
int segundoAtual = 0;
int horaAlarme = 0;
int minutoAlarme = 0;

// Estados do sistema
boolean alarmeConfigurado = false;
boolean alarmeAtivado = false;
boolean alarmeOscilando = false;
boolean aguardandoComando = false;

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
  
  // Aguarda conexão serial (útil quando conectado via USB)
  delay(2000);
  
  Serial.println("Sistema de Despertador Arduino");
  Serial.println("---------------------------------");
  
  // Configura o horário inicial
  solicitarHorarioAtual();
  
  // Inicializa o tempo de referência
  ultimoSegundo = millis();
  
  // Bip inicial
  bipCurto();
  
  Serial.println("\nSistema iniciado! O relógio está funcionando.");
  Serial.println("Pressione o botão para configurar o alarme.");
}

void loop() {
  // Atualiza o tempo atual
  atualizarRelogio();
  
  // Verifica se é hora de disparar o alarme
  verificarAlarme();
  
  // Verifica o botão para desativar o alarme ou configurar um novo
  if (botaoPressionado()) {
    if (alarmeAtivado || alarmeOscilando) {
      // Desativa o alarme se estiver ativo ou oscilando
      digitalWrite(RELE_PIN, LOW);
      alarmeAtivado = false;
      alarmeOscilando = false;
      bipCurto();
      Serial.println("Alarme desativado.");
    } else {
      // Configura um novo alarme
      solicitarHorarioAlarme();
    }
  }
  
  // Verifica se há comandos disponíveis no Serial
  if (Serial.available() > 0) {
    char comando = Serial.read();
    
    // Limpa caracteres de nova linha e retorno de carro
    if (comando == '\n' || comando == '\r') {
      // Ignora
    } 
    // Comando 'a': configurar alarme
    else if (comando == 'a' || comando == 'A') {
      solicitarHorarioAlarme();
    } 
    // Comando 'h': configurar hora atual
    else if (comando == 'h' || comando == 'H') {
      solicitarHorarioAtual();
    } 
    // Comando 'r': reiniciar/desativar alarme
    else if (comando == 'r' || comando == 'R') {
      if (alarmeAtivado || alarmeOscilando) {
        digitalWrite(RELE_PIN, LOW);
        alarmeAtivado = false;
        alarmeOscilando = false;
        Serial.println("Alarme desativado.");
      } else {
        Serial.println("Não há alarme ativo para desativar.");
      }
    }
    // Comando 's': mostrar status
    else if (comando == 's' || comando == 'S') {
      exibirStatus();
    }
    // Comando inválido
    else {
      Serial.println("\nComandos disponíveis:");
      Serial.println("A - Configurar alarme");
      Serial.println("H - Configurar hora atual");
      Serial.println("R - Reiniciar/desativar alarme");
      Serial.println("S - Mostrar status");
    }
    
    // Limpa o restante do buffer
    while (Serial.available()) {
      Serial.read();
    }
  }
  
  // Controla a oscilação do relé quando o alarme estiver ativado
  if (alarmeOscilando) {
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
  
  // Se o alarme estiver ativado ou oscilando, emite bips periódicos
  static unsigned long ultimoBip = 0;
  if (alarmeAtivado || alarmeOscilando) {
    unsigned long tempoAtual = millis();
    if (tempoAtual - ultimoBip >= 1000) { // Bip a cada segundo
      ultimoBip = tempoAtual;
      bipCurto();
    }
  }
  
  // Garante que o buzzer está na posição "desligado" ao final do loop
  digitalWrite(BUZZER_PIN, HIGH);
}

// Função para verificar comandos do Serial periodicamente
void verificarComandosSerial() {
  if (!Serial.available() && !aguardandoComando) {
    aguardandoComando = true;
    Serial.println("\n----- MENU DO DESPERTADOR -----");
    Serial.println("Digite um comando:");
    Serial.println("A - Configurar alarme");
    Serial.println("H - Configurar hora atual");
    Serial.println("R - Reiniciar/desativar alarme");
    Serial.println("S - Mostrar status atual");
    Serial.println("-----------------------------");
  }
}

// Função para exibir o status atual do sistema
void exibirStatus() {
  Serial.println("\n----- STATUS DO SISTEMA -----");
  
  // Mostra o horário atual
  Serial.print("Horário atual: ");
  if (horaAtual < 10) Serial.print("0");
  Serial.print(horaAtual);
  Serial.print(":");
  if (minutoAtual < 10) Serial.print("0");
  Serial.print(minutoAtual);
  Serial.print(":");
  if (segundoAtual < 10) Serial.print("0");
  Serial.println(segundoAtual);
  
  // Mostra status do alarme
  if (alarmeConfigurado) {
    Serial.print("Alarme configurado para: ");
    if (horaAlarme < 10) Serial.print("0");
    Serial.print(horaAlarme);
    Serial.print(":");
    if (minutoAlarme < 10) Serial.print("0");
    Serial.println(minutoAlarme);
    
    // Verifica se o alarme está ativo
    if (alarmeAtivado || alarmeOscilando) {
      Serial.println("Status: ALARME ATIVADO!");
    } else {
      Serial.println("Status: Aguardando horário programado");
    }
  } else {
    Serial.println("Alarme: Não configurado");
  }
  
  // Status do relé
  Serial.print("Relé: ");
  Serial.println(digitalRead(RELE_PIN) == HIGH ? "LIGADO" : "DESLIGADO");
  
  Serial.println("----------------------------");
}

// Função para atualizar o relógio
void atualizarRelogio() {
  tempoAtualMillis = millis();
  
  // Atualiza a cada segundo
  if (tempoAtualMillis - ultimoSegundo >= 1000) {
    ultimoSegundo = tempoAtualMillis;
    
    // Atualiza os segundos
    segundoAtual++;
    
    // Atualiza os minutos
    if (segundoAtual >= 60) {
      segundoAtual = 0;
      minutoAtual++;
      
      // Atualiza as horas
      if (minutoAtual >= 60) {
        minutoAtual = 0;
        horaAtual++;
        
        // Reinicia o dia
        if (horaAtual >= 24) {
          horaAtual = 0;
        }
      }
      
      // Mostra o horário atual a cada minuto
      if (segundoAtual == 0) {
        exibirHorarioAtual();
      }
    }
    
    // Verifica comandos do Serial a cada 30 segundos
    if (segundoAtual % 30 == 0) {
      verificarComandosSerial();
    } else {
      aguardandoComando = false;
    }
  }
}

// Função para verificar se é hora de disparar o alarme
void verificarAlarme() {
  if (alarmeConfigurado && !alarmeAtivado && !alarmeOscilando) {
    if (horaAtual == horaAlarme && minutoAtual == minutoAlarme && segundoAtual == 0) {
      dispararAlarme();
    }
  }
}

// Função para disparar o alarme
void dispararAlarme() {
  // Inicia a oscilação do relé
  alarmeOscilando = true;
  alarmeAtivado = true;
  
  // Inicializa o estado do relé (começa ligado)
  digitalWrite(RELE_PIN, HIGH);
  ultimaMudancaRele = millis();
  
  // Sinaliza com o buzzer
  bipAlarme();
  
  Serial.println("\n*** ALARME ATIVADO! ***");
  Serial.println("Pressione o botão para desativar o alarme.");
}

// Exibe o horário atual formatado
void exibirHorarioAtual() {
  Serial.print("\nHorário atual: ");
  if (horaAtual < 10) {
    Serial.print("0");
  }
  Serial.print(horaAtual);
  Serial.print(":");
  if (minutoAtual < 10) {
    Serial.print("0");
  }
  Serial.print(minutoAtual);
  Serial.print(":");
  if (segundoAtual < 10) {
    Serial.print("0");
  }
  Serial.println(segundoAtual);
  
  if (alarmeConfigurado) {
    Serial.print("Alarme: ");
    if (horaAlarme < 10) {
      Serial.print("0");
    }
    Serial.print(horaAlarme);
    Serial.print(":");
    if (minutoAlarme < 10) {
      Serial.print("0");
    }
    Serial.println(minutoAlarme);
  }
}

// Solicita a configuração do horário atual
void solicitarHorarioAtual() {
  Serial.println("\nConfigurando o horário atual...");
  
  // Solicita a hora
  Serial.println("Digite a hora atual (0-23):");
  
  // Pequeno atraso para dar tempo do usuário ver a mensagem
  delay(500);
  
  // Aguarda entrada do usuário com timeout
  unsigned long tempoInicio = millis();
  while (Serial.available() == 0) {
    // Verifica se passou muito tempo sem resposta
    if (millis() - tempoInicio > 30000) { // 30 segundos
      Serial.println("Tempo esgotado! Definindo hora para 12.");
      horaAtual = 12;
      break;
    }
    delay(100); // Pequeno atraso para não sobrecarregar o processador
  }
  
  // Lê a hora se houver entrada disponível
  if (Serial.available() > 0) {
    horaAtual = Serial.parseInt();
    if (horaAtual < 0 || horaAtual > 23) {
      horaAtual = 12;
      Serial.println("Hora inválida! Definindo para 12.");
    } else {
      Serial.print("Hora configurada para: ");
      Serial.println(horaAtual);
    }
    
    // Limpa o buffer
    while (Serial.available()) {
      Serial.read();
    }
  }
  
  delay(500); // Pequeno atraso para o usuário ler a mensagem
  
  // Solicita o minuto
  Serial.println("Digite o minuto atual (0-59):");
  
  // Aguarda entrada do usuário com timeout
  tempoInicio = millis();
  while (Serial.available() == 0) {
    // Verifica se passou muito tempo sem resposta
    if (millis() - tempoInicio > 30000) { // 30 segundos
      Serial.println("Tempo esgotado! Definindo minuto para 0.");
      minutoAtual = 0;
      break;
    }
    delay(100); // Pequeno atraso para não sobrecarregar o processador
  }
  
  // Lê o minuto se houver entrada disponível
  if (Serial.available() > 0) {
    minutoAtual = Serial.parseInt();
    if (minutoAtual < 0 || minutoAtual > 59) {
      minutoAtual = 0;
      Serial.println("Minuto inválido! Definindo para 0.");
    } else {
      Serial.print("Minuto configurado para: ");
      Serial.println(minutoAtual);
    }
    
    // Limpa o buffer
    while (Serial.available()) {
      Serial.read();
    }
  }
  
  // Inicializa os segundos
  segundoAtual = 0;
  
  Serial.println("Horário atual configurado!");
  exibirHorarioAtual();
}

// Solicita a configuração do horário do alarme
void solicitarHorarioAlarme() {
  Serial.println("\nConfigurando o alarme...");
  
  // Solicita a hora
  Serial.println("Digite a hora do alarme (0-23):");
  
  // Pequeno atraso para dar tempo do usuário ver a mensagem
  delay(500);
  
  // Aguarda entrada do usuário com timeout
  unsigned long tempoInicio = millis();
  while (Serial.available() == 0) {
    // Verifica se passou muito tempo sem resposta
    if (millis() - tempoInicio > 30000) { // 30 segundos
      Serial.println("Tempo esgotado! Definindo hora do alarme para atual + 1 hora.");
      horaAlarme = (horaAtual + 1) % 24; // Próxima hora, com overflow para 0 se for 23
      break;
    }
    delay(100); // Pequeno atraso para não sobrecarregar o processador
  }
  
  // Lê a hora se houver entrada disponível
  if (Serial.available() > 0) {
    horaAlarme = Serial.parseInt();
    if (horaAlarme < 0 || horaAlarme > 23) {
      horaAlarme = (horaAtual + 1) % 24;
      Serial.println("Hora inválida! Definindo para próxima hora.");
    } else {
      Serial.print("Hora do alarme: ");
      Serial.println(horaAlarme);
    }
    
    // Limpa o buffer
    while (Serial.available()) {
      Serial.read();
    }
  }
  
  delay(500); // Pequeno atraso para o usuário ler a mensagem
  
  // Solicita o minuto
  Serial.println("Digite o minuto do alarme (0-59):");
  
  // Aguarda entrada do usuário com timeout
  tempoInicio = millis();
  while (Serial.available() == 0) {
    // Verifica se passou muito tempo sem resposta
    if (millis() - tempoInicio > 30000) { // 30 segundos
      Serial.println("Tempo esgotado! Definindo minuto do alarme para 0.");
      minutoAlarme = 0;
      break;
    }
    delay(100); // Pequeno atraso para não sobrecarregar o processador
  }
  
  // Lê o minuto se houver entrada disponível
  if (Serial.available() > 0) {
    minutoAlarme = Serial.parseInt();
    if (minutoAlarme < 0 || minutoAlarme > 59) {
      minutoAlarme = 0;
      Serial.println("Minuto inválido! Definindo para 0.");
    } else {
      Serial.print("Minuto do alarme: ");
      Serial.println(minutoAlarme);
    }
    
    // Limpa o buffer
    while (Serial.available()) {
      Serial.read();
    }
  }
  
  // Ativa o alarme
  alarmeConfigurado = true;
  Serial.println("Alarme configurado com sucesso!");
  Serial.print("Alarme programado para: ");
  if (horaAlarme < 10) {
    Serial.print("0");
  }
  Serial.print(horaAlarme);
  Serial.print(":");
  if (minutoAlarme < 10) {
    Serial.print("0");
  }
  Serial.println(minutoAlarme);
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
