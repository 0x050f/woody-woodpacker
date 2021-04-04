#include "woody_woodpacker.h"

static unsigned long int next_rand = 1;

void		ft_srand(unsigned int seed)
{
	next_rand = seed;
}

int			ft_rand(void)
{
	next_rand = next_rand * 1103515245 + 12345;
	return (unsigned int)(next_rand/65536) % 32768;
}

/**
* @brief Count the size of string s
*
* @param s
*
* @return size of s
*/
size_t		ft_strlen(const char *s)
{
	const char *ptr;

	ptr = s;
	while (*ptr)
		++ptr;
	return (ptr - s);
}

/**
* @brief Compare string s1 with s2 and return the unsigned char difference
*
* @param s1
* @param s2
*
* @return difference between s1 and s2
*/
int			ft_strcmp(const char *s1, const char *s2)
{
	int i;

	i = 0;
	while (s1[i] && s2[i] && s1[i] == s2[i])
		i++;
	return ((unsigned char)s1[i] - (unsigned char)s2[i]);
}

/**
* @brief Set len byte of b to c value
*
* @param b pointer
* @param c ascii char
* @param len length to set
*
* @return b
*/
void		*ft_memset(void *b, int c, size_t len)
{
	unsigned char	*pt;

	pt = (unsigned char *)b;
	while (len--)
		*pt++ = (unsigned char)c;
	return (b);
}

/**
* @brief Copy n byte from src to dst
*
* @param dst Memory destination
* @param src Memory source
* @param n Number of bytes need to be copied
*
* @return dst
*/
void		*ft_memcpy(void *dst, const void *src, size_t n)
{
	char	*pt_src;
	char	*pt_dst;

	pt_src = (char *)src;
	pt_dst = (char *)dst;
	while (n--)
		*pt_dst++ = *pt_src++;
	return (dst);
}
