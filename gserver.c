/*
 * read dictionary file to array of words & get ready to play the hangman!
if you are using fgets() to read the word
          cptr = strchr(line, '\n');
          if (cptr)
                *cptr = '\0';
 However, since we are dealing with words, you can use fscanf(...,"%s", ...) instead!

 * wait for a request to come in (client specifies unique pipename)
 * select a random word using rand()
 * fork() to create a child to handle this client! (dedicated server process for that client)
 * fork() enables games to proceed in parallel. Parent returns to wait for new client requests
 * respond with another unique server-side pipename
 *
 * send a bunch of stars (indicating the word length)
 * for each guess the client sends in, respond with a message
 * and send updated display word.
 *
 * Whenever you send strings through named pipes,
 * terminate with '\n' and also do fflush() for that data to make it to other side without getting stuck
 * in buffers.
 *
 * open public fifo
 * while (fgets()) {
 * }
 *
 *


srand(....);

wait for a client connection
rand() to select a word
fork()
        child process:
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAXWORDS 100000
#define MAXLEN 1000

char *words[MAXWORDS];
int numWords = 0;

int main() {
    char line[MAXLEN];

    FILE *fp = fopen("dictionary.txt", "r");
    if (!fp) {
        puts("dictionary.txt cannot be opened for reading.");
        exit(1);
    }

    while (fscanf(fp, "%s", line) == 1) {
        words[numWords] = strdup(line);
        numWords++;
        if (numWords >= MAXWORDS) break;
    }
    fclose(fp);
    printf("%d words read.\n", numWords);

    srand(getpid() + time(NULL) + getuid());

    char filename[MAXLEN];
    sprintf(filename, "/tmp/%s-%d", getenv("USER"), getpid());
    mkfifo(filename, 0600);
    chmod(filename, 0622);
    printf("Send your requests to %s\n", filename);

    while (1) {
        FILE *fp = fopen(filename, "r");
        if (!fp) {
            printf("FIFO %s cannot be opened for reading.\n", filename);
            exit(2);
        }
                printf("Opened %s to read...\n", filename);

        while (fgets(line, MAXLEN, fp)) {
            char *cptr = strchr(line, '\n');
            if (cptr)
                                 *cptr = '\0';

            if (fork() == 0) {
                FILE *clientfp = fopen(line, "w");

                char serverfifo[MAXLEN];
                sprintf(serverfifo, "/tmp/%s-%d", getenv("USER"), getpid());
                mkfifo(serverfifo, 0600);
                chmod(serverfifo, 0622);

                fprintf(clientfp, "%s\n", serverfifo);
                fflush(clientfp);

                char *word = words[rand() % numWords];
                int wordLen = strlen(word);

                char display[wordLen + 1];
                for (int i = 0; i < wordLen; i++)
                    display[i] = '*';
                display[wordLen] = '\0';

                fprintf(clientfp, "Current word: %s\n", display);
                fprintf(clientfp, "Enter a letter:\n ");

                fflush(clientfp);

                FILE *serverfp = fopen(serverfifo, "r");
                int wrongGuesses = 0;
                int unexposed = wordLen;

                while (unexposed > 0) {
                    if (!fgets(line, MAXLEN, serverfp)) break;

                    char guess = line[0];
                    int found = 0;

                    for (int i = 0; i < wordLen; i++) {
                        if (word[i] == guess) {
                            if (display[i] == guess) {
                                fprintf(clientfp, "%c is already in the word.\n", guess);
                                found = -1;
                                break;
                            } else {
                                display[i] = guess;
                                unexposed--;
                                found = 1;
                            }
                        }
                    }

                    if (found == 0) {
                        wrongGuesses++;
                        fprintf(clientfp, "%c is not in the word.\n", guess);
                    }


                    fprintf(clientfp, "Current word: %s\n", display);
                    if (unexposed > 0) {
                        fprintf(clientfp, "Enter a letter:\n ");
                                    }

                    fflush(clientfp);
                }

                fprintf(clientfp, "The word was %s. You missed %d times.\n", word, wrongGuesses);
                fflush(clientfp);

                fclose(serverfp);
                unlink(serverfifo);
                exit(0);
            }
        }
        fclose(fp);
    }
}
