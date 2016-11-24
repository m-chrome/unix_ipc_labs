#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>

#ifndef MSGMAX
#define MSGMAX 2048
#endif

#define SERVER_MSG_MTYPE 36

struct msgbuf
{
    long mtype;
    char text[MSGMAX];
} messageBuf, newMessage;

int main()
{
    // Подключение к очереди сообщений
    int serverMsQ = msgget(1488,0);
    while (serverMsQ == -1)
    {
        printf("[SERVER] Не удалось подключиться к очереди. Попробую ещё раз.\n");
        sleep(5);
        serverMsQ = msgget(1488,0);
    }
    printf("[SERVER] Подключение к очереди сообщений прошло успешно.\n");

    // Прочесть самое старое сообщение и найти файлы,
    // где больше 10 строк
    if (msgrcv(serverMsQ, &messageBuf, MSGMAX, 0, IPC_NOWAIT)==-1)
        perror("[SERVER] Не удалось прочесть старое сообщение из очереди.\n");
    char *command = malloc(sizeof(char)*500);
    memset(command, '\0', 500);
    strncpy(command, "wc -l ", 6);
    int i = 0;
    strcat(command, messageBuf.text);
    memset(newMessage.text, '\0', MSGMAX);
    strcpy(newMessage.text, "[SERVER] Файлы, строк в которых более 10:");
    FILE *wc = popen(command, "r");
    char *filename = malloc(256);
    memset(filename, '\0', 256);
    int number;
    while (fscanf(wc, "%d %s\n", &number, filename) == 2)
    {
        if (strcmp(filename, "total") != 0)
        {
            if (number > 10)
            {
                strcat(newMessage.text, " ");
                strcat(newMessage.text, filename);
            }
        }
    }
    strcat(newMessage.text, "\n");
    printf("%s", newMessage.text);
    pclose(wc);

    // Передача сообщения в очередь
    newMessage.mtype = SERVER_MSG_MTYPE;
    if (msgsnd(serverMsQ, &newMessage, MSGMAX, IPC_NOWAIT) == -1)
    {
        perror("[SERVER] Не удалось отправить сообщение.\n");
        exit(1);
    }
    else
        printf("[SERVER] Сообщение отправлено.\n");

    // Время отправки последнего сообщения в очередь
    struct msqid_ds queue;
    msgctl(serverMsQ, IPC_STAT, &queue);
    time_t lastMessage = queue.msg_stime;
    printf("[SERVER] Время передачи последнего сообщения в очередь: %s", ctime(&lastMessage));

    // Удаление очереди сообщений
    if (msgctl(serverMsQ, IPC_RMID, 0)==-1)
        perror("[SERVER] Не удалось удалить очередь сообщений.\n");
    else
        printf("[SERVER] Очередь сообщений удалена.\n");
    printf("[SERVER] Конец работы.\n");
    free(command);
    free(filename);
    return 0;
}
