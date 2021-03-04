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
# define FILE_UNDF 1

/* inject.s */
int			_get_inject_size(void);
void		_inject(void);

/* utils.c */
size_t		ft_strlen(const char *s);
int			ft_strcmp(const char *s1, const char *s2);
void		*ft_memcpy(void *dst, const void *src, size_t n);

#endif
