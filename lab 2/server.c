/* Сервер.
 *
 * Создать набор семафоров и разделяемую область памяти (РОП).
 * Подождать, пока клиент не пришлет информацию. Определить права
 * владельца каждого файла и переслать  эти данные через разделяемую
 * область памяти клиенту.*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

struct sembuf op1 = {0, 0, 0};
struct sembuf op2 = {0, 1, 0};

union semun
{
    int val;
    struct semid_ds *buf;
    ushort *array;
} arg;

int main(void)
{
    int shmid = shmget(1488, 1024, IPC_CREAT | 0660);
    if (shmid == -1)
    {
        perror("[SERVER] Не удалось создать РОП. Программа будет закрыта.\n");
        exit(1);
    }
    printf("[SERVER] РОП успешно создана.\n");

    int semsid = semget(101, 1, IPC_CREAT | 0660);
    if (semsid == -1)
    {
        perror("[SERVER] Не удалось создать набор семафоров. Программа будет закрыта.\n");
        exit(1);
    }
    printf("[SERVER] Набор семафоров успешно создан.\n");

    arg.val = 1;
    semctl(semsid, 0, SETVAL, arg);
    char *addr = (char*)shmat(shmid, 0, 0);

    if (semop(semsid, &op1, 1)==-1)
        perror("semop error\n");

    // Работа с РОП
    printf("[SERVER] Содержимое РОП:\n----\n%s\n----\n", addr);

    // Чтение из РОП имен файлов
    char files[512];
    memset(files, '\0', 512);
    int i=0;
    while (addr[i]!='\0')
    {
        if (addr[i] == '\n')
            files[i] = ' ';
        else
            files[i] = addr[i];
        i++;
    }

    // Составление команды для знания прав доступа файлов
    char command[500];
    memset(command, '\0', 500);
    strcpy(command, "ls -l ");
    strcat(command, files);
    strcat(command, " | awk '{print $1,$3,$9}'\n");

    // Запуск команды и запись её в РОП
    FILE *ls = popen(command, "r");
    char fileStr[256];
    memset(fileStr, '\0', 256);
    printf("[SERVER] Записываю данные о правах доступа.\n");
    memset(addr, '\0', 1024);
    while(fgets(fileStr, 257, ls))
    {
        int i=1;
        char str[256];
        for(i=1; i<256; ++i)
            str[i-1]=fileStr[i];
        strcat(addr, str);
    }
    printf("[SERVER] Данные о правах доступа файлов записаны в РОП.\n");

    if (shmdt(addr) == -1)
        perror("[SERVER] Нельзя отсоединить РОП.\n");
    else
        printf("[SERVER] РОП отсоединена.\n");

    semop(semsid, &op2, 1);
    printf("[SERVER] Конец работы.\n");
    return 0;
}
