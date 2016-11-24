#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>

#ifndef MSGMAX
#define MSGMAX 2048
#endif

#define CLIENT_MSG_MTYPE 35
#define SERVER_MSG_MTYPE 36

struct msgbuf
{
    long mtype;
    char text[MSGMAX];
} messageBuf;

int main()
{
    int clientMsQ = msgget(1488, IPC_CREAT|0660);
    if (clientMsQ == -1)
    {
        perror("[CLIENT] Невозможно создать очередь. Программа будет закрыта.");
        exit(1);
    }
    printf("[CLIENT] Очередь сообщений clientMsQ успешно создана.\n");

    FILE* txtInCWD;
    txtInCWD = popen("file * | grep text", "r");
    if (txtInCWD == NULL)
    {
        perror("[CLIENT] В директории отсутствуют тексторые файлы.\nПрограмма будет закрыта.");
        exit(1);
    }

    memset(messageBuf.text, '\0', MSGMAX);
    char fileString[100];
    while(fgets(fileString, 101, txtInCWD))
    {
        int i = 0;
        char fileName[256];
        memset(fileName, '\0', 256);
        while (fileString[i]!=':')
        {
            fileName[i]=fileString[i];
            i++;
        }
        strcat(messageBuf.text, fileName);
        strcat(messageBuf.text, " ");
    }
    strcat(messageBuf.text, "\n");

    if (strcmp(messageBuf.text, "\0")==0)
    {
        printf("[CLIENT] Текстовые файлы не найдены в директории. Программа будет закрыта.\n");
        exit(1);
    }
    else
        printf("[CLIENT] Найдены следующие текстовые файлы: %s", messageBuf.text);

    if (pclose(txtInCWD) == -1)
    {
        perror("Невозможно закрыть команду.\nПрограмма будет закрыта.");
        exit(1);
    }
    messageBuf.mtype = CLIENT_MSG_MTYPE;

    if (msgsnd(clientMsQ, &messageBuf, MSGMAX, IPC_NOWAIT) == -1)
    {
        perror("[CLIENT] Не удалось отправить сообщение.\n");
        exit(1);
    }
    else
        printf("[CLIENT] Сообщение отправлено.\n");

    printf("[CLIENT] Ожидание ответа с сервера.\n");
    memset(messageBuf.text, '\0', MSGMAX);
    while (msgrcv(clientMsQ, &messageBuf, MSGMAX, SERVER_MSG_MTYPE, 0)==-1)
        perror("[CLIENT] Не удалось прочесть сообщение с сервера.\n");
    printf("%s", messageBuf.text);
    printf("[CLIENT] Конец работы.\n");
}
