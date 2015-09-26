#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

class thdata {
public:
	thdata();
	~thdata();

	int getNumber();
	void setNumber(int newNum);
	pthread_mutex_t* getMutexAddr();
	pthread_cond_t* getCondAddr();
private:
	int number;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
};