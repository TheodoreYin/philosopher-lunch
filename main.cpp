#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdlib.h>
sem_t *chopsticks[5];

int init_all()
// sem_init() is deprecated in MacOS, using sem_open()
{
    for (int i = 0; i < 5; i++)
    {
        char name[10];
        sprintf(name, "/%d", i);
        chopsticks[i] = sem_open(name, O_CREAT, 0644, 1);
        if (chopsticks[i] == SEM_FAILED)
            return false;
    }
    return true;
}

int close_all()
{
    //semaphore must be closed and unlinked when it's used
    // ---- which may cause unblocked semaphore next time

    //sem_destroy() and sem_getvalue() are deprecated in MacOS, using sem_close()
    for (int i = 0; i < 5; i++)
    {
        char name[10];
        sprintf(name, "/%d", i);
        if (sem_close(chopsticks[i]) == -1 && sem_unlink(name) == -1)
            return false;
    }
    return true;
}


void eat(int id)
{
    printf("philosopher %d is eating\n", id);
    sleep(1);
}

void think(int id)
{
    printf("Philosopher %d is thinking\n", id);
   sleep(1);
}

void process(int id)
{
    if (id % 2)
    {
        sem_wait(chopsticks[id]);
        sem_wait(chopsticks[(id + 1) % 5]);
        eat(id);
        sem_post(chopsticks[(id + 1) % 5]);
        sem_post(chopsticks[id]);
        think(id);
    }
    else
    {
        sem_wait(chopsticks[(id + 1) % 5]);
        sem_wait(chopsticks[id]);
        eat(id);
        sem_post(chopsticks[id]);
        sem_post(chopsticks[(id + 1) % 5]);
        think(id);
    }
}

int main()
{
    if (!init_all())
        exit(-1);
    pid_t pid, pid_list[5];
    int num;
    for (int i = 0; i < 5; i++)
    {
        pid = fork();
        if (!pid)
        {
            num = i;
            break;
        }
        else
            pid_list[i] = pid;
    }
    if (!pid)
    {
        printf("Philosopher %d started!\n", num);
        for (int i = 0; i <10; i++)
            process(num);
        printf("Philosopher %d is done.\n", num);
        exit(1);
    }
    else
    {
        for (int i = 0; i < 5; i++)
            waitpid(pid_list[i], NULL, 0);
    }
    if (!close_all())
        exit(-2);
    printf("All done\n");
    return 0;
};