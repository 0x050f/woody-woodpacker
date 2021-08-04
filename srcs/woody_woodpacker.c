#include "woody_woodpacker.h"

/**
* @brief Write content in the file 'filename'
*
* @param filename name of the file
* @param content content to write
* @param size size to write
*
* @return 0 or errno on error
*/
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

/**
* @brief Generate a key to encrypt the text section, then allocate memory needed
* to write the new woody file then fill it.
*
* @param addr source pointer
* @param size size of source file
*
* @return 0 or error on problems
*/
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
	if (elf.pt_load->p_offset + elf.pt_load->p_memsz + INJECT_SIZE + key.size > next->p_offset)
	{
		type = ADD_PADDING;
	}
	size_dst = size;
	if (type == ADD_PADDING)
		size_dst += (((INJECT_SIZE + key.size) / PAGE_SIZE) + 1) * PAGE_SIZE;
	if (!(dst = malloc(size_dst)))
	{
		free(key.str);
		return (MALLOC_ERROR);
	}
	ret = fill_binary(&elf, &key, dst, type);
	if (ret)
	{
		free(dst);
		free(key.str);
		return (OUTPUT_ERROR);
	}
	ret = write_file("woody", dst, size_dst);
	free(dst);
	if (ret)
	{
		free(key.str);
		return (OUTPUT_ERROR);
	}
	write(STDOUT_FILENO, "========================== KEY =========================\n", 57);
	ft_print_memory(key.str, key.size);
	write(STDOUT_FILENO, "=================== COPY/PASTE FORMAT ==================\n", 57);
	print_hexa_key(key.str, key.size);
	free(key.str);
	return (0);
}

/**
* @brief Open, then map the given file to pass parameter to check the file with check_file()
* and create the woody file
*
* @see create_woody_file(void *addr, long size)
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
		ret = WRONG_FILETYPE;
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

	if (argc != 2)
	{
		fprintf(stderr, "usage: %s file\n", argv[0]);
		return (EINVAL);
	}
	ret = woody_woodpacker(argv[1]);
	if (ret)
		print_error(argv, ret);
	return (ret);
}
