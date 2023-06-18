#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

int main(int argc, char *argv[]) {
    openlog("writer", LOG_PID | LOG_CONS, LOG_USER);

    if (argc < 3) {
        syslog(LOG_ERR, "Insufficient arguments. Usage: writer <string> <file>");
        closelog();
        exit(EXIT_FAILURE);
    }

    char *string = argv[1];
    char *file = argv[2];

    FILE *fp = fopen(file, "w");
    if (fp == NULL) {
        syslog(LOG_ERR, "Error opening file: %s", file);
        closelog();
        exit(EXIT_FAILURE);
    }

    fprintf(fp, "%s\n", string);
    fclose(fp);

    syslog(LOG_DEBUG, "Writing '%s' to '%s'", string, file);

    closelog();
    return EXIT_SUCCESS;
}
