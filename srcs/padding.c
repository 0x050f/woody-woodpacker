#include "woody_woodpacker.h"

/**
* @brief update p_filesz and p_memsz of the segment pt_load where will be the
* injection

* @param src pointer of the source file
* @param dst pointer to the allocated destination
* @param segment pt_load where goes the injection
* @param key key generated to add the size of it
*
* @return pointer to src
*/
void		*update_segment_sz(void *src, void **dst, Elf64_Phdr *segment, t_key *key)
{
	uint64_t	p_filesz;
	uint64_t	p_memsz;

	ft_memcpy(*dst, src, (unsigned long)&segment->p_filesz - (unsigned long)src);
	*dst += (unsigned long)&segment->p_filesz - (unsigned long)src;
	src = &segment->p_filesz;
	p_filesz = segment->p_filesz + INJECT_SIZE + key->size;
	ft_memcpy(*dst, &p_filesz, sizeof(segment->p_filesz));
	*dst += sizeof(segment->p_filesz);
	src += sizeof(segment->p_filesz);
	p_memsz = segment->p_memsz + INJECT_SIZE + key->size;
	ft_memcpy(*dst, &p_memsz, sizeof(segment->p_memsz));
	*dst += sizeof(segment->p_memsz);
	src += sizeof(segment->p_memsz);
	return (src);
}

/**
* @brief add the segment padding (basically PAGE_SIZE) of every segments after
* injection
*
* @param elf struct of pointer of the elf file
* @param src pointer to the source file
* @param dst pointer to the allocated destination
* @param key key generated
*
* @return pointer to source file
*/
void		*add_padding_segments(t_elf *elf, void *src, void **dst, t_key *key)
{
	Elf64_Off		shoff;

	shoff = elf->header->e_shoff + PAGE_SIZE;
	ft_memcpy(*dst, src, (unsigned long)&elf->header->e_shoff - (unsigned long)src);
	*dst += (unsigned long)&elf->header->e_shoff - (unsigned long)src;
	ft_memcpy(*dst, &shoff, sizeof(shoff));
	*dst += sizeof(shoff);
	src = (void *)&elf->header->e_shoff + sizeof(elf->header->e_shoff);
	shoff = elf->header->e_shoff + PAGE_SIZE;
	for (int i = 0; i < elf->header->e_phnum; i++)
	{
		if ((unsigned long)&elf->segments[i] == (unsigned long)elf->pt_load)
			src = update_segment_sz(src, dst, elf->pt_load, key);
		else if (elf->segments[i].p_offset >= (unsigned long)elf->pt_load->p_offset + elf->pt_load->p_filesz)
		{
			shoff = elf->segments[i].p_offset + PAGE_SIZE;
			ft_memcpy(*dst, src, (unsigned long)&elf->segments[i].p_offset - (unsigned long)src);
			*dst += (unsigned long)&elf->segments[i].p_offset - (unsigned long)src;
			ft_memcpy(*dst, &shoff, sizeof(shoff));
			*dst += sizeof(shoff);
			src = (void *)&elf->segments[i].p_offset + sizeof(elf->segments[i].p_offset);
		}
	}
	return (src);
}

/**
* @briefadd the section padding (basically PAGE_SIZE) of every sections after
* injection
*
* @param elf struct of pointer of the elf file
* @param src pointer to the source file
* @param dst pointer to the allocated destination
* @param key key generated
*
* @return pointer to source file
*/
void		*add_padding_sections(t_elf *elf, void *src, void **dst, t_key *key)
{
	int				diff;
	Elf64_Phdr		*segments;
	Elf64_Off		shoff;

	segments = elf->pt_load + 1;
	diff = (INJECT_SIZE + key->size) - (segments->p_offset - (elf->pt_load->p_offset + elf->pt_load->p_filesz));
	ft_memset(*dst, 0, PAGE_SIZE - (diff % PAGE_SIZE));
	*dst += PAGE_SIZE - (diff % PAGE_SIZE);
	src = elf->addr + segments->p_offset;
	for (int i = 0; i < elf->header->e_shnum; i++)
	{
		if ((unsigned long)elf->sections[i].sh_offset > (unsigned long)elf->pt_load->p_offset + elf->pt_load->p_filesz)
		{
			shoff = elf->sections[i].sh_offset + PAGE_SIZE;
			ft_memcpy(*dst, src, (unsigned long)&elf->sections[i].sh_offset - (unsigned long)src);
			*dst += (unsigned long)&elf->sections[i].sh_offset - (unsigned long)src;
			ft_memcpy(*dst, &shoff, sizeof(shoff));
			*dst += sizeof(shoff);
			src = (void *)&elf->sections[i].sh_offset + sizeof(elf->sections[i].sh_offset);
		}
	}
	return (src);
}
