#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"



void 
find (char *path, char *target)
{
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if ((fd = open(path, O_RDONLY)) < 0){
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0){
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    switch(st.type){
        case T_DEVICE:
        case T_FILE:
            if (strcmp(de.name, target) == 0){
                printf("%s\n", path);
                break;
            }
            break;
        case T_DIR:
            strcpy(buf, path);
            p = buf + strlen(buf);
            *p++ = '/';

            while (read(fd, &de, sizeof(de)) == sizeof(de)){
                if (de.inum == 0) continue;
                if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) continue;

                memmove(p, de.name, DIRSIZ);
                p[DIRSIZ] = 0;

                if (stat(buf, &st) < 0){
                    printf("find: cannot stat %s\n", buf);
                    continue;
                }

                if (strcmp(de.name, target) == 0){
                    printf("%s\n", buf);
                } else {
                    find(buf, target);
                }
            }
            break;
    }
    close(fd);
}

int
main(int argc, char *argv[])
{
  if (argc == 3){
    find(argv[1], argv[2]); // In path tim duoc cho target 
  } else {
    fprintf(2, "find: insufficent arguments - find <path> <target>\n");
    exit(1);
  }
  exit(0);
}