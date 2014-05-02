/* 
 * File:   main.c
 * Author: g.lagrange
 *
 * Created on 11 avril 2014, 08:31
 */

#define USE_OR_MASKS
#define EAUSART_V3
#define _XTAL_FREQ 8000000

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <plib/usart.h>

#define RS232_CONFIG    USART_TX_INT_OFF | USART_RX_INT_OFF | USART_ASYNCH_MODE\
             | USART_EIGHT_BIT | USART_CONT_RX | USART_BRGH_HIGH
#define RS232_SPBRG     207
#define RS232_BAUD      BAUD_16_BIT_RATE | BAUD_AUTO_OFF | BAUD_IDLE_CLK_LOW

#pragma config  OSC = INTIO67, BOREN = OFF, PWRT = ON, WDT = OFF, DEBUG = ON, LVP = OFF

#define ROW_1       PORTBbits.RB6   //K
#define ROW_2       PORTBbits.RB5   //J
#define ROW_3       PORTBbits.RB4   //H
#define ROW_4       PORTBbits.RB3   //G
#define COLUMN_1    TRISBbits.RB2   //F
#define COLUMN_2    TRISBbits.RB1   //E
#define COLUMN_3    TRISBbits.RB0   //D

void putch(char data);
char getch();
char getche();

void interrupt ISR();

char column_number = 0;
char keypad[3] = {0};
char transmitCounter = 0;

int main(int argc, char** argv) {
    OSCCON = 0xF7; //0b11110111;
    TRISC = 0x80; //RX: input, TX: output
    TRISB = 0x10; //Columns and rows = inputs, except column 1 = output
    LATB = 0x00;
    CloseUSART();
    OpenUSART(RS232_CONFIG, RS232_SPBRG);
    baudUSART(RS232_BAUD);
    INTCON = 0xA0; //GIE = 1, TMR0IE = 1, TMR0IF = 0
    while (1) {

    }
    return (EXIT_SUCCESS);
}

void interrupt ISR() {
    char i = 0, j = 0;
    char buttonToSend = 0;
    if (INTCON & (1 << 2)) { //TMR0IF
        INTCON &= ~(1 << 2); //Clear TMR0IF

        switch (column_number) {
            case 0:
                COLUMN_1 = 0;
                COLUMN_2 = 1;
                COLUMN_3 = 1;
                break;
            case 1:
                COLUMN_1 = 1;
                COLUMN_2 = 0;
                COLUMN_3 = 1;
                break;
            case 2:
                COLUMN_1 = 1;
                COLUMN_2 = 1;
                COLUMN_3 = 0;
                break;
        }
        keypad[column_number] = (ROW_1) | ((ROW_2) << 1) | ((ROW_3) << 2) | ((ROW_4) << 3);

        if (column_number == 2) {
            column_number = 0;
        } else {
            column_number++;
        }

        if (transmitCounter == 2) { //do not transmit until done scanning            
            for (i = 0; i < 3; i++) {
                while (!buttonToSend && j < 4) {
                    if (keypad[i] & (1 << j)) {
                        buttonToSend = j; //row number
                        buttonToSend |= i << 2; //columns and rows
                    }
                    j++;
                }
                if (buttonToSend) {
                    WriteUSART(buttonToSend);
                }
            }
            transmitCounter = 0;
        }
        transmitCounter++;
    }
}

void putch(char data) {
    while (!TXIF);
    TXREG = data;
}

char getch() {
    while (!RCIF);
    return (RCREG);
}

char getche() {
    return (getch());
}