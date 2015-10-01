#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>

class thdata {
public:
	thdata();
	~thdata();

	int getNumber();
	void setNumber(int newNum);
	sem_t* getLock();
	sem_t* getStored();
private:
	int number;
	sem_t lock;
	sem_t stored;
};