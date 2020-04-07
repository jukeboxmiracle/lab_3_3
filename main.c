//Nik Pepmeyer
//Electronic Compass
//Lab 3.3

#define RIN 661961476

#include "C8051_SIM.h"
#include<stdio.h>
#include<stdlib.h>

#define ss P3_7

unsigned int ReadCompass(void);
void XBR_Init();
void PCA_ISR ( void );
void PCA_Init(void);
void Interrupt_Init(void);
void Set_Pulsewidth(void);
void SMB_Init();
void Port_Init();

unsigned int actual_heading;
unsigned int desired_heading = 1800;
unsigned int h_count = 0;
unsigned int heads;
signed int error;
unsigned int temp_servo_pw;

unsigned int PW_CENTER = 2765;
unsigned int PW_MIN = 2335;
unsigned int PW_MAX = 3305;
unsigned int PW = 0;
unsigned int PCA_Start = 28671;
unsigned int counts = 0;

void main(void){
    //unsigned int dir_count = 0;
    //unsigned int time = 0;
    Sys_Init();
    putchar(0);
    PCA_Init();
    Interrupt_Init();
    XBR_Init();
    SMB_Init();
    Port_Init();

    //PW = PW_CENTER;
	//PCA0CP0 = 65535 - PW;

    //TR0 = 1;
    //while(counts != 338);

    PW = PW_CENTER;
    PCA0CP2 = 65535 - PW;
    while(counts<50){
        Sim_Update();
    }


    while(1){
        Sim_Update();   // This function synchronizes the simulation and this program


        actual_heading = ReadCompass();
                                        // Sim_Update() needs to be called in EVERY LOOP

        if(ss == 0){

            PW = PW_CENTER;
        }
        else if (ss == 1){
            //printf("Reached");
            Set_Pulsewidth();
        }

         if(actual_heading > 999 ){
            printf("desired:  %d || actual:  %d || PW: %d \r", desired_heading, actual_heading, PW);
        }
        else if(actual_heading < 10){
           printf("desired:  %d || actual:  %d   || PW:  %d \r", desired_heading, actual_heading, PW);
        }
        else{
           printf("desired:  %d || actual:  %d  || PW:  %d \r", desired_heading, actual_heading, PW);

        }
    }


}

void PCA_Init(void){
    PCA0MD = 0x81; // SYSCLK/12, Interrupt Enable
    PCA0CN |= 0x40;
    PCA0CPM0 = 0xC2;
}

void Interrupt_Init(void){
    EIE1 |= 0x08;       // Enable PCA interrupt
    EA = 1;
    IE |= 0x02;             // Globally Enable interrupts
}

void PCA_ISR ( void ){
    if (CF){
        CF = 0;
        PCA0 = PCA_Start;
        h_count++;
        counts++;
        if(h_count >= 2){
            //new_heading = 1; // 2 overflows is about 40 ms
            h_count = 0;
        }
    }
    else {
        PCA0CN &= 0xC0;
    }
}

unsigned int ReadCompass( void ){

    unsigned char addr = 0xC0; // the address of the sensor, 0xC0 for the compass
    unsigned char Data[2]; // Data is an array with a length of 2
    unsigned int actual_heading = 0; // the heading returned in degrees between 0 and 3599
    i2c_read_data(addr, 2, Data, 2); // read two byte, starting at reg 2
    actual_heading =(((unsigned int)Data[0] << 8) | Data[1]);
    i2c_write_data(addr,2,Data,2);//combine the two values
                                //heading has units of 1/10 of a degree
    return actual_heading;             // heading returned in tenths degrees from 0 to 3599

}

void Port_Init(){
    P1MDOUT = 0x0D;
    P3MDOUT &= !0x80;
    P3 = 0x80;
}

void XBR_Init(){
    XBR0 = 0x27;
}

void SMB_Init(){
    SMB0CR = 0x93;
    ENSMB = 1;
}

//void Timer_Init(void)
//{
    //CKCON |= 0x08;   // Timer0 uses SYSCLK as source
    //TMOD &= 0xF0;;   // clear the 4 least significant bits
    //TMOD |= 0x01;    // Timer0 in mode 1 (16-bit counting)
    //TR0 = 0;         // Stop Timer0
    //TMR0 = 0;        // Clear high & low byte of T0
//}

void Set_Pulsewidth()
{
    error = actual_heading - desired_heading;
    if(error>1800){
        error = error - 3600;
    }
    else if (error<-1800){
        error = error + 3600;
    }



    PW = ((3*error)/10) + PW_CENTER;

    if(PW < PW_MIN){
        PW = PW_MIN;
    }
    else if(PW > PW_MAX){
        PW = PW_MAX;
    }


    PCA0CP0 = 0xFFFF - PW;

}

//void Timer0_ISR(void)
//{
    //counts++; // increment overflow counter
//}
