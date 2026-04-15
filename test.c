#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>

#define BASE_DIR "urban_districts"

typedef enum { ROAD, LIGHTING, FLOODING, OTHER } IssueType;

typedef struct {
    int id;
    IssueType type;
    char description[256];
    time_t timestamp;
    int severity; 
} InfrastructureReport;

void ensure_district_exists(const char *district) {
    struct stat st = {0};
    if (stat(BASE_DIR, &st) == -1) {
        mkdir(BASE_DIR, 0755);
    }

    char path[512];
    snprintf(path, sizeof(path), "%s/%s", BASE_DIR, district);
    if (stat(path, &st) == -1) {
        mkdir(path, 0770);
    }
}

void add_report(const char *role, const char *district) {
    if (strcmp(role, "inspector") != 0 && strcmp(role, "manager") != 0) {
        return;
    }

    ensure_district_exists(district);
    InfrastructureReport report;
    report.id = rand() % 10000;
    report.timestamp = time(NULL);
    
    printf("Descriere: ");
    scanf(" %[^\n]", report.description);
    printf("Severitate (1-5): ");
    scanf("%d", &report.severity);
    report.type = ROAD;

    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/%s/report_%d.bin", BASE_DIR, district, report.id);

    int fd = open(filepath, O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (fd < 0) return;

    write(fd, &report, sizeof(InfrastructureReport));
    close(fd);

    if (strcmp(role, "manager") == 0) {
        chmod(filepath, S_IRUSR | S_IWUSR | S_IRGRP);
    } else {
        chmod(filepath, S_IRUSR | S_IWUSR);
    }

    printf("Raport %d creat.\n", report.id);
}

void remove_report(const char *role, const char *district, int id) {
    if (strcmp(role, "manager") != 0) {
        fprintf(stderr, "Eroare: Acces refuzat.\n");
        return;
    }

    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/%s/report_%d.bin", BASE_DIR, district, id);

    if (unlink(filepath) == 0) {
        printf("Raport %d sters.\n", id);
    }
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    if (argc < 5) return 1;

    const char *role = argv[2];
    if (strcmp(argv[3], "--add") == 0) {
        add_report(role, argv[4]);
    } else if (strcmp(argv[3], "--remove_report") == 0) {
        if (argc < 6) return 1;
        remove_report(role, argv[4], atoi(argv[5]));
    }
    return 0;
}