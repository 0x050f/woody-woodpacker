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
		return (NULL);
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

void		write_inject_fd(int fd, Elf64_Shdr *text, Elf64_Addr new_entry, Elf64_Addr vaddr)
{
	uint64_t	key;

	write(fd, INJECT, INJECT_SIZE - (sizeof(uint64_t) * 5));
	write(fd, &vaddr, sizeof(uint64_t));
	write(fd, &text->sh_offset, sizeof(uint64_t));
	write(fd, &text->sh_size, sizeof(uint64_t));
	write(fd, &new_entry, sizeof(uint64_t));
	key = 0x123456789;
	write(fd, &key, sizeof(uint64_t));

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
void		*write_new_segment_sz_fd(int fd, void *ptr, Elf64_Phdr *segment)
{
	uint64_t	p_filesz;
	uint64_t	p_memsz;

	write(fd, ptr, (unsigned long)&segment->p_filesz - (unsigned long)ptr);
	ptr = &segment->p_filesz;
	p_filesz = segment->p_filesz + INJECT_SIZE;
	write(fd, &p_filesz, sizeof(segment->p_filesz));
	ptr += sizeof(segment->p_filesz);
	p_memsz = segment->p_memsz + INJECT_SIZE;
	write(fd, &p_memsz, sizeof(segment->p_memsz));
	ptr += sizeof(segment->p_memsz);
	return (ptr);
}

void		*add_padding_segments(int fd, void *addr, void *ptr, Elf64_Phdr *segment_inject)
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
			write(fd, ptr, (unsigned long)&segments[i].p_offset - (unsigned long)ptr);
			write(fd, &shoff, sizeof(shoff));
			ptr = (void *)&segments[i].p_offset + sizeof(segments[i].p_offset);
		}
		else if ((unsigned long)&segments[i] == (unsigned long)segment_inject)
			ptr = write_new_segment_sz_fd(fd, ptr, segment_inject);
	}
	return (ptr);
}

int			write_injection(int fd, void *addr, int size, Elf64_Phdr *segment, int type)
{
	void			*ptr;
	void			*end;
	Elf64_Ehdr		*header;
	Elf64_Addr		new_entry;

	ptr = addr;
	end = addr + size;
	header = addr;
	new_entry = segment->p_vaddr + segment->p_offset + segment->p_memsz;
	write(fd, ptr, (unsigned long)&header->e_entry - (unsigned long)ptr);
	write(fd, &new_entry, sizeof(new_entry));
	ptr = (void *)&header->e_entry + sizeof(header->e_entry);
	Elf64_Shdr		*sections;
	Elf64_Off		shoff;
//	size_t			payload_vaddr;

	sections = addr + header->e_shoff;
//	payload_vaddr = segment->p_vaddr + segment->p_filesz;
	if (type == ADD_PADDING)
	{
		shoff = header->e_shoff + PAGE_SIZE;
		write(fd, ptr, (unsigned long)&header->e_shoff - (unsigned long)ptr);
		write(fd, &shoff, sizeof(shoff));
		ptr = (void *)&header->e_shoff + sizeof(header->e_shoff);
		ptr = add_padding_segments(fd, addr, ptr, segment);
	}
	else
		ptr = write_new_segment_sz_fd(fd, ptr, segment);
// ---------------
	Elf64_Shdr		*text;
	text = get_text_section(addr);
	char			*encrypt;
	encrypt = xor_encrypt(addr + text->sh_offset, text->sh_size, 0x123456789);
	write(fd, ptr, ((unsigned long)addr + (unsigned long)text->sh_offset) - (unsigned long)ptr);
	write(fd, encrypt, text->sh_size);
//
	ptr = addr + text->sh_offset + text->sh_size;
	write(fd, ptr, ((unsigned long)addr + (unsigned long)segment->p_offset + segment->p_memsz) - (unsigned long)ptr);
	ptr = addr + segment->p_offset + segment->p_memsz;
	write_inject_fd(fd, text, new_entry, segment->p_vaddr);
	if (type ==  ADD_PADDING)
	{
		Elf64_Phdr		*segments;

		segments = segment + 1;
		char	zero[PAGE_SIZE];
		ft_memset(zero, 0, PAGE_SIZE);
		if (segment->p_offset + segment->p_memsz + INJECT_SIZE < segments->p_offset)
		{
			write(fd, ptr, ((unsigned long)addr + segments->p_offset) - (unsigned long)ptr);
			write(fd, zero, PAGE_SIZE);
		}
		else
		{
			int diff = INJECT_SIZE - (segments->p_offset - (segment->p_offset + segment->p_filesz));
			while (diff > PAGE_SIZE)
				diff -= PAGE_SIZE;
			write(fd, zero, PAGE_SIZE - diff);
		}
		ptr = addr + segments->p_offset;
		for (int i = 0; i < header->e_shnum; i++)
		{
			if ((unsigned long)sections[i].sh_offset > (unsigned long)segment->p_offset + segment->p_filesz)
			{
				shoff = sections[i].sh_offset + PAGE_SIZE;
				write(fd, ptr, (unsigned long)&sections[i].sh_offset - (unsigned long)ptr);
				write(fd, &shoff, sizeof(shoff));
				ptr = (void *)&sections[i].sh_offset + sizeof(sections[i].sh_offset);
			}
		}
	}
	else
		ptr += INJECT_SIZE;
	write(fd, ptr, end - ptr);
	return (0);
}

int			create_woody_file(void *addr, int size)
{
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
	fd = open("woody", O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	if (segments->p_type == PT_LOAD && next->p_type == PT_LOAD && segments->p_offset + segments->p_memsz + INJECT_SIZE <= next->p_offset + segments->p_offset)
		write_injection(fd, addr, size, segments, 0);
	else // ADD PADDING
		write_injection(fd, addr, size, load_segment, ADD_PADDING);
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
	int		size;
	int		ret;
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
	// TODO: if the program is a executable encrypt it
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
