#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAXLEN 1000

int main(int argc, char *argv[]) {
    if (argc != 2) {
        puts("Usage: gclient <server-fifo-name>");
        exit(1);
    }

    char clientfifo[MAXLEN];
    sprintf(clientfifo, "/tmp/%s-%d", getenv("USER"), getpid());
    mkfifo(clientfifo, 0600);
    chmod(clientfifo, 0622);

    FILE *fp = fopen(argv[1], "w");

        fprintf(fp, "%s\n", clientfifo);
    fclose(fp);

    FILE *clientfp = fopen(clientfifo, "r");

    char serverfifo[MAXLEN];
    fscanf(clientfp, "%s", serverfifo);
    char line[MAXLEN];
    fgetc(clientfp);

    FILE *serverfp = fopen(serverfifo, "w");

    while (1) {
        if (!fgets(line, MAXLEN, clientfp)) break;

        if (strstr(line, "Enter a letter:")) {
            printf("%s", line);
            char response[100];
            scanf("%s", response);
            fprintf(serverfp, "%c\n", response[0]);
            fflush(serverfp);
        }
        else {
            printf("%s", line);
            if (strstr(line, "The word was")) break;
        }
    }

    fclose(clientfp);
    fclose(serverfp);
    unlink(clientfifo);
}
