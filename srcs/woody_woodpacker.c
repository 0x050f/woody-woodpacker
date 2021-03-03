#include "woody_woodpacker.h"

int			encrypt(void *addr, int size)
{
	(void)addr;
	(void)size;
	return (0);
}

int			create_woody(void *addr, int size)
{
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
