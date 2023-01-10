/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

#define Test 1

#define DELAYUS 100
#define DIR1RE 2
#define DIR2DE 1

// find substring in string
bool includeSubStr(char* str, uint8_t lengStr, char* subStr, uint8_t lengSubStr) {
    int i, j;
    if (lengStr - lengSubStr + 1 < 0) return false;
    for (i = 0; i <= lengStr - lengSubStr; i++) {
        for (j = 0; j < lengSubStr && str[i + j] == subStr[j]; j++);
        if (j == lengSubStr) return true;
    }
    return false;
}

char charToByte(char Char) {
    switch (Char) {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;
        case 'A': return 10;
        case 'B': return 11;
        case 'C': return 12;
        case 'D': return 13;
        case 'E': return 14;
        case 'F': return 15;
        default: return 0;
    }
}

// Longitudinal Redundancy Check
static char calculateLRC(char* bytes, char length) {
    char LRC = 0;
    int i;
    for (i = 1; i < length; i += 2) {
        LRC -= (charToByte(bytes[i]) << 4) + charToByte(bytes[i + 1]);
    }
    return LRC;
}

//PIC32MX575F512H
///Optic to RS-485 converter
int main() {
    SYS_Initialize(NULL); 

//    TRISEbits.TRISE1 = 0; // DE
//    TRISEbits.TRISE2 = 0; // ~RE
//    TRISBbits.TRISB0 = 0; // LED
//    PORTE &= ~((1 << DIR2DE) | (1 << DIR1RE)); // recieve mode

    bool uart1break = false;
    bool uart3break = false;
    int count = 0;
    uint32_t i = 0;
    char ReceivedChar1;
    char ReceivedChar2;
    char ReceivedChar3;
    uint8_t UART1buf[3000];
    char UART1command[2000];
    size_t UART1ReadCount;
    uint8_t UART2buf[3000];
    char UART2command[2000];
    size_t UART2ReadCount;
    char UART3buf[2000];
    uint8_t UART1bufCursor = 0;
    uint8_t UART2bufCursor = 0;
    uint8_t UART3bufCursor = 0;
    uint8_t UART1CommandCursor = 0;
    uint8_t UART2CommandCursor = 0;
                        //0         1         2         3
                        //0123456789012345678901234567890
    char UIQ[] =        ":0104001D0006D8\r\n"; // I1...I3, U1...U3 Question
    char UIR[] =        ":010408000100020003000400050006DE\r\n"; // I1...I3, U1...U3 Response
    char scanCBQ[] =    ":010400000001FA\r\n"; // check Circuit Breakers before break
    char scanCBR[] =    ":010402FFFFFB\r\n";
    char breakCB1Q[] =  ":01100000000102FFFEEF\r\n"; // break CB1
    char breakCB1R[] =  ":011000000001EE\r\n";
    char breakCB2Q[] =  ":01100000000102FFFDF0\r\n"; // break CB2
    char breakCB2R[] =  ":011000000001EE\r\n";
    char breakCB3Q[] =  ":01100000000102FFFBF2\r\n"; // break CB3
    char breakCB3R[] =  ":011000000001EE\r\n";
    char CircuitBreakersQ[] = ":010200000010ED\r\n"; // 
    char CircuitBreakersR[] = ":0102020000FB\r\n";
    char B1[] = ":B1=0\r\n"; // Circuit Breaker 1
    char B2[] = ":B2=0\r\n"; // Circuit Breaker 1
    char B3[] = ":B3=0\r\n"; // Circuit Breaker 1
    char LRC;
    unsigned int uiValue;
    signed int siValue;
    int I1ch1 = 0, I2ch1 = 0, I3ch1 = 0;
    int I1ch2 = 0, I2ch2 = 0, I3ch2 = 0;
    unsigned int U1ch1 = 0, U2ch1 = 0, U3ch1 = 0;
    unsigned int U1ch2 = 0, U2ch2 = 0, U3ch2 = 0;
    char I1[] = ":I1=-1000\r\n"; // I1
    char I2[] = ":I2=+0101\r\n"; // I2
    char I3[] = ":I3=+3000\r\n"; // I3
    char U1[] = ":U1=010\r\n"; // U1
    char U2[] = ":U2=500\r\n"; // U2
    char U3[] = ":U3=999\r\n"; // U3
    char hex[] = "0123456789ABCDEF";
    unsigned int counterWaitUartI1 = 0;
    unsigned int counterWaitUartI2 = 0;
    unsigned int counterWaitUartI3 = 0;
    unsigned int counterWaitUartU1 = 0;
    unsigned int counterWaitUartU2 = 0;
    unsigned int counterWaitUartU3 = 0;
    unsigned int counterWaitUartB1 = 0;
    unsigned int counterWaitUartB2 = 0;
    unsigned int counterWaitUartB3 = 0;
    unsigned int timeoutdata = 2000;
    
    uint32_t startCountUART = 0;
    uint32_t delay_msUART = 1;
    uint32_t endCountUART = (CORE_TIMER_FREQUENCY/1000)*delay_msUART;
    
    CORETIMER_Start();    
    //PORTBbits.RB0 = 0; // LED
    
    signed int temp = 0;

    while (1) {
        SYS_Tasks(); // Maintain state machines of all polled MPLAB Harmony modules. 
        //WDT_CLR();
        
        if ((_CP0_GET_COUNT() - startCountUART) > endCountUART) { // timer spi        
            startCountUART = _CP0_GET_COUNT();          
            counterWaitUartI1++;
            counterWaitUartI2++;
            counterWaitUartI3++;
            counterWaitUartU1++;
            counterWaitUartU2++;
            counterWaitUartU3++;
            counterWaitUartB1++;
            counterWaitUartB2++;
            counterWaitUartB3++;
            if (counterWaitUartI1 > timeoutdata) { counterWaitUartI1 = timeoutdata; I1ch1 = 0; }
            if (counterWaitUartI2 > timeoutdata) { counterWaitUartI2 = timeoutdata; I2ch1 = 0; }
            if (counterWaitUartI3 > timeoutdata) { counterWaitUartI3 = timeoutdata; I3ch1 = 0; }
            if (counterWaitUartU1 > timeoutdata) { counterWaitUartU1 = timeoutdata; U1ch1 = 0; }
            if (counterWaitUartU2 > timeoutdata) { counterWaitUartU2 = timeoutdata; U2ch1 = 0; }
            if (counterWaitUartU3 > timeoutdata) { counterWaitUartU3 = timeoutdata; U3ch1 = 0; }
            if (counterWaitUartB1 > timeoutdata) { counterWaitUartB1 = timeoutdata; B1[4] = '0'; }
            if (counterWaitUartB2 > timeoutdata) { counterWaitUartB2 = timeoutdata; B2[4] = '0'; }
            if (counterWaitUartB3 > timeoutdata) { counterWaitUartB3 = timeoutdata; B3[4] = '0'; }
        }

        UART1ReadCount = UART1_ReadCountGet();
        if (UART1ReadCount > 0  && UART1_Read(UART1buf, UART1ReadCount)) { // UART1 GPT1
            for (i = 0; i < UART1ReadCount; i++, UART1CommandCursor++) {
                if (UART1buf[i] == ':') UART1CommandCursor = 0;
                UART1command[UART1CommandCursor] = UART1buf[i];
                if (UART1command[UART1CommandCursor] == '\n') { // end of received command
                    PORTBbits.RB0 ^= 1; // LED                       
                    if (includeSubStr(UART1command, UART1CommandCursor, B1, 3)) {
                        counterWaitUartB1 = 0;
                        B1[4] = (UART1command[4] == '1') ? '1' : '0'; // condition of CB1                   
                        continue;
                    }
                    if (includeSubStr(UART1command, UART1CommandCursor, B2, 3)) {
                        counterWaitUartB2 = 0;
                        B2[4] = (UART1command[4] == '1') ? '1' : '0'; // condition of CB2                   
                        continue;
                    }
                    if (includeSubStr(UART1command, UART1CommandCursor, I1, 3)) {
                        counterWaitUartI1 = 0;
                        for (uiValue = 4; uiValue <= 8; uiValue++) I1[uiValue] = UART1command[uiValue]; 
                        I1ch1 = (charToByte(I1[5])*1000 + charToByte(I1[6])*100 + charToByte(I1[7])*10 + charToByte(I1[8]));                   
                        if (I1[4] == '-') I1ch1 = -I1ch1;   
                        continue;
                    }
                    if (includeSubStr(UART1command, UART1CommandCursor, I2, 3)) {
                        counterWaitUartI2 = 0;
                        for (uiValue = 4; uiValue <= 8; uiValue++) I2[uiValue] = UART1command[uiValue]; 
                        I2ch1 = (charToByte(I2[5])*1000 + charToByte(I2[6])*100 + charToByte(I2[7])*10 + charToByte(I2[8]));                   
                        if (I2[4] == '-') I2ch1 = -I2ch1;     
                        continue;
                    }
                    if (includeSubStr(UART1command, UART1CommandCursor, U1, 3)) {
                        counterWaitUartU1 = 0;
                        for (uiValue = 4; uiValue <= 6; uiValue++) U1[uiValue] = UART1command[uiValue]; 
                        U1ch1 = charToByte(U1[4])*100 + charToByte(U1[5])*10 + charToByte(U1[6]);   
                        continue;
                    }
                    if (includeSubStr(UART1command, UART1CommandCursor, U2, 3)) {
                        counterWaitUartU2 = 0;
                        for (uiValue = 4; uiValue <= 6; uiValue++) U2[uiValue] = UART1command[uiValue]; 
                        U2ch1 = charToByte(U2[4])*100 + charToByte(U2[5])*10 + charToByte(U2[6]);   
                        continue;
                    }
                }
            }
        }
              
        UART2ReadCount = UART2_ReadCountGet();
        if (UART2ReadCount > 0  && UART2_Read(UART2buf, UART2ReadCount)) { // UART2 GPT2
            for (i = 0; i < UART2ReadCount; i++, UART2CommandCursor++) {
                if (UART2buf[i] == ':') UART2CommandCursor = 0;
                UART2command[UART2CommandCursor] = UART2buf[i];
                if (UART2command[UART2CommandCursor] == '\n') { // end of received command
                    PORTBbits.RB0 ^= 1; // LED                       
                    if (includeSubStr(UART2command, UART2CommandCursor, B1, 3)) {
                        counterWaitUartB3 = 0;
                        B3[4] = (UART2command[4] == '1') ? '1' : '0'; // condition of CB1                   
                        continue;
                    }
                    if (includeSubStr(UART2command, UART2CommandCursor, B2, 3)) {
                        counterWaitUartB2 = 0;
                        B2[4] = (UART2command[4] == '1') ? '1' : '0'; // condition of CB2                   
                        continue;
                    }
                    if (includeSubStr(UART2command, UART2CommandCursor, I1, 3)) {
                        counterWaitUartI3 = 0;
                        for (uiValue = 4; uiValue <= 8; uiValue++) I1[uiValue] = UART2command[uiValue]; 
                        I3ch1 = (charToByte(I1[5])*1000 + charToByte(I1[6])*100 + charToByte(I1[7])*10 + charToByte(I1[8]));                   
                        if (I1[4] == '-') I3ch1 = -I3ch1;   
                        continue;
                    }
                    if (includeSubStr(UART2command, UART2CommandCursor, I2, 3)) {
                        counterWaitUartI2 = 0;
                        for (uiValue = 4; uiValue <= 8; uiValue++) I2[uiValue] = UART2command[uiValue]; 
                        I2ch1 = (charToByte(I2[5])*1000 + charToByte(I2[6])*100 + charToByte(I2[7])*10 + charToByte(I2[8]));                   
                        if (I2[4] == '-') I2ch1 = -I2ch1;     
                        continue;
                    }
                    if (includeSubStr(UART2command, UART2CommandCursor, U1, 3)) {
                        counterWaitUartU3 = 0;
                        for (uiValue = 4; uiValue <= 6; uiValue++) U3[uiValue] = UART2command[uiValue]; 
                        U3ch1 = charToByte(U3[4])*100 + charToByte(U3[5])*10 + charToByte(U3[6]);   
                        continue;
                    }
                    if (includeSubStr(UART2command, UART2CommandCursor, U2, 3)) {
                        counterWaitUartU2 = 0;
                        for (uiValue = 4; uiValue <= 6; uiValue++) U2[uiValue] = UART2command[uiValue]; 
                        U2ch1 = charToByte(U2[4])*100 + charToByte(U2[5])*10 + charToByte(U2[6]);   
                        continue;
                    }
                }
            }
        }
        
        while (UART3_Read(&ReceivedChar3, 1) == true) { // read UART3 buffer (panel)            
            if (ReceivedChar3 == ':') UART3bufCursor = 0;
            UART3buf[UART3bufCursor] = ReceivedChar3;
            UART3bufCursor++;
            if (ReceivedChar3 == '\n') { // end of received command
                if (includeSubStr(UART3buf, UART3bufCursor, scanCBQ, sizeof (scanCBQ) - 1)) {
                    PORTE |= ((1 << DIR2DE) | (1 << DIR1RE)); // transmit mode
                    CORETIMER_DelayUs(DELAYUS);
                    UART3_Write(&scanCBR[0], sizeof (scanCBR) - 1); //work good
                    while (!U3STAbits.TRMT); // Transmit Shift Register is Empty bit (read-only)
                    PORTE &= ~((1 << DIR2DE) | (1 << DIR1RE)); // recieve mode
                    break;
                }

                if (includeSubStr(UART3buf, UART3bufCursor, breakCB1Q, sizeof (breakCB1Q) - 1)) {
                    PORTE |= ((1 << DIR2DE) | (1 << DIR1RE)); // transmit mode
                    CORETIMER_DelayUs(DELAYUS);
                    UART3_Write(&breakCB1R[0], sizeof (breakCB1R) - 1); //work good
                    while (!U3STAbits.TRMT); // Transmit Shift Register is Empty bit (read-only)
                    PORTE &= ~((1 << DIR2DE) | (1 << DIR1RE)); // recieve mode
                    UART1_Write(&breakCB1Q[0], sizeof (breakCB1Q) - 1);
                    UART2_Write(&breakCB1Q[0], sizeof (breakCB1Q) - 1);
                    break;
                }

                if (includeSubStr(UART3buf, UART3bufCursor, breakCB2Q, sizeof (breakCB2Q) - 1)) {
                    PORTE |= ((1 << DIR2DE) | (1 << DIR1RE)); // transmit mode
                    CORETIMER_DelayUs(DELAYUS);
                    UART3_Write(&breakCB2R[0], sizeof (breakCB2R) - 1); //work good
                    //while (UART3_WriteIsBusy());                  
                    //CORETIMER_DelayUs(DELAYUS/1);
                    while (!U3STAbits.TRMT); // Transmit Shift Register is Empty bit (read-only)
                    PORTE &= ~((1 << DIR2DE) | (1 << DIR1RE)); // recieve mode
//                    PORTB |= (1 << 0); // LED ON
                    //temp[0] = '0';
                    UART1_Write(&breakCB2Q[0], sizeof (breakCB2Q) - 1);
                    //while (!U1STAbits.TRMT); // Transmit Shift Register is Empty bit (read-only)
                    UART2_Write(&breakCB2Q[0], sizeof (breakCB2Q) - 1);
                    //while (!U2STAbits.TRMT); // Transmit Shift Register is Empty bit (read-only)
                    break;
                }

                if (includeSubStr(UART3buf, UART3bufCursor, breakCB3Q, sizeof (breakCB3Q) - 1)) {
                    PORTE |= ((1 << DIR2DE) | (1 << DIR1RE)); // transmit mode
                    CORETIMER_DelayUs(DELAYUS);
                    UART3_Write(&breakCB3R[0], sizeof (breakCB3R) - 1); //work good
                    while (!U3STAbits.TRMT); // Transmit Shift Register is Empty bit (read-only)
                    PORTE &= ~((1 << DIR2DE) | (1 << DIR1RE)); // recieve mode
//                    PORTB &= ~(1 << 0); // LED OFF
                    UART1_Write(&breakCB3Q[0], sizeof (breakCB3Q) - 1);
                    UART2_Write(&breakCB3Q[0], sizeof (breakCB3Q) - 1);
                    break;
                }

                if (includeSubStr(UART3buf, UART3bufCursor, UIQ, sizeof (UIQ) - 1)) {
                    CORETIMER_DelayUs(DELAYUS);
                    PORTE |= ((1 << DIR2DE) | (1 << DIR1RE)); // transmit mode
                    CORETIMER_DelayUs(DELAYUS);

                    // I1
                    siValue = I1ch1;
                    UIR[10] = hex[siValue & 0xF];
                    siValue = siValue >> 4;
                    UIR[9] = hex[siValue & 0xF];
                    siValue = siValue >> 4;
                    UIR[8] = hex[siValue & 0xF];
                    siValue = siValue >> 4;
                    UIR[7] = hex[siValue & 0xF];

                    //I2
                    siValue = I2ch1; //rand() % 10000 - 5000;
                    UIR[14] = hex[siValue & 0xF];
                    siValue = siValue >> 4;
                    UIR[13] = hex[siValue & 0xF];
                    siValue = siValue >> 4;
                    UIR[12] = hex[siValue & 0xF];
                    siValue = siValue >> 4;
                    UIR[11] = hex[siValue & 0xF];

                    //I3
                    siValue = I3ch1;
                    UIR[18] = hex[siValue & 0xF];
                    siValue = siValue >> 4;
                    UIR[17] = hex[siValue & 0xF];
                    siValue = siValue >> 4;
                    UIR[16] = hex[siValue & 0xF];
                    siValue = siValue >> 4;
                    UIR[15] = hex[siValue & 0xF];

                    //U1
                    siValue = U1ch1; //rand() % 800;
                    UIR[22] = hex[siValue & 0xF];
                    siValue = siValue >> 4;
                    UIR[21] = hex[siValue & 0xF];
                    siValue = siValue >> 4;
                    UIR[20] = hex[siValue & 0xF];
                    siValue = siValue >> 4;
                    UIR[19] = hex[siValue & 0xF];

                    //U2
                    siValue = U2ch1;
                    UIR[26] = hex[siValue & 0xF];
                    siValue = siValue >> 4;
                    UIR[25] = hex[siValue & 0xF];
                    siValue = siValue >> 4;
                    UIR[24] = hex[siValue & 0xF];
                    siValue = siValue >> 4;
                    UIR[23] = hex[siValue & 0xF];

                    //U3
                    siValue = U3ch1; 
                    UIR[30] = hex[siValue & 0xF];
                    siValue = siValue >> 4;
                    UIR[29] = hex[siValue & 0xF];
                    siValue = siValue >> 4;
                    UIR[28] = hex[siValue & 0xF];
                    siValue = siValue >> 4;
                    UIR[27] = hex[siValue & 0xF];

                    LRC = calculateLRC(UIR, 30);
                    UIR[32] = hex[LRC & 0xF];
                    LRC = LRC >> 4;
                    UIR[31] = hex[LRC & 0xF];

                    UART3_Write(&UIR[0], sizeof (UIR) - 1); //work good
                    while (!U3STAbits.TRMT); // Transmit Shift Register is Empty bit (read-only)
                    PORTE &= ~((1 << DIR2DE) | (1 << DIR1RE)); // recieve mode 
                    break;
                }

                if (includeSubStr(UART3buf, UART3bufCursor, CircuitBreakersQ, sizeof (CircuitBreakersQ) - 1)) {
                    PORTE |= ((1 << DIR2DE) | (1 << DIR1RE)); // transmit mode
                    CORETIMER_DelayUs(DELAYUS);
                    uiValue = 0xFFFF;
                    if (B1[4] == '0') uiValue &= ~(1 << 8);  // RSO-1 OFF
                    if (B2[4] == '0') uiValue &= ~(1 << 9);  // RSO-2 OFF 
                    if (B3[4] == '0') uiValue &= ~(1 << 10); // RSO-3 OFF
                    CircuitBreakersR[10] = hex[uiValue & 0xF];
                    uiValue = uiValue >> 4;
                    CircuitBreakersR[9] = hex[uiValue & 0xF];
                    uiValue = uiValue >> 4;
                    CircuitBreakersR[8] = hex[uiValue & 0xF];
                    uiValue = uiValue >> 4;
                    CircuitBreakersR[7] = hex[uiValue & 0xF];

                    LRC = calculateLRC(CircuitBreakersR, 11);
                    CircuitBreakersR[12] = hex[LRC & 0xF];
                    LRC = LRC >> 4;
                    CircuitBreakersR[11] = hex[LRC & 0xF];

                    UART3_Write(&CircuitBreakersR[0], sizeof (CircuitBreakersR) - 1); //work good
                    while (!U3STAbits.TRMT); // Transmit Shift Register is Empty bit (read-only)
                    PORTE &= ~((1 << DIR2DE) | (1 << DIR1RE)); // recieve mode  
                    break;
                }
            }
        }       
    }
    return 0;
}