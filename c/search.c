#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "dbg.h"


char *add_str(char *cmd, char *dir)
{
  char *full_cmd = malloc(strlen(cmd) + strlen(dir) + 1);
  memset(full_cmd, 0, strlen(cmd) + strlen(dir) + 1);
  strncat(full_cmd, cmd, strlen(cmd));
  strncat(full_cmd, dir, strlen(dir));
  return full_cmd;
}

int get_lines(FILE *fpipe, char *cmd)
{
  char c[100];
  int lines = 0;
  while (fgets(c, sizeof(c), fpipe)) {
    lines++;
  }
  pclose(fpipe);
  return lines;
}

void search(char **terms, int terms_length, char *logic,
	    FILE *fpipe, int lines, char *dir)
{
  int i = 0;
  int term = 0;  
  char c[100];
  struct stat path_stat;

  if (dir[strlen(dir) - 1] != '/') {
    strncat(dir, "/", 1);
  }
   
  for (i = 0; i < lines; i++) {
    fgets(c, sizeof(c), fpipe);
    char *dir_full = add_str(dir, c);
    dir_full[strcspn(dir_full, "\n")] = '\0';
    debug("dir_full: %s", dir_full);
    FILE *fp = fopen(dir_full, "r");
    if (fp == NULL) {
      debug("Permissions: %s", dir_full);
      free(dir_full);
      continue;
    }
    stat(dir_full, &path_stat);
    if (!S_ISREG(path_stat.st_mode)) {
      char *sub_cmd = add_str("ls ", dir_full);
      FILE *sub_dir = popen(sub_cmd, "r");
      int sub_lines = get_lines(sub_dir, sub_cmd);

      if (sub_lines == 0) {
	free(sub_cmd);
	fclose(fp);
	free(dir_full);
	continue;
      }
      sub_dir = popen(sub_cmd, "r");
      search(terms, terms_length, logic, sub_dir,
	     sub_lines, dir_full);
      free(sub_cmd);
      pclose(sub_dir);
    }
    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    if (fsize == 0 || fsize > 99999999) {
      pclose(fp);
      free(dir_full);
      continue;
    }
    fseek(fp, 0, SEEK_SET);
    char *string = malloc(fsize + 1);
    memset(string, 0, fsize + 1);
    fread(string, fsize, 1, fp);

    for (term = 1; term < terms_length; term++) {
      if (strstr(string, terms[term])) {
	printf("%s found in %s\n", terms[term], dir_full);
      }
    }
    free(dir_full);
    free(string);
    fclose(fp);
  }
}

int main(int argc, char *argv[])
{
  char *logic = "a";
  char *dir = "/var/log/";

  if (strncmp(argv[1], "-o", 2) == 0) {
    logic = "o";
  }
  if (argv[1][0] == '/') {
    dir = argv[1];
  } else if (argc > 2 && argv[2][0] == '/') {
    dir = argv[2];
  }
  char *cmd = add_str("ls ", dir);

  FILE *fpipe = popen(cmd, "r");

  
  int lines = get_lines(fpipe, cmd);
  fpipe = popen(cmd, "r");
  
  search(argv, argc, logic, fpipe, lines, dir);
  
  pclose(fpipe);
  free(cmd);

  return 0;
}
