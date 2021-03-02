#ifndef WOODY_WOODPACKER_H
# define WOODY_WOODPACKER_H

# include <elf.h>
# include <errno.h>
# include <fcntl.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/mman.h>
# include <unistd.h>

# define FILE_EXEC 0
# define FILE_ENCRYPTED 1
# define FILE_UNDF 2

/* utils.c */
void		*ft_memcpy(void *dst, const void *src, size_t n);

#endif
