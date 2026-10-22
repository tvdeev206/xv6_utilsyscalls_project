#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"

void 
find(char *path, char *target, char **cmd_argv)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if ((fd = open(path, O_RDONLY)) < 0) {
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  if (fstat(fd, &st) < 0) {
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch (st.type) {
  case T_DEVICE:
  case T_FILE:
    break;

  case T_DIR:
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
      fprintf(2, "find: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';

    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
      if (de.inum == 0)
        continue;


      if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
        continue;

      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;

      if (stat(buf, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", buf);
        continue;
      }

      if (strcmp(p, target) == 0) {
        if (cmd_argv == 0) {
          printf("%s\n", buf);
        } else {
          int pid = fork();

          if (pid < 0) {
            fprintf(2, "find: fork failed\n");
          } else if (pid == 0) {
            char *new_argv[MAXARG-5];
            int i = 0;

            while (cmd_argv[i] != 0) {
              new_argv[i] = cmd_argv[i];
              i++;
            }

            new_argv[i] = buf;
            i++;
            new_argv[i] = 0;

            exec(new_argv[0], new_argv);
            fprintf(2, "find: exec %s failed\n", new_argv[0]);
            exit(1);
          } else {
            wait(0);
          }
        }
      }

      if (st.type == T_DIR) {
        find(buf, target, cmd_argv);
      }
    }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  if (argc == 3) {
    find(argv[1], argv[2], 0);
  } else if (argc >= 5 && strcmp(argv[3], "-exec") == 0) {
    find(argv[1], argv[2], &argv[4]);
  } else {
    fprintf(2, "find: find <path> <target> [-exec <cmd> ...]\n");
    exit(1);
  }
  exit(0);
}