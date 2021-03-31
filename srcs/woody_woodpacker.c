#include "woody_woodpacker.h"

void		*get_text_section(void *addr)
{
	uint16_t		i;
	Elf64_Ehdr		*header;
	Elf64_Shdr		*sections;
	Elf64_Shdr		*str_table;
	char			*str;

	header = addr;
	sections = addr + header->e_shoff;
	for (i = 0; i < header->e_shnum; i++)
	{
		if (sections[i].sh_type == SHT_STRTAB)
			str_table = &sections[i];
	}
	str = addr + str_table->sh_offset;
	i = 0;
	while (i < header->e_shnum && ft_strcmp(str + sections[i].sh_name, ".text"))
		i++;
	if (i == header->e_shnum)
	{
		return (NULL);
	}
	return (&sections[i]);
}

typedef struct	s_node
{
	unsigned char	c;
	unsigned int	freq;
	void			*right;
	void			*left;
}				t_node;

typedef struct	s_nodelst
{
	t_node		*node;
	void		*next;
}				t_nodelst;

t_node		*new_node(unsigned char c, unsigned int freq)
{
	t_node	*node;

	node = malloc(sizeof(t_node));
	if (!node)
		return (NULL);
	node->c = c;
	node->freq = freq;
	node->left = NULL;
	node->right = NULL;
	return (node);
}

t_nodelst	*new_nodelst(t_node *node)
{
	t_nodelst	*lst;

	lst = malloc(sizeof(t_nodelst));
	if (!lst)
		return (NULL);
	lst->node = node;
	lst->next = NULL;
	return (lst);
}

void		add_nodelst(t_nodelst **nodes, t_node *node) // add croissant
{
	t_nodelst	*prev;
	t_nodelst	*ptr;
	t_nodelst	*new;

	new = new_nodelst(node);
	ptr = *nodes;
	prev = NULL;
	while (ptr && ptr->node->freq < node->freq)
	{
		prev = ptr;
		ptr = ptr->next;
	}
	if (prev)
		prev->next = new;
	else
		*nodes = new;
	new->next = ptr;
}

void		remove_nodelst(t_nodelst **nodes, t_node *node)
{
	t_nodelst	*ptr;
	t_nodelst	*prev;

	ptr = *nodes;
	prev = NULL;
	while (ptr && ptr->node != node)
	{
		prev = ptr;
		ptr = ptr->next;
	}
	if (ptr)
	{
		if (prev)
			prev->next = ptr->next;
		else if (ptr->next)
			*nodes = ptr->next;
		else
			*nodes = NULL;
		free(ptr);
	}
}

void	print_binary_tree(t_node	*x, int n)
{
	int i;

	i = n;
	if (x->left)
		print_binary_tree(x->left, n + 1);
	while (i--)
		printf("   ");
	printf("%d\n", x->freq);
	if (x->right)
		print_binary_tree(x->right, n + 1);
}

int		print_translation(t_node	*x, char *string)
{
	int		ret;

	ret = 0;
	if (x->left)
		ret += print_translation(x->left, ft_strjoin(string, "0"));
	if (x->right)
		ret += print_translation(x->right, ft_strjoin(string, "1"));
	if (!x->right && !x->left)
	{
		printf("char ");
		if (x->c > 31 && x->c < 127)
			printf("'%c'", x->c);
		else
			printf("%d", x->c);
		printf(": %d occurence - ", x->freq);
		printf("%s\n", string);
		ret = x->freq * ft_strlen(string);
	}
	free(string);
	return (ret);
}

void		create_huffman_tree(int count[256])
{
	int			i;
	t_nodelst	*nodes;
//	t_nodelst	*ptr;

	nodes = NULL;
	i = 0;
	while (i < 256)
	{
		if (count[i] != 0)
			add_nodelst(&nodes, new_node(i, count[i]));
		i++;
	}
	/*
	ptr = nodes;
	while (ptr)
	{
		printf("char ");
		if (ptr->node->c > 31 && ptr->node->c < 127)
			printf("'%c'", ptr->node->c);
		else
			printf("%d", ptr->node->c);
		printf(": %d occurence\n", ptr->node->freq);
		ptr = ptr->next;
	}
	*/
	t_node	*left;
	t_node	*right;
	t_node	*tmp;
	while (nodes->next)
	{
		left = nodes->node;
		right = ((t_nodelst *)nodes->next)->node;
		remove_nodelst(&nodes, left);
		remove_nodelst(&nodes, right);
		tmp = new_node(0, left->freq + right->freq);
		tmp->left = left;
		tmp->right = right;
		add_nodelst(&nodes, tmp);
	}
//	print_binary_tree(nodes->node, 0);
	int ret;
	ret = print_translation(nodes->node, ft_strjoin("", ""));
	printf("=== AFTER COMPRESSION\n");
	printf("%d bits\n", ret);
	printf("%d bytes\n", ret / 8);
}

void		compress(unsigned char *addr, size_t size)
{
	size_t	i;
	int		count[256];

	i = 0;
	while (i < 256)
		count[i++] = 0;
	i = 0;
	while (i < size)
		count[(int)addr[i++]]++;
	create_huffman_tree(count);
	printf("=== BASE\n");
	printf("%ld bits\n", size * 8);
	printf("%ld bytes\n", size);
}

char		*xor_encrypt(char *input, size_t input_len, uint64_t key)
{
	size_t		i;
	char		*encrypt;

	if (!(encrypt = malloc(sizeof(char) * input_len)))
		return (NULL);
	for (i = 0; i < input_len; i++)
	{
		encrypt[i] = input[i] ^ key;
		key = ((key & 0xFF) << 56) | (key >> 8);
	}
	return (encrypt);
}

void		inject(void **ptr_dst, Elf64_Shdr *text, Elf64_Addr new_entry, Elf64_Addr vaddr, Elf64_Addr old_entry)
{
	uint64_t	key;

	ft_memcpy(*ptr_dst, INJECT, INJECT_SIZE - (sizeof(uint64_t) * 6));
	*ptr_dst += INJECT_SIZE - (sizeof(uint64_t) * 6);
	ft_memcpy(*ptr_dst + sizeof(uint64_t) * 0, &vaddr, sizeof(uint64_t));
	ft_memcpy(*ptr_dst + sizeof(uint64_t) * 1, &text->sh_offset, sizeof(uint64_t));
	ft_memcpy(*ptr_dst + sizeof(uint64_t) * 2, &text->sh_size, sizeof(uint64_t));
	ft_memcpy(*ptr_dst + sizeof(uint64_t) * 3, &new_entry, sizeof(uint64_t));
	ft_memcpy(*ptr_dst + sizeof(uint64_t) * 4, &old_entry, sizeof(uint64_t));
	key = 0x123456789;
	ft_memcpy(*ptr_dst + sizeof(uint64_t) * 5, &key, sizeof(uint64_t));
	*ptr_dst += sizeof(uint64_t) * 6;

}

/**
* @brief write modified p_filesz and p_memsz of the previous segment
*
* @param fd file descriptor to write
* @param ptr pointer of the mmaped binary
* @param segment previous segment
*
* @return moved ptr
*/
void		*write_new_segment_sz(void *ptr_src, void **ptr_dst, Elf64_Phdr *segment)
{
	uint64_t	p_filesz;
	uint64_t	p_memsz;

	ft_memcpy(*ptr_dst, ptr_src, (unsigned long)&segment->p_filesz - (unsigned long)ptr_src);
	*ptr_dst += (unsigned long)&segment->p_filesz - (unsigned long)ptr_src;
	ptr_src = &segment->p_filesz;
	p_filesz = segment->p_filesz + INJECT_SIZE;
	ft_memcpy(*ptr_dst, &p_filesz, sizeof(segment->p_filesz));
	*ptr_dst += sizeof(segment->p_filesz);
	ptr_src += sizeof(segment->p_filesz);
	p_memsz = segment->p_memsz + INJECT_SIZE;
	ft_memcpy(*ptr_dst, &p_memsz, sizeof(segment->p_memsz));
	*ptr_dst += sizeof(segment->p_memsz);
	ptr_src += sizeof(segment->p_memsz);
	return (ptr_src);
}

void		*add_padding_segments(void *addr, void *ptr_src, void **ptr_dst, Elf64_Phdr *segment)
{
	Elf64_Phdr		*segments;
	Elf64_Ehdr		*header;
	Elf64_Off		shoff;

	header = addr;
	segments = addr + header->e_phoff;
	shoff = header->e_shoff + PAGE_SIZE;
	for (int i = 0; i < header->e_phnum; i++)
	{
		if (segments[i].p_offset > (unsigned long)segment->p_offset + segment->p_filesz)
		{
			shoff = segments[i].p_offset + PAGE_SIZE;
			ft_memcpy(*ptr_dst, ptr_src, (unsigned long)&segments[i].p_offset - (unsigned long)ptr_src);
			*ptr_dst += (unsigned long)&segments[i].p_offset - (unsigned long)ptr_src;
			ft_memcpy(*ptr_dst, &shoff, sizeof(shoff));
			*ptr_dst += sizeof(shoff);
			ptr_src = (void *)&segments[i].p_offset + sizeof(segments[i].p_offset);
		}
		else if ((unsigned long)&segments[i] == (unsigned long)segment)
			ptr_src = write_new_segment_sz(ptr_src, ptr_dst, segment);
	}
	return (ptr_src);
}

void		*add_padding_sections(void *addr, void *ptr_src, void **ptr_dst, Elf64_Phdr *segment)
{
	Elf64_Ehdr		*header;
	Elf64_Shdr		*sections;
	Elf64_Off		shoff;

	header = addr;
	sections = addr + header->e_shoff;
	for (int i = 0; i < header->e_shnum; i++)
	{
		if ((unsigned long)sections[i].sh_offset > (unsigned long)segment->p_offset + segment->p_filesz)
		{
			shoff = sections[i].sh_offset + PAGE_SIZE;
			ft_memcpy(*ptr_dst, ptr_src, (unsigned long)&sections[i].sh_offset - (unsigned long)ptr_src);
			*ptr_dst += (unsigned long)&sections[i].sh_offset - (unsigned long)ptr_src;
			ft_memcpy(*ptr_dst, &shoff, sizeof(shoff));
			*ptr_dst += sizeof(shoff);
			ptr_src = (void *)&sections[i].sh_offset + sizeof(sections[i].sh_offset);
		}
	}
	return (ptr_src);
}

int			create_injection(void *src, void *dst, long size, Elf64_Phdr *segment, int type)
{
	void			*ptr_src;
	void			*ptr_dst;
	void			*end;
	Elf64_Ehdr		*header;
	Elf64_Addr		new_entry;

	ptr_src = src;
	ptr_dst = dst;
	end = src + size;
	header = src;
	new_entry = segment->p_vaddr + segment->p_offset + segment->p_memsz;
	ft_memcpy(ptr_dst, ptr_src, (unsigned long)&header->e_entry - (unsigned long)ptr_src);
	ptr_dst += (unsigned long)&header->e_entry - (unsigned long)ptr_src;
	ft_memcpy(ptr_dst, &new_entry, sizeof(new_entry));
	ptr_dst += sizeof(new_entry);
	ptr_src = (void *)&header->e_entry + sizeof(header->e_entry);
	Elf64_Off		shoff;
//	size_t			payload_vaddr;

//	payload_vaddr = segment->p_vaddr + segment->p_filesz;
	if (type == ADD_PADDING)
	{
		shoff = header->e_shoff + PAGE_SIZE;
		ft_memcpy(ptr_dst, ptr_src, (unsigned long)&header->e_shoff - (unsigned long)ptr_src);
		ptr_dst += (unsigned long)&header->e_shoff - (unsigned long)ptr_src;
		ft_memcpy(ptr_dst, &shoff, sizeof(shoff));
		ptr_dst += sizeof(shoff);
		ptr_src = (void *)&header->e_shoff + sizeof(header->e_shoff);
		ptr_src = add_padding_segments(src, ptr_src, &ptr_dst, segment);
	}
	else
		ptr_src = write_new_segment_sz(ptr_src, &ptr_dst, segment);
// ---------------
	Elf64_Shdr		*text;
	text = get_text_section(src);
	char			*encrypt;
	compress(src + text->sh_offset, text->sh_size);
	encrypt = xor_encrypt(src + text->sh_offset, text->sh_size, 0x123456789);
	ft_memcpy(ptr_dst, ptr_src, ((unsigned long)src + (unsigned long)text->sh_offset) - (unsigned long)ptr_src);
	ptr_dst += ((unsigned long)src + (unsigned long)text->sh_offset) - (unsigned long)ptr_src;
	ft_memcpy(ptr_dst, encrypt, text->sh_size);
	ptr_dst += text->sh_size;
	free(encrypt);
//
	ptr_src = src + text->sh_offset + text->sh_size;
	ft_memcpy(ptr_dst, ptr_src, ((unsigned long)src + (unsigned long)segment->p_offset + segment->p_memsz) - (unsigned long)ptr_src);
	ptr_dst += ((unsigned long)src + (unsigned long)segment->p_offset + segment->p_memsz) - (unsigned long)ptr_src;
	ptr_src = src + segment->p_offset + segment->p_memsz;
	inject(&ptr_dst, text, new_entry, segment->p_vaddr, header->e_entry);
	if (type ==  ADD_PADDING)
	{
		Elf64_Phdr		*segments;

		segments = segment + 1;
		int diff = INJECT_SIZE - (segments->p_offset - (segment->p_offset + segment->p_filesz));
		ft_memset(ptr_dst, 0, PAGE_SIZE - (diff % PAGE_SIZE));
		ptr_dst += PAGE_SIZE - (diff % PAGE_SIZE);
		ptr_src = src + segments->p_offset;
		ptr_src = add_padding_sections(src, ptr_src, &ptr_dst, segment);
	}
	else
		ptr_src += INJECT_SIZE;
	ft_memcpy(ptr_dst, ptr_src, end - ptr_src);
	return (0);
}

long		get_size_needed(void *addr, long size, int type)
{
	(void)addr;
	if (type == ADD_PADDING)
	{
		size += ((INJECT_SIZE / PAGE_SIZE) + 1) * PAGE_SIZE;
	}
	return (size);
}

int			write_file(char *filename, char *content, long size)
{
	int				fd;
	long			i;
	void			*ptr;

	fd = open(filename, O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	if (fd < 0)
		return (errno);
	i = 0;
	while (i < size)
	{
		ptr = content + i;
		if (size - i >= 4096)
			i += write(fd, ptr, 4096);
		else
			i += write(fd, ptr, size % 4096);
	}
	close(fd);
	return (0);
}

int			create_woody_file(void *addr, long size)
{
	int				ret;
	void			*dst;
	long			size_dst;
	int				i;
	int				type;
	Elf64_Ehdr		*header;
	Elf64_Phdr		*segments;
	Elf64_Phdr		*next;

	header = addr;
	segments = addr + header->e_phoff;
	next = (header->e_phnum > 1) ? segments + 1 : NULL;
	for (i = 0; i < header->e_phnum; i++)
	{
		if (segments->p_type == PT_LOAD && next && next->p_type == PT_LOAD && segments->p_flags & PF_X)
			break;
		segments = (i < header->e_phnum) ? segments + 1 : NULL;
		next = (i < header->e_phnum - 1) ? segments + 1 : NULL;
	}
	if (segments->p_type != PT_LOAD || !next || next->p_type != PT_LOAD)
		return (-1); // TODO: error
	// TODO: do it first with malloc/mmap, and then paste in it to avoid scribble shit
	type = 0;
	if (segments->p_offset + segments->p_memsz + INJECT_SIZE > next->p_offset + segments->p_offset)
		type = ADD_PADDING;
	size_dst = get_size_needed(addr, size, type);
	if (!(dst = malloc(size_dst)))
		return (-1); // TODO: error malloc
	create_injection(addr, dst, size, segments, type);
	ret = write_file("woody", dst, size_dst);
	free(dst);
	if (ret)
		return (ret);
	return (0);
}

/**
* @brief Check which file of type it is (FILE_EXEC, FILE_ENCRYPTED, FILE_UNDF)
*
* @param addr Mapped file
*
* @return Type of file
*/
int			check_file(void *addr)
{
	Elf64_Ehdr		*header;
	unsigned char	magic[EI_NIDENT];

	header = addr;
	ft_memcpy(magic, addr, sizeof(magic));
	if ((magic[EI_MAG0] == ELFMAG0) &&
		(magic[EI_MAG1] == ELFMAG1) &&
		(magic[EI_MAG2] == ELFMAG2) &&
		(magic[EI_MAG3] == ELFMAG3))
	{
		if (magic[EI_CLASS] == ELFCLASS64)
			return (header->e_type);
		// TODO: bonus 32 bits
	}
	return (ET_NONE);
}

/**
* @brief Open, then map the given file to pass parameter to check the file with check_file()
* and 'TODO' to encrypt the program if it's a executable
*
* @see check_file(void *addr)
*
* @param filename Name of the program
*
* @return 0 or a errno if any error happened
*/
int			woody_woodpacker(char *filename)
{
	int		fd;
	int		ret;
	long	size;
	void	*addr;

	fd = open(filename, O_RDONLY);
	if (fd < 0)
		return (errno);
	ret = read(fd, NULL, 0);
	if (ret < 0)
	{
		close(fd);
		return (errno);
	}
	size = lseek(fd, (size_t)0, SEEK_END);
	if (size < 0)
	{
		close(fd);
		if (!errno)
			errno = EINVAL;
		return (errno);
	}
	addr = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (addr == MAP_FAILED)
	{
		close(fd);
		return (errno);
	}
	// TODO: Compression
	ret = check_file(addr);
	if (ret == ET_EXEC || ret == ET_DYN)
		ret = 0;
	else
		ret = EINVAL;
	if (!ret)
		ret = create_woody_file(addr, size);
	else
		errno = ret;
	munmap(addr, size);
	close(fd);
	return (ret);
}

/**
* @brief Check number of arguments and call woody_woodpacker(), print error according
* to errno on any encounter problem by woody_woodpacker()
*
* @see woody_woodpacker(char *filename)
*
* @param argc
* @param argv[]
*
* @return Return of woody_woodpacker
*/
int			main(int argc, char *argv[])
{
	int		ret;

	if (argc < 2)
	{
		fprintf(stderr, "usage: %s file\n", argv[0]);
		return (EINVAL);
	}
	ret = woody_woodpacker(argv[1]);
	if (ret)
		fprintf(stderr, "%s: %s: %s\n", argv[0], argv[1], strerror(errno));
	return (ret);
}
