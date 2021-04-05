#include "woody_woodpacker.h"

/**
* @brief Get the text section from the elf file
*
* @param elf struct elf
*
* @return text section
*/
void		*get_text_section(t_elf *elf)
{
	uint16_t		i;
	Elf64_Shdr		*str_table;
	char			*str;

	str_table = NULL;
	for (i = 0; i < elf->header->e_shnum; i++)
	{
		if (elf->sections[i].sh_offset > (unsigned int)elf->size)
			return (NULL);
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

/**
* @brief init the elf struct with pointers from addr
*
* @param elf struct elf
* @param addr pointer to the source file
* @param size size of the source file
*
* @return 0 or errno if any problem occurs
*/
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

/**
* @brief Check if the file got the ELF64 magic bytes
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
	}
	return (ET_NONE);
}
