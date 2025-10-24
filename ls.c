#include <linux/limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

char *get_mode_str(__mode_t mode) {
    switch (mode) {
    case S_IFBLK:
        return "BLOCK DEVICE";
    case S_IFSOCK:
        return "SOCKET";
    case S_IFLNK:
        return "SYMBOLIC LINK";
    case S_IFREG:
        return "REGULAR FILE";
    case S_IFDIR:
        return "DIRECTORY";
    case S_IFCHR:
        return "CHARACTER DEVICE";
    case S_IFIFO:
        return "FIFO";
    }

    return "UNRECOGNIZED FILE TYPE";
}

const char *COLOR_RESET = "\x1b[0m";
const char *COLOR_CODES[] = {
    "\x1b[0m",  // c_RESET
    "\x1b[34m", // c_DIRECTORY (blue)
    "\x1b[37m", // c_FILE (white)
};

enum COLORS {
    c_RESET = 0,
    c_DIRECTORY = 1,
    c_FILE = 2,
    c_SYMLINK = 3,
};

char *format_color(char *string, enum COLORS color) {
    const char *code = COLOR_CODES[color];
    int len = strlen(code) + strlen(string) + strlen(COLOR_RESET) + 1;
    char *dest = malloc(len);

    if (!dest)
        return NULL;

    dest[0] = '\0';
    strcat(dest, code);
    strcat(dest, string);
    strcat(dest, COLOR_RESET);

    return dest;
};

int main(int argc, char **argv) {
    char *arg;
    FILE *file;

    for (int i = 1; i < argc; i++) {
        arg = argv[i];

        struct stat file_stat;
        lstat(arg, &file_stat);

        if ((file_stat.st_mode & S_IFMT) == S_IFDIR) {
            DIR *dirp = opendir(arg);

            if (!dirp) {
                printf("Error reading dir: %s\n", arg);
                continue;
            }

            struct dirent *dir_info;

            char **directories = NULL;
            size_t directories_len = 0;

            char **files = NULL;
            size_t files_len = 0;

            printf(".   ..  ");
            while ((dir_info = readdir(dirp)) != NULL) {
                if (strcmp(dir_info->d_name, ".") == 0 ||
                    strcmp(dir_info->d_name, "..") == 0) {
                    continue;
                }

                char full_path[PATH_MAX];
                snprintf(full_path, sizeof(full_path), "%s/%s", arg,
                         dir_info->d_name);

                struct stat subfile_stat;
                if (lstat(full_path, &subfile_stat) == -1) {
                    perror("lstat");
                    exit(EXIT_FAILURE);
                };

                switch (subfile_stat.st_mode & S_IFMT) {
                case S_IFDIR:
                    directories =
                        realloc(directories,
                                sizeof(const char *) * (directories_len + 1));

                    directories[directories_len] =
                        malloc(strlen(dir_info->d_name) + 1);

                    strcpy(directories[directories_len], dir_info->d_name);
                    directories_len += 1;

                    break;

                case S_IFREG:
                    files =
                        realloc(files, sizeof(const char *) * (files_len + 1));

                    files[files_len] = malloc(strlen(dir_info->d_name) + 1);

                    strcpy(files[files_len], dir_info->d_name);
                    files_len += 1;
                    break;

                default:
                    printf("UNKNOWN FILE TYPE: %u, %s\n", subfile_stat.st_mode,
                           dir_info->d_name);
                }
            }

            for (int i = 0; i < directories_len; i++) {
                char *formatted = format_color(directories[i], c_DIRECTORY);
                printf("%s  ", formatted);
                free(formatted);
            }

            for (int i = 0; i < files_len; i++) {
                char *formatted = format_color(files[i], c_FILE);
                printf("%s  ", formatted);
                free(formatted);
            }

        } else if ((file_stat.st_mode & S_IFMT) == S_IFREG) {
            printf("NAME    TYPE    UID    SIZE    GID\n");
            printf("----------------------------------\n");
            printf("%s     %u       %u    %ld     %u\n", arg, file_stat.st_mode, file_stat.st_uid, file_stat.st_size, file_stat.st_gid); }

        printf("\n");
    }

    return 0;
}
