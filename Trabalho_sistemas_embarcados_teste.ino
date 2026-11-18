// ===============================================================
// Código teste do trabalho de sistemas embarcados (Controle PD)
// ===============================================================

// Saturação do sinal de controle
#define MIN_PWM 0
#define MAX_PWM 255

// Variáveis do Encoder
double RPM = 0;
volatile unsigned long ultimo_pulso = 0;
volatile unsigned long periodo_pulso = 0;
double frequencia_pulsos = 0;
const int PPR = 10; // Pulsos Por Revolução (Quantidade de furos/dentes do disco)

// Variáveis do controle PD 
float KP = 0.2;
float KD = 0.005;
const float Ts = 0.001;
double velocidade_ang = 0;
double velocidade_ref = 10.0;
double erro = 0;
double erro_antes = 0;
uint32_t sinal_saida = 0;
uint32_t ultima_amostragem = 0;

// Controle da execução do sistema
bool executar = true;

// Pinos
const int PINO_ENA = 27;
const int PINO_IN1 = 26;
const int PINO_ENCODER = 19;

// ==================================================================================================
// Saída serial (Isso vai sair depois. Está aqui apenas para fins de teste por meio da saída Serial)
// ==================================================================================================
const unsigned long INTERVALO_PRINT_MS = 20;   // imprime a cada 20 ms
unsigned long ultimo_print = 0;
// ==================================================================================================
// Saída serial (Isso vai sair depois. Está aqui apenas para fins de teste por meio da saída Serial)
// ==================================================================================================


// Contagem do intervalo entre pulsos
void IRAM_ATTR contador() {

    uint32_t agora = micros();

    if (ultimo_pulso != 0) {

        uint32_t intervalo = agora - ultimo_pulso;

        if (intervalo >= 1000) { // Elimina ruído de 1 ms
            periodo_pulso = intervalo;
            ultimo_pulso = agora;
        }

    } else {
        ultimo_pulso = agora;
    }
}

void setup() {
  
  // ==================================================================================================
  // Saída serial (Isso vai sair depois. Está aqui apenas para fins de teste por meio da saída Serial)
  // ==================================================================================================
  //Inicializacao do monitor serial
  Serial.begin(115200);
  Serial.println("Monitoramento do RPM e velocidade angular");
  // ==================================================================================================
  // Saída serial (Isso vai sair depois. Está aqui apenas para fins de teste por meio da saída Serial)
  // ==================================================================================================

  // Configuração dos pinos   
  pinMode(PINO_IN1, OUTPUT);
  pinMode(PINO_ENCODER, INPUT_PULLUP);
  ledcAttach(PINO_ENA, 1000, 8);

  // Ativação do pino IN1 da ponte H
  digitalWrite(PINO_IN1, HIGH);

  // Inicializa a interrupção com o pino do encoder para contar o pulso a cada borda de subida
  attachInterrupt(digitalPinToInterrupt(PINO_ENCODER), contador, RISING);

  // ==================================================================================================
  // Saída serial (Isso vai sair depois. Está aqui apenas para fins de teste por meio da saída Serial)
  // ==================================================================================================
  Serial.println("ref_rad_s,w_rad_s,rpm,freq_hz,erro,u");
  // ==================================================================================================
  // Saída serial (Isso vai sair depois. Está aqui apenas para fins de teste por meio da saída Serial)
  // ==================================================================================================

}

void loop() {
  
  uint32_t agora = micros();

  if ((uint32_t)(agora - ultima_amostragem) >= 1000) { // Executa o controle a cada 1 ms

    ultima_amostragem = agora;

    // Leitura segura das informações do encoder
    uint32_t periodo_lido;
    uint32_t ultimo_pulso_lido;

    noInterrupts();

    periodo_lido = periodo_pulso;
    ultimo_pulso_lido = ultimo_pulso;

    interrupts();

    // Cálculo da velocidade e RPM
    uint32_t agora = micros();

    if (periodo_lido > 0 && (uint32_t)(agora - ultimo_pulso_lido) < 200000) { // Timeout de 200 ms
      RPM = 60000000.0 / (PPR * periodo_lido);

      frequencia_pulsos = (PPR * RPM) / 60.0;

      velocidade_ang = RPM * 2.0 * PI / 60.0;
    } else {
      RPM = 0;
      frequencia_pulsos = 0;
      velocidade_ang = 0;
    }

    if (executar) {
      // Cálculo do erro
      erro = velocidade_ref - velocidade_ang;

      // Controlador PD
      sinal_saida = (uint32_t)constrain(KP * erro + KD * (erro - erro_antes) / Ts, MIN_PWM, MAX_PWM);

      erro_antes = erro;

      // Aplicação do PWM
      ledcWrite(PINO_ENA, sinal_saida);
    } else {
      ledcWrite(PINO_ENA, 0);

      sinal_saida = 0;
      erro_antes = velocidade_ref - velocidade_ang;
    }

  }

  // ==================================================================================================
  // Saída serial (Isso vai sair depois. Está aqui apenas para fins de teste por meio da saída Serial)
  // ==================================================================================================
  // Saída serial (CSV) a cada 20 ms
  unsigned long agora_ms = millis();
  if (agora_ms - ultimo_print >= INTERVALO_PRINT_MS) {
    ultimo_print = agora_ms;
     Serial.printf("%.2f,%.3f,%.1f,%.2f,%.3f,%.1f\n",
                   velocidade_ref,
                   velocidade_ang,
                   RPM,
                   frequencia_pulsos,
                   erro,
                   sinal_saida);
  }
  // ==================================================================================================
  // Saída serial (Isso vai sair depois. Está aqui apenas para fins de teste por meio da saída Serial)
  // ==================================================================================================
}
