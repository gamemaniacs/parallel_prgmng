#include <stdlib.h>
#include <pthread.h>
#include <malloc.h>
#include "api.h"


DEFINE_PER_THREAD(unsigned long, counter);
unsigned long global_count;
int stopflag;
int startflag;

void inc_count(void)
{
	ACCESS_ONCE(__get_thread_var(counter))++;
}

unsigned long read_count(void)
{
	return ACCESS_ONCE(global_count);
}

void *eventual(void *arg)
{
        int t;
        int sum;

        while(startflag == 0)
        {
                continue;
        }


        while(stopflag < 10)
        {
                sum = 0;
                printf("Adding!!\n");
                for_each_thread(t)
                        sum += ACCESS_ONCE(per_thread(counter, t));

                ACCESS_ONCE(global_count) = sum;
                poll(NULL, 0, 10);

                if(stopflag)
                {
                        smp_mb();
                        stopflag++;
                }
        }
        return NULL;
}


void count_init(thread_id_t **tid)
{
        void *vp;


        **tid = create_thread(eventual, NULL);

        ACCESS_ONCE(startflag) = 1;

//        vp = wait_thread(tid);

}

void count_cleanup(void)
{
        stopflag = 1;
        while(stopflag < 10)
        {
                poll(NULL, 0, 1);
        }
        smp_mb();
        printf("%s::end\n",__func__);
}


void *counter(void *arg)
{
	int i;
	
	
	while(startflag == 0) continue;

	
	for(i = 0; i < 100; i++)
	{
		printf("Incrementing!!\n");
		inc_count();
		poll(NULL, 0, 1);
	}
	return NULL;
}

void *reader(void *arg)
{
	unsigned long val;
	int i;

	while(startflag == 0) continue;


	for(i = 0; i < 100; i++)
	{
		val = read_count();
		printf("val read = %ld\n", val);
		poll(NULL, 0, 10);	
	}

	count_cleanup();
	return NULL;
}



int main(void)
{
	thread_id_t 	tid1;
	thread_id_t 	tid2;
	thread_id_t 	tid3;
	thread_id_t	*tid_eventual;
	int 		frst = 1, scnd = 2;
	void 		*vp;
	
	tid_eventual = malloc(sizeof(thread_id_t));

	tid1 = create_thread(counter, &frst);
	//tid2 = create_thread(counter, &scnd);
	tid3 = create_thread(reader, NULL);

	count_init(&tid_eventual);
	//ACCESS_ONCE(startflag) = 1;
	//count_cleanup();

	vp = wait_thread(tid1);
	//vp = wait_thread(tid2);
	vp = wait_thread(tid3);
	vp = wait_thread(*tid_eventual);
	free(tid_eventual);

	return 0;
}
