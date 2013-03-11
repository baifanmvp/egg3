#include <egg3/Egg3.h>
#include <stdio.h>
#include <assert.h>

int usage(char *prog)
{
    printf("usage: %s /absolute_egg_dir/\n", prog);
    exit(1);
}

int
main(int argc, char *argv[])
{
    if (argc < 2)
    {
        usage(argv[0]);
    }
    char *dir_name = argv[1];

    if (dir_name[0] != '/')
    {
        usage(argv[0]);
    }
    struct stat st;
    if (stat(dir_name, &st) < 0
        || ! S_ISDIR(st.st_mode)
        || access(dir_name, X_OK) != 0)
    {
        printf(stderr, "%s not directory or cannot access\n", dir_name);
        exit(-1);
    }
    
    int n;
    n = strlen(dir_name);
    
    char *base_name;
    base_name = malloc(n + 100);
    assert(base_name);
    strcpy(base_name, dir_name);
    
    n--;
    while (n >= 0 && base_name[n] == '/')
    {
        base_name[n] = '\0';
        n--;
    }
    strcat(base_name, "/egg.rlog");
    EGGRECOVERYHANDLE *hRecoveryHandle = eggRecoveryLog_init(base_name);
    eggRecoveryLog_destroy(hRecoveryHandle);
    free(base_name);
    return 0;
}
