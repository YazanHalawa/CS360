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

sem_t* 
thdata::getLock(){
	return &lock;
}

sem_t* 
thdata::getStored(){
	return &stored;
}



