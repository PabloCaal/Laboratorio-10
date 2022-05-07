/* 
 * File:   main10.c
 * Author: Pablo Caal
 * 
 * Comunicaci�n serial con oscilador de 1MHz y baud rate 9600
 *
 * Created on 04 de mayo de 2022, 03:30 PM
 */

// PIC16F887 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT        // Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF                   // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF                  // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF                  // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF                     // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF                    // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF                  // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF                   // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF                  // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF                    // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V               // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF                    // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdint.h>
#include<stdio.h>

/*------------------------------------------------------------------------------
 * CONSTANTES 
 ------------------------------------------------------------------------------*/
#define _XTAL_FREQ 1000000

/*------------------------------------------------------------------------------
 * VARIABLES 
 ------------------------------------------------------------------------------*/
uint8_t opcion = 0;                 // Variable para almacenar valor ingresado en consola
uint8_t cambio = 0;                 // Variable para indicar si hubo un cambio en el valor ingresado
uint8_t valor_old = 0;              // Variable para almacenar el valor antiguo ingresado
uint8_t bandera = 1;                // Variable para indicar si se reinicio o no el men�

/*------------------------------------------------------------------------------
 * PROTOTIPO DE FUNCIONES 
 ------------------------------------------------------------------------------*/
void setup(void);                       // Funci�n de configuraci�n
void Mostrar(unsigned char *cadena);    // Funci�n para mostrar cadenas de texto
void Saltodelinea(void);                // Funci�n para generar un salto de l�nea en consola
void Menu(void);                        // Funci�n para mostrar el menu de opciones

/*------------------------------------------------------------------------------
 * INTERRUPCIONES 
 ------------------------------------------------------------------------------*/
void __interrupt() isr (void){
    if(PIR1bits.RCIF){         // Verificaci�n de interrupci�n de comunicacion serial
        opcion = RCREG;             // Almacenamiento de valor ingresado en memoria
        cambio = 1;                 // Actrivaci�n de variable indicadora de cambio en el ingreso
    }
    if(PIR1bits.ADIF){         // Verificaci�n de interrupci�n del m�dulo ADC
        if(ADCON0bits.CHS == 0){    // Verificaci�n de interrupci�n por canal AN0
            PORTB = ADRESH;         // Asignaci�n de valor del POT en puerto B
        }
        PIR1bits.ADIF = 0;          // Limpieza de bandera de interrupci�n del m�dulo ADC
    }
    return;
}

/*------------------------------------------------------------------------------
 * CICLO PRINCIPAL
 ------------------------------------------------------------------------------*/
void main(void) {
    setup();                    // Llamado de funci�n setup
    while(1){     
        while(bandera == 1){    // Verificaci�n de bandera
            Menu();             // Mostrar men� de opciones
            bandera = 0;        // Apagar bandera de reinicio
            cambio = 0;         // Apagar bandera de cambio en el ingreso
        }
        
        if(cambio == 1){            // Verificaci�n de cambio
            while (TXIF != 1);      // Verificaci�n de registro TXREG
            TXREG = opcion;         // Asignaci�n de valor ingresado en registro TXREG
            Saltodelinea();    
            
            if(opcion == 48){           // Opci�n "0" del men�
                Saltodelinea();
                Mostrar("Lectura de potenci�metro activa.");
                Saltodelinea();
                Mostrar("Presione cualquier tecla para reiniciar menu.");
                Saltodelinea();
                
                while(opcion == 48){            // LECTURA DE POTENCI�METRO
                    if(ADCON0bits.GO == 0){          // Si no hay proceso de conversi�n
                        ADCON0bits.CHS = 0b0000;     // Selecci�n de canal
                        __delay_us(40);              // Delay de tiempo de adquisici�n
                        ADCON0bits.GO = 1;           // Ejecuci�n de proceso de conversi�n
                    }
                }
                
            }
            else if(opcion == 49){      // Opci�n "1" del men�
                Saltodelinea();
                Mostrar("Env�o de valor ASCCI al puerto B activo.");
                Saltodelinea();
                Mostrar("Ingrese un espacio ' ' para reiniciar menu.");
                Saltodelinea();
                
                while(opcion != 32){            // ENV�O DE ASCCI A PORTB
                    PORTB = opcion;                 // Asignaci�n de valor al PORTB
                }
            } 
            else{                       // Opci�n cualquiera que no est� en el men�
                valor_old = opcion;
                Saltodelinea();
                Mostrar("La opci�n seleccionada no est� en el men�.");
                Saltodelinea();
                Mostrar("Presione cualquier tecla para continuar.");
                Saltodelinea();
                while(valor_old == opcion);
            }
            
            while (TXIF != 1);          // Verificaci�n de registro TXREG
            TXREG = 0x0C;               // Asignaci�n de valor ingresado en registro TXREG
            bandera = 1;                // Activar bandera de reinicio de men�
        }        
    }
    return;
}

/*------------------------------------------------------------------------------
 * CONFIGURACION 
 ------------------------------------------------------------------------------*/
void setup(void){
    ANSEL = 0b001;              // AN0 como entrada anal�gicas
    ANSELH = 0x00;              // I/O digitales
    TRISA = 0b001;              // AN0 como entrada
    TRISB = 0x00;               // PORTB como salidas
    PORTA = 0x00;               // Limpieza del PORTA
    PORTB = 0x00;               // Limpieza del PORTB
    
    // Configuraci�n del oscilador
    OSCCONbits.IRCF = 0b0100;   // 1 MHz
    OSCCONbits.SCS = 1;         // Oscilador interno
    
    // Configuraciones de comunicacion serial
    // SYNC = 0, BRGH = 1, BRG16 = 1, SPBRG=25 <- Valores de tabla 12-5
    TXSTAbits.SYNC = 0;         // Comunicaci�n ascincrona (full-duplex)
    TXSTAbits.BRGH = 1;         // Baud rate de alta velocidad 
    BAUDCTLbits.BRG16 = 1;      // 16-bits para generar el baud rate
    
    SPBRGH = 0;
    SPBRG = 25;                 // Baud rate ~9600, error -> 0.16%
    
    RCSTAbits.SPEN = 1;         // Habilitar comunicacion serial
    RCSTAbits.RX9 = 0;          // �nicamente 8 bits
    TXSTAbits.TXEN = 1;         // Habilitar transmisor
    RCSTAbits.CREN = 1;         // Habilitar receptor
    INTCONbits.GIE = 1;         // Habilitar interrupciones globales
    INTCONbits.PEIE = 1;        // Habilitar interrupciones de perif�ricos
    PIE1bits.RCIE = 1;          // Habilitar interrupciones de recepci�n
    PIE1bits.ADIE = 1;          // Habilitamos interrupcion de ADC
    PIR1bits.ADIF = 0;          // Limpiamos bandera de ADC
    
    // Configuraci�n ADC
    ADCON0bits.ADCS = 0b01;         // Fosc/8
    ADCON1bits.VCFG0 = 0;           // VDD
    ADCON1bits.VCFG1 = 0;           // VSS
    ADCON0bits.CHS = 0b0000;        // Selecci�n de canal AN0
    ADCON1bits.ADFM = 0;            // Configuraci�n de justificado a la izquierda
    ADCON0bits.ADON = 1;            // Habilitaci�n del modulo ADC
    __delay_us(40);                 // Display de sample time
}

/*------------------------------------------------------------------------------
 * FUNCIONES 
 ------------------------------------------------------------------------------*/
// FUNCI�N PARA MOSTRAR CADENAS DE TEXTO 
void Mostrar(unsigned char *cadena){    
    while (*cadena != '\0'){
        while (TXIF != 1);
        TXREG = *cadena;
        *cadena++;
    }    
}

//FUNCI�N PARA MOSTRAR UN SALTO DE L�NEA EN CONSOLA
void Saltodelinea(void)
{ 
    while (TXIF != 1);
    TXREG = 0x0D;  
}

// FUNCI�N PARA MOSTRAR EL MEN� EN CONSOLA
void Menu(void)
{ 
    Mostrar("Seleccione la acci�n que desea realizar");    
    Saltodelinea();
    Mostrar("0. Leer potenci�metro");
    Saltodelinea();
    Mostrar("1. Enviar ASCCI al puerto B");
    Saltodelinea();
    Mostrar("Opci�n: "); 
}