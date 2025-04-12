// Código Arduino para transmissão de código Morse via lâmpada conectada a relé (lógica invertida)
// Pino ao qual o relé está conectado
const int relePin = 8;

// Tempos para código Morse (em milissegundos)
const int dotDuration = 200;         // Duração de um ponto
const int dashDuration = dotDuration * 3;  // Duração de um traço
const int elementSpace = dotDuration;      // Espaço entre elementos (ponto/traço)
const int letterSpace = dotDuration * 3;   // Espaço entre letras
const int wordSpace = dotDuration * 7;     // Espaço entre palavras

// Estrutura para armazenar o código Morse de cada caractere
struct MorseCode {
  char character;
  String code;
};

// Tabela de código Morse para letras e números
const MorseCode morseTable[] = {
  {'A', ".-"}, {'B', "-..."}, {'C', "-.-."}, {'D', "-.."}, {'E', "."},
  {'F', "..-."}, {'G', "--."}, {'H', "...."}, {'I', ".."}, {'J', ".---"},
  {'K', "-.-"}, {'L', ".-.."}, {'M', "--"}, {'N', "-."}, {'O', "---"},
  {'P', ".--."}, {'Q', "--.-"}, {'R', ".-."}, {'S', "..."}, {'T', "-"},
  {'U', "..-"}, {'V', "...-"}, {'W', ".--"}, {'X', "-..-"}, {'Y', "-.--"},
  {'Z', "--.."}, {'0', "-----"}, {'1', ".----"}, {'2', "..---"},
  {'3', "...--"}, {'4', "....-"}, {'5', "....."}, {'6', "-...."},
  {'7', "--..."}, {'8', "---.."}, {'9', "----."},
  {' ', " "}  // Espaço para separar palavras
};

void setup() {
  // Inicializa o pino do relé como saída
  pinMode(relePin, OUTPUT);
  
  // Lâmpada desligada no início (lembre-se da lógica invertida)
  digitalWrite(relePin, HIGH);
  
  // Inicializa a comunicação serial
  Serial.begin(9600);
  Serial.println("Sistema de Transmissão de Código Morse");
  Serial.println("Digite o texto a ser transmitido e pressione Enter");
}

void loop() {
  // Verifica se há dados disponíveis na porta serial
  if (Serial.available() > 0) {
    // Lê a string enviada até encontrar o caractere de nova linha
    String inputText = Serial.readStringUntil('\n');
    
    // Converte o texto para maiúsculas para simplificar a correspondência
    inputText.toUpperCase();
    
    Serial.print("Transmitindo: ");
    Serial.println(inputText);
    
    // Transmite o texto em código Morse
    transmitMorse(inputText);
    
    Serial.println("Transmissão concluída. Digite outro texto:");
  }
}

// Função para transmitir o texto em código Morse
void transmitMorse(String text) {
  for (int i = 0; i < text.length(); i++) {
    char c = text.charAt(i);
    
    // Procura o caractere na tabela Morse
    String morseChar = "";
    for (int j = 0; j < sizeof(morseTable) / sizeof(morseTable[0]); j++) {
      if (morseTable[j].character == c) {
        morseChar = morseTable[j].code;
        break;
      }
    }
    
    // Se o caractere foi encontrado na tabela
    if (morseChar != "") {
      // Se for um espaço, aguarde o tempo entre palavras
      if (c == ' ') {
        delay(wordSpace);
      }
      else {
        // Transmite cada elemento do código Morse (ponto ou traço)
        for (int k = 0; k < morseChar.length(); k++) {
          char element = morseChar.charAt(k);
          
          // Ativa a lâmpada (LOW devido à lógica invertida)
          digitalWrite(relePin, LOW);
          
          // Mantém a lâmpada acesa pelo tempo apropriado (ponto ou traço)
          if (element == '.') {
            delay(dotDuration);
          }
          else if (element == '-') {
            delay(dashDuration);
          }
          
          // Desativa a lâmpada (HIGH devido à lógica invertida)
          digitalWrite(relePin, HIGH);
          
          // Pausa entre elementos se não for o último elemento do caractere
          if (k < morseChar.length() - 1) {
            delay(elementSpace);
          }
        }
        
        // Espaço entre letras (apenas se não for o último caractere e o próximo não for espaço)
        if (i < text.length() - 1 && text.charAt(i + 1) != ' ') {
          delay(letterSpace);
        }
      }
    }
  }
}