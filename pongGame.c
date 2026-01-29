/*
....:::: TRABALHO FINAL DA DISCIPLINA DE SISTEMAS MICROCONTROLADOS - 2025/2 ::::....

-> Clássico Jogo PONG

Materiais:
  -> Microcontrolador MSP430G2553
  -> 2x Matriz de LED 8x8 MAX7219 (circuito integrado)
  -> 2x Potenciômetro 10kohm.
  -> Protoboard.
*/

#include <msp430.h>
#include <stdint.h>
#include <stdio.h>

#define SPI_SIMO	BIT7
#define SPI_CLK		BIT5
#define SPI_CS		BIT4

#define MAX_NOOP	0x00
#define MAX_DIGIT0	0x01
#define MAX_DIGIT1	0x02
#define MAX_DIGIT2	0x03
#define MAX_DIGIT3	0x04
#define MAX_DIGIT4	0x05
#define MAX_DIGIT5	0x06
#define MAX_DIGIT6	0x07
#define MAX_DIGIT7	0x08
#define MAX_DECODEMODE	0x09
#define MAX_INTENSITY	0x0A
#define MAX_SCANLIMIT	0x0B
#define MAX_SHUTDOWN	0x0C
#define MAX_DISPLAYTEST	0x0F

void ini_P1_P2(void);
void spi_init(void);
void spi_max(uint8_t address, uint8_t data, uint8_t address2, uint8_t data2);
void tela_inicial(void);
void ini_timerA_debouncer(void);
void ini_uCon(void);
void ini_ADC10(void);
void desenha_raquete_esq(void);
void desenha_raquete_dir(void);
void desenha_bola(void);
void atualiza_bola(void);
void loop_pong(void);
void ini_timerA_game(void);
void mostra_modo(int dificuldade);
void mostra_ponto(unsigned int ponto, unsigned int ponto2);

const uint8_t pontos[8][8] = {
  {0b00111100,0b01000010,0b01000010,0b00111100,0b00000000,0b01000010,0b00100100,0b00011000},
  {0b00000000,0b01000100,0b01111110,0b01000000,0b00000000,0b01000010,0b00100100,0b00011000},
  {0b01000100,0b01100010,0b01010010,0b01001100,0b00000000,0b01000010,0b00100100,0b00011000},
  {0b00100100,0b01000010,0b01001010,0b00111100,0b00000000,0b01000010,0b00100100,0b00011000},
  {0b00011000,0b00100100,0b01000010,0b00000000,0b00111100,0b01000010,0b01000010,0b00111100},
  {0b00011000,0b00100100,0b01000010,0b00000000,0b01000100,0b01111110,0b01000000,0b00000000},
  {0b00011000,0b00100100,0b01000010,0b00000000,0b01000100,0b01100010,0b01010010,0b01001100},
  {0b00011000,0b00100100,0b01000010,0b00000000,0b00100100,0b01000010,0b01001010,0b00111100}
};

const uint8_t modos[4][8] = {
  {0b01111110,0b00001000,0b01111110,0b00000000,0b01111110,0b00001010,0b01111110,0b00000000},
  {0b01111110,0b00010010,0b01101110,0b00000000,0b01111110,0b01000010,0b00111100,0b00000000},
  {0b01111110,0b01001010,0b01001010,0b00000000,0b01111110,0b00010010,0b01111110,0b00000000},
  {0b01001110,0b01001010,0b01001010,0b01111010,0b00000000,0b01101110,0b01001000,0b01111110}
};

const uint8_t IMAGES[38][8] = {
  {0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000},
  {0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000},
  {0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000},
  {0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b01111110},
  {0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000},
  {0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b01111110,0b00010010,0b00010010},
  {0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000},
  {0b00000000,0b00000000,0b00000000,0b01111110,0b00010010,0b00010010,0b00001100,0b00000000},
  {0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000},
  {0b00000000,0b01111110,0b00010010,0b00010010,0b00001100,0b00000000,0b00111100,0b01000010},
  {0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b01111110},
  {0b00010010,0b00010010,0b00001100,0b00000000,0b00111100,0b01000010,0b01000010,0b00111100},
  {0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b01111110,0b00010010,0b00010010},
  {0b00001100,0b00000000,0b00111100,0b01000010,0b01000010,0b00111100,0b00000000,0b01111110},
  {0b00000000,0b00000000,0b00000000,0b01111110,0b00010010,0b00010010,0b00001100,0b00000000},
  {0b00111100,0b01000010,0b01000010,0b00111100,0b00000000,0b01111110,0b00000100,0b00011000},
  {0b00000000,0b01111110,0b00010010,0b00010010,0b00001100,0b00000000,0b00111100,0b01000010},
  {0b01000010,0b00111100,0b00000000,0b01111110,0b00000100,0b00011000,0b00100000,0b01111110},
  {0b00010010,0b00010010,0b00001100,0b00000000,0b00111100,0b01000010,0b01000010,0b00111100},
  {0b00000000,0b01111110,0b00000100,0b00011000,0b00100000,0b01111110,0b00000000,0b00111100},
  {0b00001100,0b00000000,0b00111100,0b01000010,0b01000010,0b00111100,0b00000000,0b01111110},
  {0b00000100,0b00011000,0b00100000,0b01111110,0b00000000,0b00111100,0b01000010,0b01010010},
  {0b00111100,0b01000010,0b01000010,0b00111100,0b00000000,0b01111110,0b00000100,0b00011000},
  {0b00100000,0b01111110,0b00000000,0b00111100,0b01000010,0b01010010,0b00110100,0b00000000},
  {0b01000010,0b00111100,0b00000000,0b01111110,0b00000100,0b00011000,0b00100000,0b01111110},
  {0b00000000,0b00111100,0b01000010,0b01010010,0b00110100,0b00000000,0b00000000,0b00000000},
  {0b00000000,0b01111110,0b00000100,0b00011000,0b00100000,0b01111110,0b00000000,0b00111100},
  {0b01000010,0b01010010,0b00110100,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000},
  {0b00000100,0b00011000,0b00100000,0b01111110,0b00000000,0b00111100,0b01000010,0b01010010},
  {0b00110100,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000},
  {0b00100000,0b01111110,0b00000000,0b00111100,0b01000010,0b01010010,0b00110100,0b00000000},
  {0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000},
  {0b00000000,0b00111100,0b01000010,0b01010010,0b00110100,0b00000000,0b00000000,0b00000000},
  {0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000},
  {0b01000010,0b01010010,0b00110100,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000},
  {0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000},
  {0b00110100,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000},
  {0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000}
};

unsigned int botaoPres = 1, jogo_pausado = 0, jogo_comecando = 1;
unsigned int ADC10_vetor[8];
uint8_t canal_adc = 1;
unsigned int i, row, framecounter, dificuldade = 1, jogo_ativo = 0, pontos_1 = 0, pontos_2 = 0;
uint16_t soma = 0, media = 0;

uint8_t tela_esq[8] = {0};
uint8_t tela_dir[8] = {0};

uint8_t raquete_esq_y = 2;
uint8_t raquete_dir_y = 2;

uint8_t bola_x = 7;
uint8_t bola_y = 3; 

int8_t vx = -1;       
int8_t vy =  1;      

int main(void)
{
    ini_P1_P2();
    ini_uCon();
    ini_timerA_debouncer();
    spi_init();
    ini_ADC10();
    tela_inicial();

    while(1)
    {

    }
}

void spi_max(uint8_t address, uint8_t data, uint8_t address2, uint8_t data2)
{
  P1OUT &= ~(SPI_CS);
	UCB0TXBUF = address2 & 0b00001111;	

	while (UCB0STAT & UCBUSY);		
	UCB0TXBUF = data2;		

	while (UCB0STAT & UCBUSY);
  UCB0TXBUF = address & 0b00001111;	

	while (UCB0STAT & UCBUSY);		
	UCB0TXBUF = data;

	while (UCB0STAT & UCBUSY);
  P1OUT |= SPI_CS;
}

void mostra_modo(int dificuldade)
{
  if(dificuldade == 1)
  { 
    for(i = 0; i < 8; i++)
    {
        spi_max(MAX_DIGIT0+i, modos[0][i], MAX_DIGIT0+i, modos[1][i]);
    } 
  }
  else
  { 
    for(i = 0; i < 8; i++)
    {
        spi_max(MAX_DIGIT0+i, modos[2][i], MAX_DIGIT0+i, modos[3][i]);
    }
  }
}

void mostra_ponto(unsigned int ponto, unsigned int ponto2)
{
  for (row=0; row<8; row++){
            spi_max(MAX_DIGIT0+row, pontos[ponto][row], MAX_DIGIT0+row, pontos[ponto2+4][row]);
  }
  __delay_cycles(2000000);
  
  pontos_1 = 0;
  pontos_2 = 0;
    
}

void ini_uCon(void) 
{
    WDTCTL = WDTPW + WDTHOLD;              
    DCOCTL = CALDCO_16MHZ;
    BCSCTL1 = CALBC1_16MHZ;
    BCSCTL2 = DIVS1;
    BCSCTL3 = XCAP0 + XCAP1;

    while(BCSCTL3 & LFXT1OF);

    __enable_interrupt();
}

void ini_P1_P2(void)
{
  P1DIR |= SPI_CS; 
  P1DIR &= ~(BIT3); 
  P1OUT |= SPI_CS + BIT3; 
  P1REN = BIT3; 
  P1IES = BIT3; 
  P1IFG = 0; 
  P1IE = BIT3; 
  
  P1SEL |= SPI_SIMO + SPI_CLK;
  P1SEL2 |= SPI_SIMO + SPI_CLK;
}

#pragma vector=PORT1_VECTOR
__interrupt void RTI_PORTA_1(void)
{
  P1IE &= ~BIT3;
  botaoPres = 0; 
  TA0CTL |= MC0; 

  if(jogo_ativo == 2) 
  {
    if(jogo_comecando == 1)
    {
      jogo_pausado = 0; 
      jogo_comecando = 0;
    }
    else if(jogo_comecando == 0)
    {//deve ser na rti da do timer.
      if(jogo_pausado == 0) 
      {
        jogo_pausado = 1;
      }
      else if(jogo_pausado == 1)
      {
        jogo_pausado = 0;
      }
    }
  }
}

void ini_timerA_debouncer(void)
{
    TA0CTL = TASSEL1 + ID0 + ID1;
    TA0CCTL0 = CCIE;
    TA0CCR0 = 49999;
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void RTI_Mod_0_Timer_0(void)
{
  TA0CTL &= ~MC0; 
  P1IFG &= ~BIT3; 
  P1IE |= BIT3; 
}

void ini_timerA_game(void)
{
  TA1CTL = TASSEL1 + ID0 + ID1 + MC0; 

  if(dificuldade == 1){
    TA1CCR0 = 24999; 
  }
  else{
    TA1CCR0 = 49999; 
  }
  TA1CCTL0 = CCIE;  
}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void RTI_Timer_Game(void)
{
  ADC10CTL0 |= ENC + ADC10SC; 

  loop_pong();

  if(jogo_pausado == 1 || jogo_comecando == 1)
    TA1CTL &= ~MC0; 
}

void ini_ADC10(void) 
{  
  ADC10CTL0 = ADC10SHT1 + MSC + ADC10IE + ADC10ON;
  ADC10CTL1 = INCH0 + ADC10SSEL1 + ADC10SSEL0 + ADC10DIV0 + CONSEQ1;
  ADC10AE0 = BIT1 + BIT2;

  ADC10DTC0 = 0;
  ADC10DTC1 = 8;
  ADC10SA = &ADC10_vetor[0]; 

  ADC10CTL0 |= ENC + ADC10SC;
}

#pragma vector=ADC10_VECTOR
__interrupt void RTI_ADC10(void)
{
    ADC10CTL0 &= ~ENC; //limpar ADC10SC

    soma = 0;
    media = 0;

    for(i = 0; i < 8; i++){
        soma += ADC10_vetor[i];
    }

    media = soma / 8;
    
    if(canal_adc == 1){
      if(dificuldade == 1){
        raquete_esq_y = (media * 6) / 1023;
      }
      else{
        raquete_esq_y = (media * 5) / 1023;
      }
        canal_adc = 2;
        ADC10CTL1 &= ~(INCH0+INCH1+INCH2);
        ADC10CTL1 |= INCH1;
    }

    else{
      if(dificuldade == 1){
        raquete_dir_y = (media * 6) / 1023;
      }
      else{
        raquete_dir_y = (media * 5) / 1023;
      }
        canal_adc = 1;
        ADC10CTL1 &= ~(INCH0+INCH1+INCH2);
        ADC10CTL1 |= INCH0;
    }

    ADC10SA = &ADC10_vetor[0];
}

void spi_init(void) 
{
	UCB0CTL1 |= UCSWRST; 
	UCB0CTL0 = UCCKPH + UCMSB + UCMST + UCSYNC;
	UCB0CTL1 |= UCSSEL1; 		
	UCB0BR0 |= 0x01; 
	UCB0BR1 = 0;
	UCB0CTL1 &= ~UCSWRST; 
}

void tela_inicial(void)
{	
  spi_max(MAX_NOOP, 0x00, MAX_NOOP, 0x00); 
	spi_max(MAX_SCANLIMIT, 0x07, MAX_SCANLIMIT, 0x07); 	
	spi_max(MAX_INTENSITY, 0x08, MAX_INTENSITY, 0x08); 	
	spi_max(MAX_DECODEMODE, 0, MAX_DECODEMODE, 0);	
	spi_max(MAX_SHUTDOWN,1, MAX_SHUTDOWN, 1);

  while(1)
  {
    if(jogo_ativo == 0) 
    {
      while(1)
      {
        for (framecounter=0; framecounter<38; framecounter = framecounter+2)
        {
          for (row=0; row<8; row++)
          {
            spi_max(MAX_DIGIT0+row, IMAGES[framecounter][row], MAX_DIGIT0+row, IMAGES[framecounter+1][row]);
          }	
          __delay_cycles(2000000);
        }

        if(botaoPres == 0)
        {
          jogo_ativo = 1; 
          botaoPres = 1; 
          break; 
        }
      }
    }

    else if(jogo_ativo == 1)
    {
      mostra_modo(dificuldade);

      while(1){ 
        dificuldade = ~dificuldade; 
        mostra_modo(dificuldade);

        __delay_cycles(16000000); 

        if(botaoPres == 0) 
        {
          jogo_ativo = 2;
          botaoPres = 1; 
          break; 
        }
      }
    }
    else if(jogo_ativo == 2) 
    {
      bola_x = 3;
      bola_y = 7;
      vx = -1;
      vy = 1;

      ini_timerA_game(); 

      while(1) 
      {
        if(botaoPres == 0) 
        {
          botaoPres = 1; 
          break; 
        }
      }
    }
  }
}

void desenha_raquete_esq(void)
{
    tela_esq[0] |= (1 << raquete_esq_y);
    tela_esq[0] |= (1 << raquete_esq_y + 1);
    tela_esq[0] |= (1 << raquete_esq_y + 2);
}

void desenha_raquete_dir(void)
{
    tela_dir[7] |= (1 << raquete_dir_y);
    tela_dir[7] |= (1 << raquete_dir_y + 1);
    tela_dir[7] |= (1 << raquete_dir_y + 2);
}

void desenha_raquete_esq_hard(void)
{
    tela_esq[0] |= (1 << raquete_esq_y);
    tela_esq[0] |= (1 << raquete_esq_y + 1);
}

void desenha_raquete_dir_hard(void)
{
    tela_dir[7] |= (1 << raquete_dir_y);
    tela_dir[7] |= (1 << raquete_dir_y + 1);
}

void desenha_bola(void)
{
    if(bola_y < 8)
    {
        tela_esq[bola_y] |= (1 << bola_x);
    }
    else
    {
        tela_dir[bola_y - 8] |= (1 << bola_x);
    }
}

void atualiza_bola(void)
{
    bola_x += vx; 
    bola_y += vy; 

    if(bola_x == 0 || bola_x == 7)
        vx = -vx;

    if(dificuldade == 1)
    {
        if(bola_y == 1)
        {
            if(bola_x >= (raquete_esq_y - 1) && bola_x <= (raquete_esq_y + 2))
                vy = -vy;
        }

        if(bola_y == 14)
        {
            if(bola_x >= (raquete_dir_y - 1) && bola_x <= (raquete_dir_y + 2))
                vy = -vy;
        }
    }
    else 
    {
        if(bola_y == 1)
        {
            if(bola_x >= (raquete_esq_y - 1) && bola_x <= (raquete_esq_y + 3))
                vy = -vy;
        }

        if(bola_y == 14)
        {
            if(bola_x >= (raquete_dir_y - 1) && bola_x <= (raquete_dir_y + 3))
                vy = -vy;
        }
    }

    if(bola_y == 0)
    {
      __delay_cycles(8000000);

      bola_x = 3;
      bola_y = 7;
      vy = -vy;
      pontos_2++;
    }
    else if(bola_y == 15)
    {
      __delay_cycles(8000000);
      bola_x = 3;
      bola_y = 7;
      vy = -vy;
      pontos_1++;
    }
}

void loop_pong(void)
{
    if(jogo_pausado == 0)
    {
      if(pontos_1 >= 3 || pontos_2 >= 3)
      {
            jogo_ativo = 0;
            TA1CTL &= ~MC0;
            jogo_comecando = 1;
            mostra_ponto(pontos_1, pontos_2);
      }
      else
      {
        for(i = 0; i < 8; i++)
        {
            tela_esq[i] = 0;
            tela_dir[i] = 0;
        }

        if(dificuldade == 1)
        {
            desenha_raquete_esq_hard();
            desenha_raquete_dir_hard();
        }
        else
        {
            desenha_raquete_esq();
            desenha_raquete_dir();
        }

        desenha_bola();

        for(i = 0; i < 8; i++)
        {
            spi_max(MAX_DIGIT0 + i, tela_esq[i], MAX_DIGIT0 + i, tela_dir[i]);
        }

        atualiza_bola();
      }
    }
    else if(botaoPres == 0)
    {
        jogo_pausado = 1;
        botaoPres = 1;
    }
}