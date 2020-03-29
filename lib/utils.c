#include "utils.h"
#include "defines.h"

void clear(char *buffer, int length)
{
	int i;
	for (i = 0; i < length; i++)
	{
		buffer[i] = 0x00;
	}
}

void copyString(char *s1, char *s2, int *len)
{
	int i;
	for (i = 0; s1[i] != '\0'; ++i)
	{
		s2[i] = s1[i];
	}
	s2[i] = '\0';
	*len = i;
}

char compare2String(char *s1, char *s2)
{
	int i;
	for (i = 0; i < SECTOR_SIZE; ++i)
	{
		if (s1[i] == '\0' && s2[i] == '\0')
			return TRUE;
		if (s1[i] != s2[i])
			return FALSE;
	}
}