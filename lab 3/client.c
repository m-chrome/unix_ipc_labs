#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int main()
{
    int sid = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sid == -1)
    {
        perror("socket");
        return 1;
    }

    struct sockaddr_un client_addr;
    bzero((void *) &client_addr, sizeof(client_addr));
    client_addr.sun_family = AF_UNIX;
    strcpy(client_addr.sun_path, "cl_sock");

    struct sockaddr_un server_addr;
    bzero((void *) &server_addr, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, "sock");

    if (bind(sid, (struct sockaddr *)&client_addr, sizeof(client_addr)) != 0)
    {
        perror("bind");
        return 1;
    }

    int len_p = sizeof(struct sockaddr_un);
    char buffer[10]={0};
    recvfrom(sid, buffer, sizeof(buffer), 0, (struct sockaddr *)&server_addr, &len_p);
    printf("[CLIENT] Информация от сервера: %s\n", buffer);
    int files = atoi(buffer);

    FILE *dotFiles = popen("file .* | wc -l", "r");
    system("ls -a");
    char dotfiles[10]={0};
    fgets(dotfiles, 11, dotFiles);
    int i=0;
    while (dotfiles[i]!='\n')
    {
        buffer[i]=dotfiles[i];
        i++;
    }
    int dfiles = atoi(buffer);
    files -= dfiles;
    printf("[CLIENT] Файлов теперь: %d\n", files);
    sprintf(buffer, "%d", files);

    while(sendto(sid, buffer, sizeof(buffer), 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_un)) < 0)
    {
        perror("sendto");
        sleep(1);
    }

    printf("[CLIENT] Информация послана на сервер.\n");
    close(sid);
    unlink("cl_sock");
    printf("[CLIENT] Конец работы.\n");
    return 0;
}
