/* 
 * File:   main10.c
 * Author: Pablo Caal
 * 
 * Comunicación serial con oscilador de 1MHz y baud rate 9600
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
uint8_t bandera = 1;                // Variable para indicar si se reinicio o no el menú

/*------------------------------------------------------------------------------
 * PROTOTIPO DE FUNCIONES 
 ------------------------------------------------------------------------------*/
void setup(void);                       // Función de configuración
void Mostrar(unsigned char *cadena);    // Función para mostrar cadenas de texto
void Saltodelinea(void);                // Función para generar un salto de línea en consola
void Menu(void);                        // Función para mostrar el menu de opciones

/*------------------------------------------------------------------------------
 * INTERRUPCIONES 
 ------------------------------------------------------------------------------*/
void __interrupt() isr (void){
    if(PIR1bits.RCIF){         // Verificación de interrupción de comunicacion serial
        opcion = RCREG;             // Almacenamiento de valor ingresado en memoria
        cambio = 1;                 // Actrivación de variable indicadora de cambio en el ingreso
    }
    if(PIR1bits.ADIF){         // Verificación de interrupción del módulo ADC
        if(ADCON0bits.CHS == 0){    // Verificación de interrupción por canal AN0
            PORTB = ADRESH;         // Asignación de valor del POT en puerto B
        }
        PIR1bits.ADIF = 0;          // Limpieza de bandera de interrupción del módulo ADC
    }
    return;
}

/*------------------------------------------------------------------------------
 * CICLO PRINCIPAL
 ------------------------------------------------------------------------------*/
void main(void) {
    setup();                    // Llamado de función setup
    while(1){     
        while(bandera == 1){    // Verificación de bandera
            Menu();             // Mostrar menú de opciones
            bandera = 0;        // Apagar bandera de reinicio
            cambio = 0;         // Apagar bandera de cambio en el ingreso
        }
        
        if(cambio == 1){            // Verificación de cambio
            while (TXIF != 1);      // Verificación de registro TXREG
            TXREG = opcion;         // Asignación de valor ingresado en registro TXREG
            Saltodelinea();    
            
            if(opcion == 48){           // Opción "0" del menú
                Saltodelinea();
                Mostrar("Lectura de potenciómetro activa.");
                Saltodelinea();
                Mostrar("Presione cualquier tecla para reiniciar menu.");
                Saltodelinea();
                
                while(opcion == 48){            // LECTURA DE POTENCIÓMETRO
                    if(ADCON0bits.GO == 0){          // Si no hay proceso de conversión
                        ADCON0bits.CHS = 0b0000;     // Selección de canal
                        __delay_us(40);              // Delay de tiempo de adquisición
                        ADCON0bits.GO = 1;           // Ejecución de proceso de conversión
                    }
                }
                
            }
            else if(opcion == 49){      // Opción "1" del menú
                Saltodelinea();
                Mostrar("Envío de valor ASCCI al puerto B activo.");
                Saltodelinea();
                Mostrar("Ingrese un espacio ' ' para reiniciar menu.");
                Saltodelinea();
                
                while(opcion != 32){            // ENVÍO DE ASCCI A PORTB
                    PORTB = opcion;                 // Asignación de valor al PORTB
                }
            } 
            else{                       // Opción cualquiera que no esté en el menú
                valor_old = opcion;
                Saltodelinea();
                Mostrar("La opción seleccionada no está en el menú.");
                Saltodelinea();
                Mostrar("Presione cualquier tecla para continuar.");
                Saltodelinea();
                while(valor_old == opcion);
            }
            
            while (TXIF != 1);          // Verificación de registro TXREG
            TXREG = 0x0C;               // Asignación de valor ingresado en registro TXREG
            bandera = 1;                // Activar bandera de reinicio de menú
        }        
    }
    return;
}

/*------------------------------------------------------------------------------
 * CONFIGURACION 
 ------------------------------------------------------------------------------*/
void setup(void){
    ANSEL = 0b001;              // AN0 como entrada analógicas
    ANSELH = 0x00;              // I/O digitales
    TRISA = 0b001;              // AN0 como entrada
    TRISB = 0x00;               // PORTB como salidas
    PORTA = 0x00;               // Limpieza del PORTA
    PORTB = 0x00;               // Limpieza del PORTB
    
    // Configuración del oscilador
    OSCCONbits.IRCF = 0b0100;   // 1 MHz
    OSCCONbits.SCS = 1;         // Oscilador interno
    
    // Configuraciones de comunicacion serial
    // SYNC = 0, BRGH = 1, BRG16 = 1, SPBRG=25 <- Valores de tabla 12-5
    TXSTAbits.SYNC = 0;         // Comunicación ascincrona (full-duplex)
    TXSTAbits.BRGH = 1;         // Baud rate de alta velocidad 
    BAUDCTLbits.BRG16 = 1;      // 16-bits para generar el baud rate
    
    SPBRGH = 0;
    SPBRG = 25;                 // Baud rate ~9600, error -> 0.16%
    
    RCSTAbits.SPEN = 1;         // Habilitar comunicacion serial
    RCSTAbits.RX9 = 0;          // Únicamente 8 bits
    TXSTAbits.TXEN = 1;         // Habilitar transmisor
    RCSTAbits.CREN = 1;         // Habilitar receptor
    INTCONbits.GIE = 1;         // Habilitar interrupciones globales
    INTCONbits.PEIE = 1;        // Habilitar interrupciones de periféricos
    PIE1bits.RCIE = 1;          // Habilitar interrupciones de recepción
    PIE1bits.ADIE = 1;          // Habilitamos interrupcion de ADC
    PIR1bits.ADIF = 0;          // Limpiamos bandera de ADC
    
    // Configuración ADC
    ADCON0bits.ADCS = 0b01;         // Fosc/8
    ADCON1bits.VCFG0 = 0;           // VDD
    ADCON1bits.VCFG1 = 0;           // VSS
    ADCON0bits.CHS = 0b0000;        // Selección de canal AN0
    ADCON1bits.ADFM = 0;            // Configuración de justificado a la izquierda
    ADCON0bits.ADON = 1;            // Habilitación del modulo ADC
    __delay_us(40);                 // Display de sample time
}

/*------------------------------------------------------------------------------
 * FUNCIONES 
 ------------------------------------------------------------------------------*/
// FUNCIÓN PARA MOSTRAR CADENAS DE TEXTO 
void Mostrar(unsigned char *cadena){    
    while (*cadena != '\0'){
        while (TXIF != 1);
        TXREG = *cadena;
        *cadena++;
    }    
}

//FUNCIÓN PARA MOSTRAR UN SALTO DE LÍNEA EN CONSOLA
void Saltodelinea(void)
{ 
    while (TXIF != 1);
    TXREG = 0x0D;  
}

// FUNCIÓN PARA MOSTRAR EL MENÚ EN CONSOLA
void Menu(void)
{ 
    Mostrar("Seleccione la acción que desea realizar");    
    Saltodelinea();
    Mostrar("0. Leer potenciómetro");
    Saltodelinea();
    Mostrar("1. Enviar ASCCI al puerto B");
    Saltodelinea();
    Mostrar("Opción: "); 
}