#ifndef WOODY_WOODPACKER_H
# define WOODY_WOODPACKER_H

# include <elf.h>
# include <errno.h>
# include <fcntl.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/mman.h>
# include <syscall.h>
# include <time.h>
# include <unistd.h>

# ifndef INJECT
#  define INJECT ""
# endif

# ifndef INJECT_SIZE
#  define INJECT_SIZE 0
# endif

# define KEY_SIZE 256

# define ADD_PADDING 1

# define PAGE_SIZE 0x1000

# define CORRUPTED_FILE -1
# define MALLOC_ERROR -2
# define OUTPUT_ERROR -3

typedef struct		s_elf
{
	void			*addr;
	long			size;
	Elf64_Ehdr		*header;
	Elf64_Phdr		*segments;
	Elf64_Shdr		*sections;
	Elf64_Phdr		*pt_load; /* injected segment */
	Elf64_Shdr		*text_section;
}					t_elf;

typedef struct		s_key
{
	char			*str;
	size_t			size;
}					t_key;

/* binary.c */
int			fill_binary(t_elf *elf, t_key *key, void *dst, int type);

/* elf.c */
void		*get_text_section(t_elf *elf);
int			init_elf(t_elf *elf, void *addr, long size);
int			check_file(void *addr);

/* encryption.c */
char		*generate_key(size_t size);
char		*xor_encrypt(char *input, size_t input_len, t_key *key);

/* padding.c */
void		*update_segment_sz(void *ptr_src, void **ptr_dst, Elf64_Phdr *segment, t_key *key);
void		*add_padding_segments(t_elf *elf, void *ptr_src, void **ptr_dst, t_key *key);
void		*add_padding_sections(t_elf *elf, void *ptr_src, void **ptr_dst, t_key *key);

/* utils.c */
void		ft_srand(unsigned int seed);
int			ft_rand(void); // RAND_MAX assumed to be 32767
void		print_error(char *argv[], int code);
size_t		ft_strlen(const char *s);
int			ft_strcmp(const char *s1, const char *s2);
void		*ft_memset(void *b, int c, size_t len);
void		*ft_memcpy(void *dst, const void *src, size_t n);

/* ft_print_memory */
void	*ft_print_memory(void *addr, unsigned int size);
void	print_hexa_key(char *key, size_t size);

#endif
