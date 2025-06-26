// C++ code
//

//precisa baixar no arduino
#include <LiquidCrystal.h>
#include <string.h>

//define os 2 lcd e seus pinos (Arduino Mega)
LiquidCrystal lcd1(52, 50, 48, 46, 44, 42);
LiquidCrystal lcd2(34, 32, 30, 28, 26, 24);

//define os pinos dos botões (analogico)
int bot_esq = A5;
int bot_dir = A4;
int bot_tiro = A3;

int buzzer = 10;

//matriz com as informações do que exibir na tela
char tela[4][16+1];

//bytes dos sprites
byte nave[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00100,
  B01110,
  B11011,
};

byte nave_tiro[8] = {
  B00000,
  B00100,
  B00100,
  B00000,
  B00000,
  B00100,
  B01110,
  B11011,
};

byte alien1[8] = {
  B01010,
  B10101,
  B01110,
  B10001,
  B00000,
  B00000,
  B00000,
  B00000,
};

byte alien2[8] = {
  B01010,
  B10101,
  B01110,
  B01010,
  B00000,
  B00000,
  B00000,
  B00000,
};

byte alien1_tiro[8] = {
  B01010,
  B10101,
  B01110,
  B10001,
  B00000,
  B00100,
  B00100,
  B00000,
};

byte alien2_tiro[8] = {
  B01010,
  B10101,
  B01110,
  B01010,
  B00000,
  B00100,
  B00100,
  B00000,
};

byte tiro_baixo[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00100,
  B00100,
  B00000,
};

byte tiro_cima[8] = {
  B00000,
  B00100,
  B00100,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
};

//variaveis auxiliares
int pos_nave = 0; //posição da nave
bool direita = true; //controla a direção dos aliens
int limite_passos = 0; //controla até onde os aliens andam na animação (para parar na borda)
int tempo_anim_alien = 0; //controla o tempo de espera da animação dos aliens
bool tiro_nave = false; //controla se a nave deu um tiro ou não
int tiro_nave_x, tiro_nave_y; //posição x do tiro da nave (COLUNA) //posição y do tiro da nave (LINHA)
int tiro_nave_passos = 0; //controla os "passos" que o tiro dá
int tempo_espera_tiro_alien = 200; //tempo de espera inicial para um alien atirar
bool corre_tempo_espera_alien = true; //controla se já pode começar a contar o tempo de espera entre os tiros dos aliens
bool tiro_alien = false; //controla se um alien deu um tiro ou não
int alien_que_atira; //alien que vai atirar (posição dele)
int tiro_alien_passos = 0; //controla os "passos" que o tiro dá
int tiro_alien_x, tiro_alien_y; //posição x do tiro do alien (COLUNA) //posição y do tiro do alien (LINHA)

int qtd_aliens = 0;
int vida = 3;

//caso precise pra interrupção
/*
void move_esquerda();
void move_direita();
void atira_nave();*/

void setup()
{
  //inicia os 2 lcd
  lcd1.begin(16, 2);
  lcd2.begin(16, 2);
  
  //define os pinos dos botões como entrada
  pinMode(bot_esq, INPUT); //0 a 1023
  pinMode(bot_dir, INPUT);
  pinMode(bot_tiro, INPUT); //INPUT_PULLUP

  pinMode(buzzer, OUTPUT); //buzzer
  
  //configura interrupções
  //attachInterrupt(digitalPinToInterrupt(bot_esq), move_esquerda, RISING);
  //attachInterrupt(digitalPinToInterrupt(bot_dir), move_direita, RISING);
  //attachInterrupt(digitalPinToInterrupt(bot_tiro), atira_nave, RISING);
  
  //criando os sprites(bytes) nos lcd
  lcd2.createChar(0, nave);
  lcd2.createChar(7, nave_tiro);
  lcd1.createChar(1, alien1);
  lcd1.createChar(2, alien2);
  lcd1.createChar(3, alien1_tiro);
  lcd1.createChar(4, alien2_tiro);
  lcd1.createChar(5, tiro_baixo);
  lcd1.createChar(6, tiro_cima);
  lcd2.createChar(5, tiro_baixo);
  lcd2.createChar(6, tiro_cima);
  
  Serial.begin(9600); //inicia monitor serial
  
  //inicia no menu do jogo
  fase0();
}

void loop()
{
  verifica_vitoria(); // verifica vitoria ou derrota
  atualiza_tela();

  move_nave();
  move_aliens();
  move_tiro_nave();  
  
  randomiza_tiro_alien();
  move_tiro_alien();
  //Serial.println(vida);
}

//atualiza a tela imprimindo o que estiver na matriz tela
void atualiza_tela() {
  //Serial.println(digitalRead(bot_tiro)); (Caso queira verificar o valor de algum botão)
  //passa pela matriz tela e imprime no 1° lcd
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 16; j++) {
      lcd1.setCursor(j, i); //(coluna, linha)
      switch (tela[i][j]) {
        case '1':
        	lcd1.write(byte(1)); //imprime sprite alien1
       		break;
        case '2':
        	lcd1.write(byte(2)); //imprime sprite alien2
       		break;
        case '3':
        	lcd1.write(byte(3)); //imprime sprite alien1_tiro
       		break;
        case '4':
        	lcd1.write(byte(4)); //imprime sprite alien2_tiro
       		break;
        case '5':
        	lcd1.write(byte(5)); //imprime sprite tiro baixo
       		break;
        case '6':
        	lcd1.write(byte(6)); //imprime sprite tiro cima
       		break;
        default:
        	lcd1.print(tela[i][j]); //imprime o que estiver na variavel (literal)
        	break;
      }
    }
  }
  //passa pela matriz tela e imprime no 2° lcd
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 16; j++) {
      lcd2.setCursor(j, i); //(coluna, linha)
      switch (tela[i+2][j]) {
        case '0':
        	lcd2.write(byte(0));  //imprime sprite nave
        	break;
        case '7':
        	lcd2.write(byte(7));  //imprime sprite nave atirando
        	break;
        case '5':
        	lcd2.write(byte(5));
        	break;
        case '6':
        	lcd2.write(byte(6));
        	break;
        default:
        	lcd2.print(tela[i+2][j]);
        	break;
      }
    }
  }
}

//deixa a tela toda vazia
void limpa_tela() {
  strcpy(tela[0], "                ");
  strcpy(tela[1], "                ");
  strcpy(tela[2], "                ");
  strcpy(tela[3], "                ");
}

//Fase - Menu Inicial
void fase0() {
  limite_passos = 0;
  //Mensagem do menu inicial
  strcpy(tela[0], " SPACE INVADERS "); //passa a string inteira para uma linha da matriz
  strcpy(tela[1], "1 1 1 1 1       "); //onde os aliens animados vão ficar (1 = sprite alien)
  strcpy(tela[2], "  APERTE START  ");
  strcpy(tela[3], "  PARA INICIAR  ");
  
  while (true) {
    atualiza_tela();
    if (digitalRead(bot_tiro) == 1) { //espera apertar o botão de tiro(start) para começar
      tone (buzzer, 5000, 50);
      fase1();
      break;
    }
    
    //animação
    if (tempo_anim_alien == 2) { //espera um pouco mais antes de executar animação
      for (int i = 0; i < 16; i++) {
        if (tela[1][i] == '1' || tela[1][i] == '2') { //espaço onde tem um alien
          if (tela[1][i] == '1') { //esse if muda o sprite para deixar animado
            tela[1][i] = '2';
          }
          else {
            tela[1][i] = '1';
          }
          
          if (direita == true) { //controla se o alien vai pra direita(true) ou esquerda(false)
            tela[1][i + 1] = tela[1][i]; //faz o alien avançar uma posição para direita
          }
          else {
            tela[1][i - 1] = tela[1][i];
          }
          tela[1][i] = ' '; //depois que o alien avança, a posição dele fica sem nada
          i++; //pula um espaço para não verificar o mesmo alien que acabou de avançar
        }
      }
      limite_passos++; //guarda que o alien deu +1 passo
      if (limite_passos == 7) { //se os aliens chegarem até a borda (deu 7 passos)
        limite_passos = 0; //reseta a quantidade de passos
        direita = !direita; //inverte a direção de movimento
      }
    }
    if (tempo_anim_alien == 2) { //confere se esperou tempo suficiente para executar animação
      tempo_anim_alien = 0;
    }
    delay(50); //espera um tempinho entre os frames
    tempo_anim_alien++;
  }
}

void fase1 () {
  strcpy(tela[0], "  =INICIANDO!=  ");
  strcpy(tela[1], "     FASE I     ");
  strcpy(tela[2], "   SE PREPARE   ");
  strcpy(tela[3], " PARA A BATALHA ");
  atualiza_tela();
  delay(3000);
  
  //adiciona os aliens e a nave na tela
  strcpy(tela[0], "    11111111    "); //posição dos aliens
  strcpy(tela[1], "                ");
  strcpy(tela[2], "                ");
  strcpy(tela[3], "       0        "); //posição da nave - pos 7
  pos_nave = 7;
  direita = true;
  limite_passos = 4; //para os aliens já começarem no meio da linha
  vida = 3; //define quantidade de vidas
  lcd1.clear();
  lcd2.clear();
  
  //por preucação
  tiro_nave == false;
  
  
  //variaveis auxiliares (caso precise para quando reiniciar o jogo)
  //tempo_anim_alien = 0;
  //tiro_nave = false;
  //tiro_nave_passos = 0;
  //tempo_espera_tiro_alien = 200;
  //corre_tempo_espera_alien = true;
  //tiro_alien = false;
  //tiro_alien_passos = 0;
}

//funções com interrupção
/*
void move_esquerda() {
  if (pos_nave > 0) {
    pos_nave--;
  }
  
  for (int i = 0; i < 16; i++) {
    if (i == pos_nave) {
      tela[3][i] = '0';
    }
    else {
      tela[3][i] = ' ';
    }
  }
}

void move_direita() {
  if (pos_nave < 15) {
    pos_nave++;
  }
  
  for (int i = 0; i < 16; i++) {
    if (i == pos_nave) {
      tela[3][i] = '0';
    }
    else {
      tela[3][i] = ' ';
    }
  }
}

void atira_nave() {
  if (tiro_nave == false) {
  	tiro_nave = true; //marca que a nave deu um tiro (só pode dar um tiro por vez)
  	tiro_nave_x = pos_nave;
  	tiro_nave_y = 2; //para a animação do tiro começar na linha 2
  	tiro_nave_passos = 0;
  	nave_atirando(); //ativa sprite da nave atirando
  }
}*/

//funções sem interrupção
void move_nave() {
  if (digitalRead(bot_esq) == 1) { //apertou botão para a esquerda
    if (pos_nave > 0) {
      pos_nave--;
    }
    while (digitalRead(bot_esq) == 1) { //esperar soltar o botão (*o jogo pausa*)
    }
  }
  else if (digitalRead(bot_dir) == 1) { //apertou botão para a direita
    if (pos_nave < 15) {
      pos_nave++;
    }
    while (digitalRead(bot_dir) == 1) {
    }
  }
  if (digitalRead(bot_tiro) == 1 && tiro_nave == false) { //apertou botão de tiro
    while (digitalRead(bot_tiro) == 1) {
    }
    tone (buzzer, 5000, 50);
    tiro_nave = true; //marca que a nave deu um tiro (só pode dar um tiro por vez)
    tiro_nave_x = pos_nave;
    tiro_nave_y = 2; //para a animação do tiro começar na linha 2
    tiro_nave_passos = 0;
    nave_atirando(); //ativa sprite da nave atirando
  }
  
  //muda a posição da nave
  for (int i = 0; i < 16; i++) {
    if (i == pos_nave) {
      tela[3][i] = '0';
    }
    else {
      tela[3][i] = ' ';
    }
  }
}

void move_aliens() {
  //animação
  //laço para trocar sprite e deixar animado
  for (int i = 0; i < 16; i++) {
    if (tempo_anim_alien == 2) { //espera um pouco mais antes de executar animação
      if (tela[0][i] == '1') {
        tela[0][i] = '2';
      }
      else if (tela[0][i] == '2') {
        tela[0][i] = '1';
      }
    }
  }
  //verifica a direção em que os aliens precisam andar e os move 1 passo
  if (tempo_anim_alien == 2) { //espera um pouco mais antes de executar animação
    if (direita) {
      for (int i = 15; i > 0; i--) {
        tela[0][i] = tela[0][i-1]; //move 1 passo para direita
      }
      tela[0][0] = ' '; //ajusta a ultima posição que sobra
    }
  	else {
      for (int i = 0; i < 15; i++) {
        tela[0][i] = tela[0][i+1]; //move 1 passo para esquerda
      }
      tela[0][15] = ' '; //ajusta a ultima posição que sobra
  	}
  	limite_passos++;
  	if (limite_passos == 8) { //se andar o limite maximo de passos e chegar a borda, inverte a direção
      limite_passos = 0;
      direita = !direita;
  	}
  }
  if (tempo_anim_alien == 2) { //confere se esperou tempo suficiente para executar animação
    tempo_anim_alien = 0;
  }
  delay(50); //espera menos tempo para conseguir responder ao botão mais rapido
  tempo_anim_alien++; //conta as esperas para executar animação
}

//faz animação da nave atirando
void nave_atirando() {
  for (int i = 0; i < 16; i++) {
    if (i == pos_nave) {
      tela[3][i] = '7'; //muda sprite da nave para nave atirando
    }
    else {
      tela[3][i] = ' ';
    }
  }
  atualiza_tela();
  delay(10);
}

//move o tiro da nave e faz a animação dele
void move_tiro_nave() {
  if (tiro_nave == true) { //se o tiro existir
    if (tiro_nave_passos == 0 || tiro_nave_passos == 2) { //os momentos em que o tiro fica na parte de baixo da celula
      tela[tiro_nave_y][tiro_nave_x] = '5'; //sprite tiro baixo
    }
    else if (tiro_nave_passos == 1 || tiro_nave_passos == 3) { //os momentos em que o tiro fica na parte de cima da celula
      tela[tiro_nave_y][tiro_nave_x] = '6'; //sprite tiro cima
    }
    
    if (tiro_nave_passos == 2 || tiro_nave_passos == 4) { //a cada dois passos o tiro vai mudar de posição e subir uma linha
      tela[tiro_nave_y + 1][tiro_nave_x] = ' '; //apaga o sprite na posição antiga
    }
    tiro_nave_passos++;
    if (tiro_nave_passos % 2 == 0) { //a cada dois passos o tiro vai mudar de posição e subir uma linha
      tiro_nave_y--; //sobe uma linha
    }
    delay(10);
    
    if (tiro_nave_passos == 5) { //quando o tiro chega na ultima linha de cima
      if (tela[tiro_nave_y][tiro_nave_x] == '1') { //se acertou um alien
        tela[tiro_nave_y][tiro_nave_x] = '3'; //muda para a animação do alien tomando um tiro
        atualiza_tela();
      	delay(10);
        tone (buzzer, 100, 50);
      }
      else if (tela[tiro_nave_y][tiro_nave_x] == '2') { //se acertou um alien
        tela[tiro_nave_y][tiro_nave_x] = '4'; //muda para a animação do alien tomando um tiro
        atualiza_tela();
      	delay(10);
        tone (buzzer, 100, 50);
      }
      else { //se não acertou um alien o tiro passa direto
        tela[tiro_nave_y][tiro_nave_x] = '5';
        atualiza_tela();
      	delay(10);
        tela[tiro_nave_y][tiro_nave_x] = '6';
        atualiza_tela();
      }
      tela[tiro_nave_y][tiro_nave_x] = ' '; //apaga o sprite do alien
      tiro_nave = false; //marca que o tirou terminou e libera para dar outro tiro
    }
  }
}

void randomiza_tiro_alien() {
  if (tempo_espera_tiro_alien <= 0) { //quando não precisar esperar mais os aliens vão atirar
    //escolhe aleatoriamente que alien vai atirar
    while (true) {
      alien_que_atira = random(0, 16); //ordem do alien que vai atirar
      if (tela[0][alien_que_atira] == '1' || tela[0][alien_que_atira] == '2') {
        tiro_alien_x = alien_que_atira; //pega a posição x do alien que vai atirar
        break;
      }
    }
    tempo_espera_tiro_alien = random(100, 200); //tempo aleatorio em que os aliens vão esperar para atirar novamente
    corre_tempo_espera_alien == false;
    
    tiro_alien = true; //inicia tiro do alien
    tiro_alien_y = 1; //para a animação do tiro começar na linha 1
    tiro_alien_passos = 0;
    alien_atirando(); //ativa sprite do alien atirando
  }
  else if (tempo_espera_tiro_alien != 0 && corre_tempo_espera_alien == true) {
  	delay(10);
    tempo_espera_tiro_alien -= 10;
  }
}

void alien_atirando() {
  //muda sprite do alien para alien atirando
  if (tela[0][alien_que_atira] == '1') {
    tela[0][alien_que_atira] = '3';
  }
  else {
    tela[0][alien_que_atira] = '4';
  }
  atualiza_tela();
  delay(10); //100
  //muda sprite para alien normal
  if (tela[0][alien_que_atira] == '3') {
    tela[0][alien_que_atira] = '1';
  }
  else {
    tela[0][alien_que_atira] = '2';
  }
  atualiza_tela();
}

void move_tiro_alien() {
  if (tiro_alien == true) { //se o tiro existir
    if (tiro_alien_passos == 0 || tiro_alien_passos == 2) { //os momentos em que o tiro fica na parte de cima da celula
      tela[tiro_alien_y][tiro_alien_x] = '6'; //sprite tiro cima
    }
    else if (tiro_alien_passos == 1 || tiro_alien_passos == 3) { //os momentos em que o tiro fica na parte de baixo da celula
      tela[tiro_alien_y][tiro_alien_x] = '5'; //sprite tiro baixo
    }
    
    if (tiro_alien_passos == 2 || tiro_alien_passos == 4) { //a cada dois passos o tiro vai mudar de posição e descer uma linha
      tela[tiro_alien_y - 1][tiro_alien_x] = ' '; //apaga o sprite na posição antiga
    }
    tiro_alien_passos++;
    if (tiro_alien_passos % 2 == 0) { //a cada dois passos o tiro vai mudar de posição e subir uma linha
      tiro_alien_y++; //desce uma linha
    }
    delay(100);
    
    if (tiro_alien_passos == 5) { //quando o tiro chega na ultima linha de baixo
      if (tela[tiro_alien_y][tiro_alien_x] == '0') { //se acertou a nave
        tela[tiro_alien_y][tiro_alien_x] = 'X'; //muda para a animação da nave tomando um tiro
        atualiza_tela();
      	delay(100);
        //dá dano na nave
      	vida--;
      }
      else { //se não acertou a nave o tiro passa direto
        tela[tiro_alien_y][tiro_alien_x] = '6';
        atualiza_tela();
      	delay(100);
        tela[tiro_alien_y][tiro_alien_x] = '5';
        atualiza_tela();
      }
      
      corre_tempo_espera_alien = true; //começa a contar o tempo de espera entre os tiros dos aliens
      tiro_alien = false; //marca que o tiro terminou e libera para dar outro tiro
    }
  }
}

void verifica_vitoria() {
  qtd_aliens = 0;
  for (int i = 0; i < 16; i++) {
    if (tela[0][i] == '1' || tela[0][i] == '2') {
      qtd_aliens++;
    }
  }
  if (qtd_aliens == 0) {
  	//proxima_fase(); (caso queira adicionar novas fases)
    vitoria();
  }
  else if (vida == 0) {
    tone (buzzer, 100, 50); //som nave morrendo
    delay(1000);
    game_over();
  }
}

void game_over() {
  strcpy(tela[0], "  FIM DE JOGO!  ");
  strcpy(tela[1], "  VOCE MORREU!  ");
  strcpy(tela[2], "  APERTE START  ");
  strcpy(tela[3], " PARA REINICIAR ");
  atualiza_tela();
  tone(buzzer, 740, 400); //efeito sonoro de vitoria
  delay(450);
  tone(buzzer, 622, 400);
  delay(450);
  tone(buzzer, 523, 600);
  delay(650);
  tone(buzzer, 392, 800);
  delay(850);
  while (true) {
    if (digitalRead(bot_tiro) == 1) { //espera apertar o botão de tiro(start) para começar
      tone (buzzer, 5000, 50);
      fase1();
      break;
    }
  }
}

void vitoria() {
  strcpy(tela[0], "    VITORIA!    ");
  strcpy(tela[1], "    VITORIA!    ");
  strcpy(tela[2], "  APERTE START  ");
  strcpy(tela[3], " PARA REINICIAR ");
  atualiza_tela();
  tone(buzzer, 1046, 200); //efeito sonoro de vitoria
  delay(250);
  tone(buzzer, 1318, 200);
  delay(250);
  tone(buzzer, 1567, 200);
  delay(250);
  tone(buzzer, 2093, 400);
  delay(500);
  while (true) {
    if (digitalRead(bot_tiro) == 1) { //espera apertar o botão de tiro(start) para começar
      tone (buzzer, 5000, 50);
      fase0();
      break;
    }
  }
}






