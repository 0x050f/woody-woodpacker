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

void		write_inject(void **ptr_dst, Elf64_Shdr *text, Elf64_Addr new_entry, Elf64_Addr vaddr, Elf64_Addr old_entry)
{
	uint64_t	key;

	ft_memcpy(*ptr_dst, INJECT, INJECT_SIZE - (sizeof(uint64_t) * 6));
	*ptr_dst += INJECT_SIZE - (sizeof(uint64_t) * 6);
	printf("12 %p\n", *ptr_dst);
	ft_memcpy(*ptr_dst, &vaddr, sizeof(uint64_t));
	*ptr_dst += sizeof(uint64_t);
	printf("13 %p\n", *ptr_dst);
	ft_memcpy(*ptr_dst, &text->sh_offset, sizeof(uint64_t));
	*ptr_dst += sizeof(uint64_t);
	printf("14 %p\n", *ptr_dst);
	ft_memcpy(*ptr_dst, &text->sh_size, sizeof(uint64_t));
	*ptr_dst += sizeof(uint64_t);
	printf("15 %p\n", *ptr_dst);
	ft_memcpy(*ptr_dst, &new_entry, sizeof(uint64_t));
	*ptr_dst += sizeof(uint64_t);
	printf("16 %p\n", *ptr_dst);
	ft_memcpy(*ptr_dst, &old_entry, sizeof(uint64_t));
	*ptr_dst += sizeof(uint64_t);
	printf("17 %p\n", *ptr_dst);
	key = 0x123456789;
	ft_memcpy(*ptr_dst, &key, sizeof(uint64_t));
	*ptr_dst += sizeof(uint64_t);
	printf("18 %p\n", *ptr_dst);

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
	printf("4 %p\n", *ptr_dst);
	ptr_src = &segment->p_filesz;
	p_filesz = segment->p_filesz + INJECT_SIZE;
	ft_memcpy(*ptr_dst, &p_filesz, sizeof(segment->p_filesz));
	*ptr_dst += sizeof(segment->p_filesz);
	printf("5 %p\n", *ptr_dst);
	ptr_src += sizeof(segment->p_filesz);
	p_memsz = segment->p_memsz + INJECT_SIZE;
	ft_memcpy(*ptr_dst, &p_memsz, sizeof(segment->p_memsz));
	*ptr_dst += sizeof(segment->p_memsz);
	printf("6 %p\n", *ptr_dst);
	ptr_src += sizeof(segment->p_memsz);
	return (ptr_src);
}

void		*add_padding_segments(void *addr, void *ptr_src, void **ptr_dst, Elf64_Phdr *segment_inject)
{
	Elf64_Phdr		*segments;
	Elf64_Ehdr		*header;
	Elf64_Off		shoff;

	header = addr;
	segments = addr + header->e_phoff;
	shoff = header->e_shoff + PAGE_SIZE;
	for (int i = 0; i < header->e_phnum; i++)
	{
		if (segments[i].p_offset > (unsigned long)segment_inject->p_offset + segment_inject->p_filesz)
		{
			shoff = segments[i].p_offset + PAGE_SIZE;
			ft_memcpy(*ptr_dst, ptr_src, (unsigned long)&segments[i].p_offset - (unsigned long)ptr_src);
			printf("%p\n", ptr_dst);
			*ptr_dst += (unsigned long)&segments[i].p_offset - (unsigned long)ptr_src;
			ft_memcpy(*ptr_dst, &shoff, sizeof(shoff));
			printf("%p\n", ptr_dst);
			*ptr_dst += sizeof(shoff);
			ptr_src = (void *)&segments[i].p_offset + sizeof(segments[i].p_offset);
		}
		else if ((unsigned long)&segments[i] == (unsigned long)segment_inject)
			ptr_src = write_new_segment_sz(ptr_src, ptr_dst, segment_inject);
	}
	return (ptr_src);
}

int			write_injection(void *src, void *dst, long size, Elf64_Phdr *segment, int type)
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
	printf("1 %p\n", ptr_dst);
	new_entry = segment->p_vaddr + segment->p_offset + segment->p_memsz;
	ft_memcpy(ptr_dst, ptr_src, (unsigned long)&header->e_entry - (unsigned long)ptr_src);
	ptr_dst += (unsigned long)&header->e_entry - (unsigned long)ptr_src;
	printf("2 %p\n", ptr_dst);
	ft_memcpy(ptr_dst, &new_entry, sizeof(new_entry));
	ptr_dst += sizeof(new_entry);
	printf("3 %p\n", ptr_dst);
	ptr_src = (void *)&header->e_entry + sizeof(header->e_entry);
	Elf64_Shdr		*sections;
	Elf64_Off		shoff;
//	size_t			payload_vaddr;

	sections = src + header->e_shoff;
//	payload_vaddr = segment->p_vaddr + segment->p_filesz;
	if (type == ADD_PADDING)
	{
		shoff = header->e_shoff + PAGE_SIZE;
		ft_memcpy(ptr_dst, ptr_src, (unsigned long)&header->e_shoff - (unsigned long)ptr_src);
		ptr_dst += (unsigned long)&header->e_shoff - (unsigned long)ptr_src;
		printf("%p\n", ptr_dst);
		ft_memcpy(ptr_dst, &shoff, sizeof(shoff));
		ptr_dst += sizeof(shoff);
		printf("%p\n", ptr_dst);
		ptr_src = (void *)&header->e_shoff + sizeof(header->e_shoff);
		ptr_src = add_padding_segments(src, ptr_src, &ptr_dst, segment);
	}
	else
		ptr_src = write_new_segment_sz(ptr_src, &ptr_dst, segment);
	printf("7 %p\n", ptr_dst);
// ---------------
	Elf64_Shdr		*text;
	text = get_text_section(src);
	char			*encrypt;
	encrypt = xor_encrypt(src + text->sh_offset, text->sh_size, 0x123456789);
	ft_memcpy(ptr_dst, ptr_src, ((unsigned long)src + (unsigned long)text->sh_offset) - (unsigned long)ptr_src);
	ptr_dst += ((unsigned long)src + (unsigned long)text->sh_offset) - (unsigned long)ptr_src;
	printf("8 %p\n", ptr_dst);
	ft_memcpy(ptr_dst, encrypt, text->sh_size);
	ptr_dst += text->sh_size;
	free(encrypt);
	printf("9 %p\n", ptr_dst);
//
	ptr_src = src + text->sh_offset + text->sh_size;
	ft_memcpy(ptr_dst, ptr_src, ((unsigned long)src + (unsigned long)segment->p_offset + segment->p_memsz) - (unsigned long)ptr_src);
	ptr_dst += ((unsigned long)src + (unsigned long)segment->p_offset + segment->p_memsz) - (unsigned long)ptr_src;
	printf("10 %p\n", ptr_dst);
	ptr_src = src + segment->p_offset + segment->p_memsz;
	write_inject(&ptr_dst, text, new_entry, segment->p_vaddr, header->e_entry);
	printf("11 %p\n", ptr_dst);
	if (type ==  ADD_PADDING)
	{
		printf("ADD PADDING\n");
		Elf64_Phdr		*segments;

		segments = segment + 1;
		char	zero[PAGE_SIZE];
		ft_memset(zero, 0, PAGE_SIZE);
		if (segment->p_offset + segment->p_memsz + INJECT_SIZE < segments->p_offset)
		{
			printf("aie\n");
			ft_memcpy(ptr_dst, ptr_src, ((unsigned long)src + segments->p_offset) - (unsigned long)ptr_src);
			ptr_dst += ((unsigned long)src + segments->p_offset) - (unsigned long)ptr_src;
			ft_memset(ptr_dst, 0, PAGE_SIZE);
			ptr_dst += PAGE_SIZE;
		}
		else
		{
			int diff = INJECT_SIZE - (segments->p_offset - (segment->p_offset + segment->p_filesz));
			ft_memset(ptr_dst, 0, PAGE_SIZE - (diff % PAGE_SIZE));
			ptr_dst += PAGE_SIZE % (diff - PAGE_SIZE);
		}
		ptr_src = src + segments->p_offset;
		for (int i = 0; i < header->e_shnum; i++)
		{
			if ((unsigned long)sections[i].sh_offset > (unsigned long)segment->p_offset + segment->p_filesz)
			{
				shoff = sections[i].sh_offset + PAGE_SIZE;
				ft_memcpy(ptr_dst, ptr_src, (unsigned long)&sections[i].sh_offset - (unsigned long)ptr_src);
				ptr_dst += (unsigned long)&sections[i].sh_offset - (unsigned long)ptr_src;
				ft_memcpy(ptr_dst, &shoff, sizeof(shoff));
				ptr_dst += sizeof(shoff);
				ptr_src = (void *)&sections[i].sh_offset + sizeof(sections[i].sh_offset);
			}
		}
	}
	else
		ptr_src += INJECT_SIZE;
	printf("inject size %d\n", INJECT_SIZE);
	printf("truc deja alloué %ld\n", ptr_dst - dst);
	printf("restant %ld\n", end - ptr_src);
	ft_memcpy(ptr_dst, ptr_src, end - ptr_src);
	printf("12 %p\n", ptr_dst);
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

int			create_woody_file(void *addr, long size)
{
	void			*dst;
	long			size_dst;
	int				fd;
	int				i;
	Elf64_Ehdr		*header;
	Elf64_Phdr		*segments;
	Elf64_Phdr		*load_segment;
	Elf64_Phdr		*next;

	header = addr;
	if (header->e_type != ET_EXEC && header->e_type != ET_DYN) // TODO: ERROR NOT EXEC ? (Check every possible exec)
		printf("NOT EXECUTABLE -> 0x%0x\n", header->e_type);
	segments = addr + header->e_phoff;
	i = 0;
	load_segment = NULL;
	next = (i < header->e_phnum - 1) ? segments + 1 : NULL;
	while (i < header->e_phnum)
	{
		if (segments->p_type == PT_LOAD && next->p_type == PT_LOAD && segments->p_flags & PF_X)
		{
			load_segment = segments;
			break;
		}
		segments = (i < header->e_phnum) ? segments + 1 : NULL;
		next = (i < header->e_phnum - 1) ? segments + 1 : NULL;
		i++;
	}
	
	// TODO: do it first with malloc/mmap, and then paste in it to avoid scribble shit
	if (segments->p_type == PT_LOAD && next->p_type == PT_LOAD && segments->p_offset + segments->p_memsz + INJECT_SIZE <= next->p_offset + segments->p_offset)
	{
		size_dst = get_size_needed(addr, size, 0);
		printf("size %ld\n", size_dst);
		dst = malloc(size_dst);
		write_injection(addr, dst, size, segments, 0);
	}
	else // ADD PADDING
	{
		printf("size base %ld\n", size);
		size_dst = get_size_needed(addr, size, ADD_PADDING);
		printf("size %ld\n", size_dst);
		dst = malloc(size_dst);
		write_injection(addr, dst, size, load_segment, ADD_PADDING);
	}
	fd = open("woody", O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	void *ptr;
	ptr = dst;
	i = 0;
	while (i < size_dst)
	{
		if (i + 4096 <= size_dst)
		{
			i += write(fd, ptr, 4096);
			ptr += 4096;
		}
		else
			i += write(fd, ptr, size_dst % 4096);
	}
	free(dst);
	close(fd);
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
	unsigned char	magic[EI_NIDENT];

	ft_memcpy(magic, addr, sizeof(magic));
	if ((magic[EI_MAG0] == ELFMAG0) &&
		(magic[EI_MAG1] == ELFMAG1) &&
		(magic[EI_MAG2] == ELFMAG2) &&
		(magic[EI_MAG3] == ELFMAG3))
	{
		if (magic[EI_CLASS] == ELFCLASS64)
			return (FILE_EXEC);
		// TODO: bonus 32 bits
	}
	return (FILE_UNDF);
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
	size = lseek(fd, (size_t)0, SEEK_END);
	if (size < 0)
	{
		close(fd);
		if (!errno) // Don't know why it doesn't fill errno in this case
			errno = EISDIR; // EINVAL ?
		return (errno);
	}
	addr = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (addr == MAP_FAILED)
	{
		close(fd);
		return (errno);
	}
	// TODO: Compression
	if (check_file(addr) == FILE_EXEC)
		ret = 0;
	else
		ret = EINVAL;
	if (!ret)
		create_woody_file(addr, size);
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
