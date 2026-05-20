#include "States.h"     //The .h file with all the information

#define Expert_mode_active 3    //The GPIO that gives a HIGH signal when the expert controller has been connected
state_t state = EXPERT;        //start state when activate the program

//interupt

void next_state(){
    switch(state){
        case EXPERT:
            if(Expert_mode_active == false){     //When the expert mode controller has been disconnected 
                state = PLC;
            }
            break;
        case PLC:
            if(Expert_mode_active == true){
                state = EXPERT;
            }
            if(/*Information of PLC has State == true*/){
                state = PLC_EXPERT;
            }
            break;
        case ERROR:
            if(Expert_mode_active == true){         //when the expert mode is connected, its going out of the Error state
                state = EXPERT;
            }
            break;
        case PLC_EXPERT:
            if(/*Information of PLC has State == false*/){
                state = PLC;
            }
    }
}

void output() {
    //reading the sensors
    //sending data to PLC
    switch (state) {
        case EXPERT:
            //sending read data to Expert controller
            Expert_mode();
            break;
        case PLC:
              PLC_mode();
            break;
        case ERROR:
            Error_mode();
            break;
        case PLC_EXPERT:
            PLC_Expert();
    }
}

void Expert_mode(){
    /* 
        Reads input data
            when position; call Movement();
            other options:
                free movement
                stop
                start
                up / right
                down / left
    */
}

void PLC_mode(){
    /*
        Reads input data          //only when the movement has been finished. we dont want to change coordinate when we havent reached the last coordinate, right? 
            call movement();
    */
}

void Error_mode(){
    /*
        Stop the system
            call movement();        //for stopping
            sending error code to PLC
            waiting for Expert mode
    */
}

void PLC_Expert(){
    /* 
        Reads input data
            when position; call Movement();
            other options:
                free movement
                stop
                start
                up / right
                down / left
    */
}

int Movement(int position){
    /*
        closed loop;
            encoder
            calculate the freq.
            Activate the motor
            bool == true when position has been reached
            return current position
    */
}

void PLC_Sending(uint8_t Data[]){
    /*
        Data sending to PLC  
    */
}

void Expert_Sending(uint8_t Data[]){
    /*
        Data sending to Expert contorller  
    */
}