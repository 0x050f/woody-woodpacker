#include "woody_woodpacker.h"

/**
* @brief write modified p_filesz and p_memsz of the previous segment
*
* @param fd file descriptor to write
* @param ptr pointer of the mmaped binary
* @param segment previous segment
*
* @return moved ptr
*/
void		*update_segment_sz(void *ptr_src, void **ptr_dst, Elf64_Phdr *segment, t_key *key)
{
	uint64_t	p_filesz;
	uint64_t	p_memsz;

	ft_memcpy(*ptr_dst, ptr_src, (unsigned long)&segment->p_filesz - (unsigned long)ptr_src);
	*ptr_dst += (unsigned long)&segment->p_filesz - (unsigned long)ptr_src;
	ptr_src = &segment->p_filesz;
	p_filesz = segment->p_filesz + INJECT_SIZE + key->size;
	ft_memcpy(*ptr_dst, &p_filesz, sizeof(segment->p_filesz));
	*ptr_dst += sizeof(segment->p_filesz);
	ptr_src += sizeof(segment->p_filesz);
	p_memsz = segment->p_memsz + INJECT_SIZE + key->size;
	ft_memcpy(*ptr_dst, &p_memsz, sizeof(segment->p_memsz));
	*ptr_dst += sizeof(segment->p_memsz);
	ptr_src += sizeof(segment->p_memsz);
	return (ptr_src);
}

void		*add_padding_segments(t_elf *elf, void *ptr_src, void **ptr_dst, t_key *key)
{
	Elf64_Off		shoff;

	shoff = elf->header->e_shoff + PAGE_SIZE;
	for (int i = 0; i < elf->header->e_phnum; i++)
	{
		if (elf->segments[i].p_offset > (unsigned long)elf->pt_load->p_offset + elf->pt_load->p_filesz)
		{
			shoff = elf->segments[i].p_offset + PAGE_SIZE;
			ft_memcpy(*ptr_dst, ptr_src, (unsigned long)&elf->segments[i].p_offset - (unsigned long)ptr_src);
			*ptr_dst += (unsigned long)&elf->segments[i].p_offset - (unsigned long)ptr_src;
			ft_memcpy(*ptr_dst, &shoff, sizeof(shoff));
			*ptr_dst += sizeof(shoff);
			ptr_src = (void *)&elf->segments[i].p_offset + sizeof(elf->segments[i].p_offset);
		}
		else if ((unsigned long)&elf->segments[i] == (unsigned long)elf->pt_load)
			ptr_src = update_segment_sz(ptr_src, ptr_dst, elf->pt_load, key);
	}
	return (ptr_src);
}

void		*add_padding_sections(t_elf *elf, void *ptr_src, void **ptr_dst)
{
	Elf64_Off		shoff;

	for (int i = 0; i < elf->header->e_shnum; i++)
	{
		if ((unsigned long)elf->sections[i].sh_offset > (unsigned long)elf->pt_load->p_offset + elf->pt_load->p_filesz)
		{
			shoff = elf->sections[i].sh_offset + PAGE_SIZE;
			ft_memcpy(*ptr_dst, ptr_src, (unsigned long)&elf->sections[i].sh_offset - (unsigned long)ptr_src);
			*ptr_dst += (unsigned long)&elf->sections[i].sh_offset - (unsigned long)ptr_src;
			ft_memcpy(*ptr_dst, &shoff, sizeof(shoff));
			*ptr_dst += sizeof(shoff);
			ptr_src = (void *)&elf->sections[i].sh_offset + sizeof(elf->sections[i].sh_offset);
		}
	}
	return (ptr_src);
}
