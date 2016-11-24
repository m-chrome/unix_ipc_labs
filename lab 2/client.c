/* Клиент.
 *
 * Записать в созданную сервером разделяемую область
 * памяти информацию (имена) обо всех файлах текущего
 * каталога. После того, как будет получена информация
 * о  владельцах файлов, вывести время, когда процесс
 * последний раз подключался к разделяемой области
 * памяти. Удалить РОП и набор семафоров.*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

struct sembuf op = {0, -1, 0};

int main()
{
    int semsid = semget(101, 1, 0);
    int tries = 5;
    while (semsid == -1)
    {
        tries--;
        printf("[CLIENT] Невозможно открыть набор семафоров. Try again.\n");
        if (tries == 0)
        {
            perror("[CLIENT] Было 5 попыток подключиться. Программа будет закрыта.\n");
            exit(1);
        }
        sleep(5);
        semsid = semget(101, 1, 0);
    }
    printf("[CLIENT] Набор семафоров успешно открыт.\n");

    int shmid = shmget(1488, 1024, 0);
    while (shmid == -1)
    {
        perror("[CLIENT] Нельзя открыть РОП. Try again.");
        shmid = shmget(1488, 1024, 0);
    }
    printf("[CLIENT] РОП успешно открыта.\n");
    char *addr = (char*)shmat(shmid, 0, 0);

    FILE *ls = popen("ls", "r");
    char *fileName = malloc(20);
    memset(fileName, '\0', 20);
    while (fscanf(ls, "%s", fileName)==1)
    {
        strcat(addr, fileName);
        strcat(addr, "\n");
    }

    semop(semsid, &op, 1);
    printf("[CLIENT] Данные о файлах записаны.\n");

    semop(semsid, &op, 1);
    printf("[CLIENT] Содержимое РОП:\n\n%s\n", addr);
    struct shmid_ds dump;
    shmctl(shmid, IPC_STAT, &dump);
    time_t last = dump.shm_atime;
    printf("[CLIENT] Время последнего присоединения процесса к РОП: %s", ctime(&last));

    if (shmdt(addr) == -1)
        perror("[CLIENT] Нельзя отсоединить РОП.\n");
    else
        printf("[CLIENT] РОП отсоединена.\n");

    if (semctl(semsid, 0, IPC_RMID, 0) == -1)
    {
        perror("[CLIENT] Невозможно удалить набор семафоров.\n");
        exit(1);
    }
    if (shmctl(shmid, IPC_RMID, 0) == -1)
    {
        perror("[CLIENT] Невозможно удалить РОП.\n");
        exit(1);
    }
    printf("[CLIENT] РОП и набор семафоров удалены.\n");
    printf("[CLIENT] Конец работы.\n");
    free(fileName);
    return 0;
}
