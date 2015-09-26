#include "thdata.h"

thdata::thdata(){
	number = 0;
}

thdata::~thdata(){}

int
thdata::getNumber(){
	int returnValue = 0;
	returnValue = number;
	return returnValue;
}

void
thdata::setNumber(int newNum){
	number = newNum;
}

