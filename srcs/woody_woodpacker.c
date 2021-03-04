#include "woody_woodpacker.h"

int			encrypt(void *addr, int size)
{
	(void)addr;
	(void)size;
	return (0);
}

int			create_woody_file(void *addr, int size)
{
//	void			*ptr;
	int				fd;
	Elf64_Ehdr		*header;
//	Elf64_Phdr		*segments;
	Elf64_Shdr		*section;
	void			*offset;
	void			*end_sections;
	void			*offset_str;
//	Elf64_Shdr		inject_section;
	char			*name;


	(void)size;
	header = addr;
//	segments = addr + header->e_phoff;
	offset = addr + header->e_shoff;
	end_sections = offset + header->e_shnum * header->e_shentsize;
	section = offset + header->e_shstrndx * header->e_shentsize;
	offset_str = addr + section->sh_offset;
	while ((void *)offset < end_sections)
	{
		section = offset;
		name = offset_str + section->sh_name;
		if (!strcmp(name, ".text"))
		{
//			inject_section.sh_addr = section->sh_addr;
//			inject_section.sh_offset = section->sh_offset;
//			inject_section.sh_size = section->sh_size;
//			section->sh_flags |= SHF_WRITE; // ?
			break;
		}
		offset += header->e_shentsize;
	}
	fd = open("woody", O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	write(fd, addr, sizeof(header->e_ident) + sizeof(header->e_type) + sizeof(header->e_machine) + sizeof(header->e_version));
	Elf64_Addr new_entry = (unsigned long)offset + size;
	write(fd, &new_entry, sizeof(header->e_entry));
	addr += sizeof(header->e_ident) + sizeof(header->e_type) + sizeof(header->e_machine) + sizeof(header->e_version) + sizeof(header->e_entry);
	size -= sizeof(header->e_ident) + sizeof(header->e_type) + sizeof(header->e_machine) + sizeof(header->e_version) + sizeof(header->e_entry);
	write(fd, addr, size);
	int				inject_size;
	void			*injection;

	inject_size = _get_inject_size();
	injection = malloc(inject_size);
	ft_memcpy(injection, (void *)_inject, inject_size);
	write(fd, injection, inject_size);
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
	addr = mmap(NULL, size, PROT_READ, MAP_FILE | MAP_SHARED, fd, 0);
	if (addr == MAP_FAILED)
	{
		close(fd);
		return (errno);
	}
	// TODO: if the program is a executable encrypt it
	if (check_file(addr) == FILE_EXEC)
		ret = encrypt(addr, size);
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
