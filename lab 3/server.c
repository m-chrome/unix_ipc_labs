#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

int main()
{
    int sid = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sid == -1)
    {
        perror("socket");
        return 1;
    }

    struct sockaddr_un serv_addr;
    bzero((void *)&serv_addr, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, "sock");

    struct sockaddr_un client_addr;
    bzero((void *)&client_addr, sizeof(client_addr));
    client_addr.sun_family = AF_UNIX;
    strcpy(client_addr.sun_path,"cl_sock");

    if (bind(sid, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) != 0)
    {
        perror("bind");
        return 1;
    }

    FILE *filesNum = popen("ls -a | wc -l", "r");
    system("ls -a");
    char files[10]={0};
    fgets(files, 11, filesNum);
    int i=0;
    char buffer[1024]={0};
    while (files[i]!='\n')
    {
        buffer[i]=files[i];
        i++;
    }
    printf("[SERVER] Файлов в директории: %s\n", buffer);

    int seconds = 30;
    while (sendto(sid, buffer, sizeof(buffer),
                  0, (struct sockaddr *)&client_addr,
                  sizeof(struct sockaddr_un)) < 0)
    {
        perror("sendto");
        sleep(1);
        if (seconds-- == 0)
        {
            printf("[SERVER] Время вышло. Закрываю программу.\n");
            return 1;
        }
    }
    printf("[SERVER] Информация отправлена клиенту.\n");

    int len_p = sizeof(struct sockaddr_un);
    if (recvfrom(sid, buffer, sizeof(buffer),
                 0, (struct sockaddr *)&client_addr,
                 &len_p)==-1)
    {
        perror("recvfrom");
        return 1;
    }

    printf("[SERVER] Информация от клиента: %s\n", buffer);
    close(sid);
    unlink("sock");
    pclose(filesNum);
    return 0;
}
