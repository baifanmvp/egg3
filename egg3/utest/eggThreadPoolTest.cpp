#include "../src/uti/eggThreadPool.h"
#include <time.h>
#include <sys/time.h>
#include "eggThreadPoolTest.h"

eggThreadPoolTest::eggThreadPoolTest(void)
{
    
}

eggThreadPoolTest::~eggThreadPoolTest(void)
{
    
}

void eggThreadPoolTest::setUp(void)
{
    
    return ;
}

void eggThreadPoolTest::tearDown(void)
{
    return ;
    
}

char *s1[20] = { "STRING 0",
	"STRING 1",
	"STRING 2",
	"STRING 3",
	"STRING 4",
	"STRING 5",
	"STRING 6",
	"STRING 7",
	"STRING 8",
	"STRING 9",
	"STRING 10",
	"STRING 11",
	"STRING 12",
	"STRING 13",
	"STRING 14",
	"STRING 15",
	"STRING 16",
	"STRING 17",
	"STRING 18",
	"STRING 19"
};


void r1(void *printstring)
{
	int i, x;

	printf("%s START\n", (char *) printstring);

	for (i = 0; i < 1000000; i++) {
		x = x + i;
	}

	printf("%s DONE\n", (char *) printstring);
}

void eggThreadPoolTest::eggThreadPool11_add_work(void)
{
    tpool_t  test_pool;
	eggThreadPool_init(&test_pool, 10, 20, 0);

    int i;
	sleep(0);
    //char s[]="malei";
    //eggThreadPool_add_work(test_pool, r1, (void*)s1[i]);
	for (i = 0; i < 20; i++) {
		printf("tpool_add_work returned %d\n",\
                eggThreadPool_add_work(test_pool, r1, s1[i]));
    }
	printf("main: all work queued\n");

	eggThreadPool_destroy(test_pool, 1);
    fprintf(stdout,"hello world....\n");
}


