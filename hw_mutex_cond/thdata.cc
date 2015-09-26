#include "thdata.h"

thdata::thdata(){
	number = 0;
}

thdata::~thdata(){}

int
thdata::getNumber(){
	return number;
}

void
thdata::setNumber(int newNum){
	number = newNum;
}

pthread_mutex_t* 
thdata::getMutexAddr(){
	return &mutex;
}


