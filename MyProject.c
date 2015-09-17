// This code has been configured with SHE879 Clock kit
// AKIT112 Aitendo has been verified.
// 2015.Aug.19 Ver1.0 Shigemal
// 2015.Aug.23 Ver3.0 Shigemal (Timer usage has been changed)
// 2015.Aug.23 Ver6.0 Shigemal (Timer0 post process method has beenchanged as counter type)
// Ver8
// Ver9 Delay logic may cause lost interrupt
// 2015.Aug.26 Ver10.0 Shigemal (Comment update)
// 12MHz crystal has been used on this clock, so 1 machine cycle is 1us
// Ver12 Timer0 used by mode2 8bit (250us)
//  Timer1 used by mode1 16bit 4ms sampling LED scanning
//  Sampling continuous right timer250us bit4 by 4ms sampling.
// It makes accurate 8ms timing with double sample.
// Ver13 Tiemr0 250us Timer1 2.5ms for oversamplng on 8ms timing.
//
// Target board definition
#define AKIT112
//#define SHE879
//
// System Parameter
#define TIM0_TICK 250 // 250uS
#define VALTH0 ((0-TIM0_TICK) & 0xff ) // mode2 makes 8bit
#define LED_SCAN 2500 // 2.5mS
#define TICK_BASE 8000 // 8mS
#define TIME1SEC 1000000 // 1Sec
#define TICK_CNT ((TIME1SEC/TICK_BASE)-1) //
#define VALTL1 ((0-LED_SCAN) & 0xff) //
#define VALTH1 ((0-LED_SCAN)>> 8) //
#define PRESS_TIME 320000
#define PRESS_CNT PRESS_TIME/TICK_BASE //
//
#ifdef SHE879
 sbit dig_ctrl_6 at P3.B0;        //Declaring control pins of the seven segments
 sbit dig_ctrl_5 at P3.B1;
 sbit dig_ctrl_4 at P3.B2;
 sbit dig_ctrl_3 at P3.B4;
 sbit dig_ctrl_2 at P3.B3;
 sbit dig_ctrl_1 at P3.B5;
  // 7seg data has been configured for SH-E879
 char digi_val[11]={0x08,0x3d,0x44,0x24,0x31,0x22,0x02,0x3c,0x00,0x20,0xff}; // SHE879
#endif
#ifdef AKIT112
 sbit dig_ctrl_6 at P3.B5;        //Declaring control pins of the seven segments
 sbit dig_ctrl_5 at P3.B4;
 sbit dig_ctrl_4 at P3.B3;
 sbit dig_ctrl_3 at P3.B2;
 sbit dig_ctrl_2 at P3.B1;
 sbit dig_ctrl_1 at P3.B0;
  // 7seg data has been configured for AKIT112
 const char digi_val[11]={0x40,0x79,0x24,0x30,0x19,0x12,0x02,0x78,0x00,0x10,0xff}; // SH-E879
 #endif
 sbit key at P3.B7;       // 0 = keypress
  unsigned char blink = 0;
  sbit blinkc at blink.B6;  // blink period is 64times of tick base
 unsigned char timer250us=0;   // 250us timer by Timer0
 unsigned char timer8ms=0;
 unsigned char dig_disp=0;
 unsigned char hour=0;
 unsigned char hour2=0;
 unsigned char hour1=0;
 unsigned char min2=0;
 unsigned char min1=0;
 unsigned char sec2=0;
 unsigned char sec1=0;
 unsigned char tempo =0;
 unsigned char mode=0;     // 0: online, 1: idle power, 2: min set, 3: hour set.
 unsigned char postflg=0;
 unsigned char timer_1_cnt=0;



 const char hour_1[24]={0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3};
 const char hour_2[24]={0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,2,2,2,2};


void delay()        //Function to provide a time delay of approx. 1 second. using Timer 1.
{
// This logic has to confirm timer_0 tick check and if not make idle force.
// In addition thisi logic makes some race condition as timer_0_cnt update between make idle happened
// Timer 0 interrupt issue but on this case timer_0_cnt may cash 2 interrupt as value.
// so following delay fucntion timing may capture happened interupt later.
// This logic should happened all of 25ms interrupt without lost.
// and timer 0 interrupt should maintain 25ms interval as accurate.
    while(timer_1_cnt==0 ){   // wait post flag from interrupt
        // PCON.B0 = 1;       // idle mode request for avoid timer1 (dynamic scan interrupt)
    };
    --timer_1_cnt;    // clear post flag (25ms)
}
/*
 unsigned char keycnt=0;
 unsigned char keyval=0;
*/
unsigned char keyscan()
{
 static unsigned char keycnt=0;
     if(key==0){
          if(keycnt < PRESS_CNT){   // Press detect
               if(++keycnt == PRESS_CNT)   return  2;
          }
          return 0;
     }
     else{
         if(keycnt == 0){  //
              return 0;
         }
         if ( keycnt< PRESS_CNT ) {
              keycnt = 0;
              return 1;
         }
         else{
              keycnt = 0;
              return 0;
         }
     }
}

void timer0() iv IVT_ADDR_ET0 ilevel 0 ics ICS_AUTO {
     ++timer250us;
}

// 4ms pseudo interval timer1 by calibration

void timer1() iv IVT_ADDR_ET1 ics ICS_AUTO {
//Function to display the number using seven segmnet multiplexing. For more details refer seven segment multiplexing.

    TL1=VALTL1;        //Reloading Timer1
    TH1=VALTH1;
    if(timer250us.B4==4){
        if(timer8ms==0){
            timer8ms=1;
            ++timer_1_cnt;
        }
    }else{
          timer8ms =0;
    }
    P3=0xbf;
    P1=0xff;
    dig_disp=++dig_disp%6;
    switch (mode){         // No display
    case 0:
        switch(dig_disp){
        case 0:
        P1=digi_val[sec1];
        dig_ctrl_1 = 0;
        break;

        case 1:
        P1=    digi_val[sec2];
        dig_ctrl_2 = 0;
        break;

        case 2:
        P1=    digi_val[min1];
        dig_ctrl_3 = 0;
        break;

        case 3:
        P1=    digi_val[min2];
        dig_ctrl_4 = 0;
        break;

        case 4:
        P1=    digi_val[hour1];
        dig_ctrl_5 = 0;
        break;

        case 5:
        P1=    digi_val[hour2];
        dig_ctrl_6 = 0;
        break;
        }

    case 2:    // Min set mode blinking
        switch(dig_disp){
        case 0:
             P1=digi_val[sec1];
             dig_ctrl_1 = 0;
             break;

        case 1:
             P1=    digi_val[sec2];
             dig_ctrl_2 = 0;
             break;

        case 2:
             P1=    digi_val[min1];
             if(blinkc == 1) dig_ctrl_3 = 0;
             break;

        case 3:
             P1=    digi_val[min2];
             if(blinkc ==1 ) dig_ctrl_4 = 0;
             break;

        case 4:
             P1=    digi_val[hour1];
             dig_ctrl_5 = 0;
             break;

        case 5:
             P1=    digi_val[hour2];
             dig_ctrl_6 = 0;
             break;
        }
        break;
    case 3:   // hour set mode blinking
        switch(dig_disp){
        case 0:
             P1=digi_val[sec1];
             dig_ctrl_1 = 0;
             break;

        case 1:
             P1=    digi_val[sec2];
             dig_ctrl_2 = 0;
             break;

        case 2:
             P1=    digi_val[min1];
             dig_ctrl_3 = 0;
             break;

        case 3:
             P1=    digi_val[min2];
             dig_ctrl_4 = 0;
             break;

        case 4:
             P1=    digi_val[hour1];
             if (blinkc ==1) dig_ctrl_5 = 0;
             break;

        case 5:
             P1=    digi_val[hour2];
             if (blinkc ==1) dig_ctrl_6 = 0;
             break;

        }
        break;
    }
}




void main()
{
    IE=0;     // All interrupt disable
    PCON=0;
    SCON=0;
    TMOD=0x12;        //Intialize Timer1=mode1 Timer0=2
    TL0=VALTH0;       // Timer0 by mode 2 with 250us interval
    TH0=VALTH0;
    TL1=VALTL1;
    TH1=VALTH1;         // - 4000 (4ms)
    TCON = 0x50;      // TR1 TR0 count enable
    IP = 0x08; //   PT0_Bit = 1;      // high priority Timer 1  (LED Scan)
    IE=0x8A;        // Enable Timer 1 and 0 interrupt
    while(1)        //Start clock
    {
          //Debug code for display position
          //hour=0;hour2=1;hour1=2;min2=3;min1=4;sec2=5;sec1=6;
          //while(1);
          delay();    // wait interrupt 4mS
          switch(mode) {
          case 0: // Online;
          case 1:
              if(++tempo>TICK_CNT){
                   tempo = 0;
                   if (++sec1>9){
                         sec1=0;
                         if (++sec2>5){
                               sec2=0;
                               if (++min1>9){
                                     min1=0;
                                     if (++min2>5){
                                           min2=0;
                                           if (++hour>23) {
                                                 hour = 0;
                                           }
                                           hour2 = hour / 10;
                                           hour1 = hour % 10;
                                     }
                               }
                         }
                    }
              }
              if(mode == 0){
                    switch(keyscan()){
                    case 1: // click
                          mode = 1;
                          break;
                    case 2: // press
                          mode = 2; //min set mode
                          break;
                    }
              }
              else{
                    switch(keyscan()){
                    case 1:  // click
                         mode = 0;   // go online
                         break;
                    case 2:  // press
                         mode = 0; // go online
                         break;
                    }
              }
               break;
          case 2: // min set;
               ++blink;
               switch(keyscan()){
               case  1:  // click
                     if (++min1>9){
                           min1=0;
                           if (++min2>5){
                                 min2=0;
                           }
                     }
                     break;
               case  2: // Press
                     mode = 3;  // move to hourset;
                     break;
               }
               break;
          case 3: // hour set;
              ++blink;
              switch(keyscan()){
              case  1:  // click
                    if (++hour>23) {
                          hour = 0;
                    }
                    hour2 = hour / 10;
                    hour1 = hour % 10;
                    break;
              case  2: // Press
                    mode = 0;  // move to hourset;
                    break;
              }
              break;
          }
    }
}
