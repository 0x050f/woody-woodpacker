#include "woody_woodpacker.h"

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
