#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	if (argc == 1)
	{
		printf("%s RecoveryLogFile\n", argv[0]);
		printf("%s I RecoveryLogFile\n", argv[0]);
		printf("%s L RecoveryLogFile\n", argv[0]);
		exit(0);
	}
	else if (argc == 2)
	{
		eggRecoveryLog_inspect(argv[1]);
	}
	else if (argc == 3)
	{
		if (argv[1][0] == 'I')
			eggRecoveryLog_readloginfo(argv[2]);
		else if (argv[1][0] == 'L')
                    eggRecoveryLog_readlog(argv[2], 0);
	}
}
