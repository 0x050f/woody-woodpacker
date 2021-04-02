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

# ifndef INJECT
#  define INJECT ""
# endif

# ifndef INJECT_SIZE
#  define INJECT_SIZE 0
# endif

# define ADD_PADDING 1

# define PAGE_SIZE 0x1000

typedef struct	s_compressed_char
{
	unsigned char	c;
	int				bits;
	char			nb_bits;
}				t_compressed_char;

typedef struct		s_compression
{
	char				*result;
	int					nb_bits;
	t_compressed_char	*table;
}					t_compression;

t_compression		*compress(unsigned char *addr, int size);

/* utils.c */
char		*ft_strjoin(char const *s1, char const *s2);
size_t		ft_strlen(const char *s);
int			ft_strcmp(const char *s1, const char *s2);
void		*ft_memset(void *b, int c, size_t len);
void		*ft_memcpy(void *dst, const void *src, size_t n);

#endif
