#include <stdio.h>
#include <math.h>
#include <M451Series.h>
#include <Display.h>
#include <Font.h>
#include <Atomizer.h>
#include <Button.h>
#include <TimerUtils.h>
#include <Battery.h>

#include "Bitmap_eVicSDK.h"

#define FIRE 0
#define RIGHT 1
#define LEFT 2

volatile uint32_t buttonSpec[3][3] = {{0},{0},{0}}; // [timesPressed, justPressed, timerWhenUpdate]
volatile uint32_t timer = 0; // TODO: Handle overflow. Will Currently last 4.97 days.
volatile uint32_t newWatts = 0; //Is this safe?
volatile uint8_t newWatts_Open = 1; // If this is true, then newWatts can be modified.
void timerCallback(){
    timer++;
}


uint16_t wattsToVolts(uint32_t watts, uint16_t res) {
    // Units: mV, mW, mOhm
    // V = sqrt(P * R)
    // Round to nearest multiple of 10
    uint16_t volts = (sqrt(watts * res) + 5) / 10;
    return volts * 10;
}

uint16_t correctVoltage(uint16_t currentVolt, uint32_t currentWatt, uint16_t res) {
    // Resistance fluctuates, this corrects voltage to the correct setting.
    uint16_t newVolts = wattsToVolts(currentWatt, res);
    
    if (newVolts != currentVolt) {
        if (Atomizer_IsOn()) {
            // Update output voltage to correct res variations:
            // If the new voltage is lower, we only correct it in
            // 10mV steps, otherwise a flake res reading might
            // make the voltage plummet to zero and stop.
            // If the new voltage is higher, we push it up by 100mV
            // to make it hit harder on TC coils, but still keep it
            // under control.
            if (newVolts < currentVolt) { newVolts = currentVolt - (currentVolt >= 10 ? 10 : 0); }
            else { newVolts = currentVolt + 100; }
        }
    }
    return newVolts;
}

// TODO: Join each callback into one. (?) Is this needed?
void buttonFireCallback(uint8_t state){ // Only gets called when something happens.
    if (state & BUTTON_MASK_FIRE){
        buttonSpec[FIRE][0]++;
        buttonSpec[FIRE][1] = 1;
        buttonSpec[FIRE][2] = timer;
    } else {
        buttonSpec[FIRE][1] = 0;
        buttonSpec[FIRE][2] = timer;
    }
}

void buttonRightCallback(uint8_t state){ // Only gets called when something happens.
    if (state & BUTTON_MASK_RIGHT){
        buttonSpec[RIGHT][0]++;
        buttonSpec[RIGHT][1] = 1;
        buttonSpec[RIGHT][2] = timer;
        if (newWatts_Open && (state == BUTTON_MASK_RIGHT)){
            newWatts += 100;
        }
    } else {
        buttonSpec[RIGHT][1] = 0;
        buttonSpec[RIGHT][2] = timer;
    }
}


void buttonLeftCallback(uint8_t state){ // Only gets called when something happens.
    if (state & BUTTON_MASK_LEFT){
        buttonSpec[LEFT][0]++;
        buttonSpec[LEFT][1] = 1;
        buttonSpec[LEFT][2] = timer;
        if (newWatts_Open && (state == BUTTON_MASK_LEFT)){
            newWatts -= 100;
        }
    } else {
        buttonSpec[LEFT][1] = 0;
        buttonSpec[LEFT][2] = timer;
    }
}


int main(){
    // TODO:    Add Stealth mode.
    //          Add TCR (Formulas?)
    //          Add bypass mode. (?)
    //          Implement a better font. With support for Omega etc.
    //          Fixed fields for values and strings.
    //          Add a small snake game. (?) What would the memory impact be? Trigger? left left right right fire 2x.
    //          See how puff and time is implemented and use these mem locations for something cool.
    //          Add volt mode. (?)
    char buf[200];
    const char *atomState, *batteryState;
    uint8_t shouldFire;
    uint16_t volts, displayVolts; /*, battVolts*/ // Unit mV
    uint32_t watts; // Unit mW
    uint8_t btnState;/*, battPerc, boardTemp*/
    Atomizer_Info_t atomInfo;
    
    Atomizer_ReadInfo(&atomInfo);
    
    watts = 20000; // Initial wattage
    volts = wattsToVolts(watts, atomInfo.resistance);
    Atomizer_SetOutputVoltage(volts);
    
    Timer_CreateTimeout(10, 1, timerCallback, 0);

    Button_CreateCallback(buttonFireCallback, BUTTON_MASK_FIRE);
    Button_CreateCallback(buttonRightCallback, BUTTON_MASK_RIGHT);
    Button_CreateCallback(buttonLeftCallback, BUTTON_MASK_LEFT);
    //Show logo!
    Display_PutPixels(0, 32, Bitmap_evicSdk, Bitmap_evicSdk_width, Bitmap_evicSdk_height);
    Display_Update();
    Timer_DelayMs(500);
    // Main loop!
    newWatts = watts;
    while(1){
        Atomizer_ReadInfo(&atomInfo);
        btnState = Button_GetState(); // Unsure if needed.
        
        if ((timer - buttonSpec[FIRE][2] > 40))
            shouldFire = 1;
        else
            shouldFire = 0;
        
        if ((btnState >= BUTTON_MASK_LEFT + BUTTON_MASK_RIGHT) || (btnState == BUTTON_MASK_LEFT + BUTTON_MASK_FIRE) || (btnState == BUTTON_MASK_RIGHT + BUTTON_MASK_FIRE)) {
            newWatts_Open = 0;
        }else{
            newWatts_Open = 1;
        }

        if(!Atomizer_IsOn() && (btnState == BUTTON_MASK_FIRE) && // Only fire if fire is pressed alone.
            (atomInfo.resistance != 0) && (Atomizer_GetError() == OK) && shouldFire){
                Atomizer_Control(1);
        } else if (Atomizer_IsOn() && !(btnState & BUTTON_MASK_FIRE)){
                Atomizer_Control(0);
        }
       

        for(int i=0; i<=2; i++){
            uint32_t mod = 1; 
            if (i == LEFT)
                mod = -1;
            if (timer - buttonSpec[i][2] > 60){
                buttonSpec[i][0] = 0;
            }
            if (buttonSpec[i][1] == 1 && (i != 0) && newWatts_Open){
                uint32_t elapsed = timer - buttonSpec[i][2];

                if (elapsed > 60 && elapsed < 180) {
                    newWatts += mod * 25;
                } else if (elapsed > 180){
                    newWatts += mod * 350; 
                }
            }
        }

        if (newWatts > 75000){
            newWatts = 75000;
        } else if (newWatts < 1000){
            newWatts = 1000;
        }
        watts = newWatts;


        
        Atomizer_ReadInfo(&atomInfo);
        
        volts = correctVoltage(volts, watts, atomInfo.resistance);
        Atomizer_SetOutputVoltage(volts);
        switch(Atomizer_GetError()) {
            case SHORT:
                atomState = "SHORT";
                break;
            case OPEN:
                atomState = "NO ATOM";
                break;
            default:
                 atomState = Atomizer_IsOn() ? "FIRING" : "";
                break;
        }
        if (Battery_IsPresent()){
            if (Battery_IsCharging()){
                batteryState = "Charging";
            } else {
                batteryState = "";
            }
        } else {
            batteryState = "USB";
        }
        displayVolts = Atomizer_IsOn() ? atomInfo.voltage : volts;
        
        siprintf(buf, "P:%3lu.%luW\nV:%3d.%02d\n%1d.%02d Ohm\nBV:%uV\nI:%2d.%02dA\n%s\n%s\n%d  %d",
        watts / 1000, watts % 1000 / 100,
        displayVolts / 1000, displayVolts % 1000 / 10,
        atomInfo.resistance / 1000, atomInfo.resistance % 1000 / 10, Battery_GetVoltage(),
        atomInfo.current / 1000, atomInfo.current % 1000 / 10,
        atomState, batteryState, buttonSpec[LEFT][0], buttonSpec[RIGHT][0]);
        Display_Clear();
        Display_PutText(0, 0, buf, FONT_DEJAVU_8PT);
        Display_Update();
    }
}
