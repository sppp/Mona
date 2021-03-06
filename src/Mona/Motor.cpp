#include "Mona.h"



// Motor constructor.
Motor::Motor() {
	type = MOTOR;
	response = -1;
}

void Motor::Init(RESPONSE response, Mona* mona) {
	Neuron::Init(mona);
	type           = MOTOR;
	this->response = response;
}


// Motor destructor.
Motor::~Motor() {
	Clear();
}


// Is given motor a duplicate of this?
bool Motor::IsDuplicate(Motor& motor) {
	if (response == motor.response)
		return true;
	else
		return false;
}


void Motor::Serialize(Stream& fp) {
	Neuron::Serialize(fp);
	fp % response;
}

/*
    // Print motor.
    #ifdef MONA_TRACKING
    void Motor::Print(FILE *out)
    {
    Print((TRACKING_FLAGS)0, out);
    }


    void Motor::Print(TRACKING_FLAGS tracking, FILE *out)
    #else
    void Motor::Print(FILE *out)
    #endif
    {
    fprintf(out, "<motor><id>%llu</id><response>%d</response>",
           id, response);
    fprintf(out, "<goals>");
    goals.Print(out);
    fprintf(out, "</goals>");
    #ifdef MONA_TRACKING
    if ((tracking & TRACK_FIRE) && tracker.fire)
    {
      fprintf(out, "<fire>true</fire>");
    }
    if ((tracking & TRACK_ENABLE) && tracker.enable)
    {
      fprintf(out, "<enable>true</enable>");
    }
    if ((tracking & TRACK_DRIVE) && tracker.drive)
    {
      fprintf(out, "<motive>%f</motive>", tracker.motive);
    }
    #endif
    fprintf(out, "</motor>");
    }
*/