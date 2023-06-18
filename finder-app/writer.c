#include<stdio.h>
#include<syslog.h>

int main(int argc, char *argv[])
{
	openlog(NULL, 0, LOG_USER);

	char *filepath=argv[1];
	char *str=argv[2];

	FILE *file;

	if ( argc < 3 )
	{
		syslog(LOG_ERR,"Invalid arguments");

		return 1;
	}

	file=fopen(filepath,"w");

	if (file == NULL) {
		syslog(LOG_ERR, "Unable to open file");

		return 1;
	}
	else {
		fprintf(file, "%s", str);
		syslog(LOG_DEBUG, "Writing %s to %s", str, filepath);

		return 0;
	}
}
