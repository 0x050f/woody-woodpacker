#include "woody_woodpacker.h"

void		*get_text_section(t_elf *elf)
{
	uint16_t		i;
	Elf64_Shdr		*str_table;
	char			*str;

	str_table = NULL;
	for (i = 0; i < elf->header->e_shnum; i++)
	{
		if (elf->sections[i].sh_type == SHT_STRTAB)
			str_table = &elf->sections[i];
	}
	if (!str_table)
		return (NULL);
	str = elf->addr + str_table->sh_offset;
	i = 0;
	while (i < elf->header->e_shnum && ft_strcmp(str + elf->sections[i].sh_name, ".text"))
		i++;
	if (i == elf->header->e_shnum)
		return (NULL);
	return (&elf->sections[i]);
}

void		add_injection(t_elf *elf, void **ptr_dst, Elf64_Addr new_entry, t_key *key)
{
	ft_memcpy(*ptr_dst, INJECT, INJECT_SIZE - (sizeof(uint64_t) * 6) + 5);
	*ptr_dst += INJECT_SIZE - ((sizeof(uint64_t) * 6) + 5);
	ft_memcpy(*ptr_dst + sizeof(uint64_t) * 0, &elf->pt_load->p_vaddr, sizeof(uint64_t));
	ft_memcpy(*ptr_dst + sizeof(uint64_t) * 1, &elf->text_section->sh_offset, sizeof(uint64_t));
	ft_memcpy(*ptr_dst + sizeof(uint64_t) * 2, &elf->text_section->sh_size, sizeof(uint64_t));
	ft_memcpy(*ptr_dst + sizeof(uint64_t) * 3, &new_entry, sizeof(uint64_t));
	ft_memcpy(*ptr_dst + sizeof(uint64_t) * 4, &elf->header->e_entry, sizeof(uint64_t)); // old_entry
	ft_memcpy(*ptr_dst + sizeof(uint64_t) * 5, &key->size, sizeof(uint64_t));
	*ptr_dst += sizeof(uint64_t) * 6;
	ft_memcpy(*ptr_dst, INJECT + (INJECT_SIZE - 5), 5);
	*ptr_dst += 5;
	ft_memcpy(*ptr_dst, key->str, key->size);
	*ptr_dst += key->size;
}

int			fill_binary(t_elf *elf, t_key *key, void *dst, int type)
{
	void			*ptr_src;
	void			*end;
	Elf64_Addr		new_entry;
	Elf64_Off		shoff;
	char			*encrypt;

	ptr_src = elf->addr;
	end = elf->addr + elf->size;
	new_entry = elf->pt_load->p_vaddr + elf->pt_load->p_offset + elf->pt_load->p_memsz;
	ft_memcpy(dst, ptr_src, (unsigned long)&elf->header->e_entry - (unsigned long)ptr_src);
	dst += (unsigned long)&elf->header->e_entry - (unsigned long)ptr_src;
	ft_memcpy(dst, &new_entry, sizeof(new_entry));
	dst += sizeof(new_entry);
	ptr_src = (void *)&elf->header->e_entry + sizeof(elf->header->e_entry);
	if (type == ADD_PADDING)
	{
		shoff = elf->header->e_shoff + PAGE_SIZE;
		ft_memcpy(dst, ptr_src, (unsigned long)&elf->header->e_shoff - (unsigned long)ptr_src);
		dst += (unsigned long)&elf->header->e_shoff - (unsigned long)ptr_src;
		ft_memcpy(dst, &shoff, sizeof(shoff));
		dst += sizeof(shoff);
		ptr_src = (void *)&elf->header->e_shoff + sizeof(elf->header->e_shoff);
		ptr_src = add_padding_segments(elf, ptr_src, &dst, key);
	}
	else
		ptr_src = update_segment_sz(ptr_src, &dst, elf->pt_load, key);
	encrypt = xor_encrypt(elf->addr + elf->text_section->sh_offset, elf->text_section->sh_size, key);
	ft_memcpy(dst, ptr_src, ((unsigned long)elf->addr + (unsigned long)elf->text_section->sh_offset) - (unsigned long)ptr_src);
	dst += ((unsigned long)elf->addr + (unsigned long)elf->text_section->sh_offset) - (unsigned long)ptr_src;
	ft_memcpy(dst, encrypt, elf->text_section->sh_size);
	dst += elf->text_section->sh_size;
	free(encrypt);
	ptr_src = elf->addr + elf->text_section->sh_offset + elf->text_section->sh_size;
	ft_memcpy(dst, ptr_src, ((unsigned long)elf->addr + (unsigned long)elf->pt_load->p_offset + elf->pt_load->p_memsz) - (unsigned long)ptr_src);
	dst += ((unsigned long)elf->addr + (unsigned long)elf->pt_load->p_offset + elf->pt_load->p_memsz) - (unsigned long)ptr_src;
	ptr_src = elf->addr + elf->pt_load->p_offset + elf->pt_load->p_memsz;
	add_injection(elf, &dst, new_entry, key);
	if (type ==  ADD_PADDING)
	{
		Elf64_Phdr		*segments;

		segments = elf->pt_load + 1;
		int diff = (INJECT_SIZE + key->size) - (segments->p_offset - (elf->pt_load->p_offset + elf->pt_load->p_filesz));
		ft_memset(dst, 0, PAGE_SIZE - (diff % PAGE_SIZE));
		dst += PAGE_SIZE - (diff % PAGE_SIZE);
		ptr_src = elf->addr + segments->p_offset;
		ptr_src = add_padding_sections(elf, ptr_src, &dst);
	}
	else
		ptr_src += INJECT_SIZE + key->size;
	ft_memcpy(dst, ptr_src, end - ptr_src);
	return (0);
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

int			init_elf(t_elf *elf, void *addr, long size)
{
	int				i;
	Elf64_Phdr		*next;

	elf->addr = addr;
	elf->size = size;
	elf->header = addr;
	if ((long)elf->header->e_phoff > size || (long)elf->header->e_shoff > size)
		return (CORRUPTED_FILE);
	elf->segments = addr + elf->header->e_phoff;
	elf->sections = addr + elf->header->e_shoff;
	elf->text_section = get_text_section(elf);
	if (!elf->text_section)
		return (CORRUPTED_FILE);
	elf->pt_load = elf->segments;
	next = (elf->header->e_phnum > 1) ? elf->pt_load + 1 : NULL;
	for (i = 0; i < elf->header->e_phnum; i++)
	{
		if (elf->pt_load->p_type == PT_LOAD && next && next->p_type == PT_LOAD && elf->pt_load->p_flags & PF_X)
			break;
		elf->pt_load = (i < elf->header->e_phnum) ? elf->pt_load + 1 : NULL;
		next = (i < elf->header->e_phnum - 1) ? elf->pt_load + 1 : NULL;
	}
	if (elf->pt_load->p_type != PT_LOAD || !next || next->p_type != PT_LOAD)
		return (errno = EINVAL);
	return (0);
}

int			create_woody_file(void *addr, long size)
{
	int				ret;
	void			*dst;
	long			size_dst;
	int				type;
	t_elf			elf;
	t_key			key;
	Elf64_Phdr		*next;

	ret = init_elf(&elf, addr, size);
	if (ret)
		return (ret);
	key.size = KEY_SIZE;
	key.str = generate_key(key.size);
	type = 0;
	next = elf.pt_load + 1;
	if (elf.pt_load->p_offset + elf.pt_load->p_memsz + INJECT_SIZE + key.size > next->p_offset + elf.pt_load->p_offset)
		type = ADD_PADDING;
	size_dst = size;
	if (type == ADD_PADDING)
		size_dst += (((INJECT_SIZE + key.size) / PAGE_SIZE) + 1) * PAGE_SIZE;
	if (!(dst = malloc(size_dst)))
	{
		free(key.str);
		return (MALLOC_ERROR);
	}
	fill_binary(&elf, &key, dst, type);
	ret = write_file("woody", dst, size_dst);
	free(dst);
	if (ret)
	{
		free(key.str);
		return (OUTPUT_ERROR);
	}
	write(STDOUT_FILENO, key.str, key.size);
	free(key.str);
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

void		print_error(char *argv[], int code)
{
	if (code == CORRUPTED_FILE)
		fprintf(stderr, "%s: %s: %s\n", argv[0], argv[1], "File corrupted");
	else if (code == MALLOC_ERROR)
		fprintf(stderr, "%s: %s: %s\n", argv[0], argv[1], "Malloc error");
	else if	 (code == OUTPUT_ERROR)
		fprintf(stderr, "%s: %s: %s\n", argv[0], "woody", strerror(errno));
	else if (errno)
		fprintf(stderr, "%s: %s: %s\n", argv[0], argv[1], strerror(errno));
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
		print_error(argv, ret);
	return (ret);
}
