#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>


#define BUFFER_SIZE 4096

int main(int argc, char **argv) {
    int src_fd = -1, dst_fd = -1;
    ssize_t bytes_read, bytes_written, total_written;
    char buffer[BUFFER_SIZE];
    struct stat src_stat, dst_stat;

    if (argc != 3) {
        printf("Usage: %s <source> <destination>\n", argv[0]);
        return 1;
    }

    // 檢查來源檔是否存在
    if (access(argv[1], F_OK) != 0) {
        fprintf(stderr, "Error: source file '%s' does not exist.\n", argv[1]);
        return 1;
    }

    // 取得來源檔資訊
    if (stat(argv[1], &src_stat) != 0) {
        perror("Error: stat on source file failed");
        return 1;
    }

    // 確認來源是一般檔案
    if (!S_ISREG(src_stat.st_mode)) {
        fprintf(stderr, "Error: source file '%s' is not a regular file.\n", argv[1]);
        return 1;
    }

    // 若目的檔已存在，檢查是不是同一個檔案
    if (stat(argv[2], &dst_stat) == 0) {
        if (src_stat.st_dev == dst_stat.st_dev &&
            src_stat.st_ino == dst_stat.st_ino) {
            fprintf(stderr, "Error: source and destination are the same file.\n");
            return 1;
        }
    }

    // 開啟來源檔（唯讀）
    src_fd = open(argv[1], O_RDONLY);
    if (src_fd < 0) {
        perror("Error: cannot open source file");
        return 1;
    }

    // 開啟/建立目的檔（寫入、清空、若不存在則建立）
    dst_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dst_fd < 0) {
        perror("Error: cannot open/create destination file");
        close(src_fd);
        return 1;
    }

    // 逐塊複製
    while ((bytes_read = read(src_fd, buffer, BUFFER_SIZE)) > 0) {
        total_written = 0;
        while (total_written < bytes_read) {
            bytes_written = write(dst_fd,
                                  buffer + total_written,
                                  bytes_read - total_written);
            if (bytes_written < 0) {
                perror("Error: write failed");
                close(src_fd);
                close(dst_fd);
                return 1;
            }
            total_written += bytes_written;
        }
    }

    if (bytes_read < 0) {
        perror("Error: read failed");
        close(src_fd);
        close(dst_fd);
        return 1;
    }

    if (close(src_fd) < 0) {
        perror("Warning: failed to close source file");
    }

    if (close(dst_fd) < 0) {
        perror("Warning: failed to close destination file");
    }

    printf("File copied successfully from '%s' to '%s'.\n", argv[1], argv[2]);
    return 0;
}