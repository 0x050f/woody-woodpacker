#include "woody_woodpacker.h"

/**
* @brief Add the injection + params and key to the destination dst
*
* @param elf struct elf
* @param dst destination pointer
* @param new_entry new entry of the futur file
* @param key key generated
*/
void		add_injection(t_elf *elf, void **dst, Elf64_Addr new_entry, t_key *key)
{
	ft_memcpy(*dst, INJECT, INJECT_SIZE - (sizeof(uint64_t) * 7) + 5);
	*dst += INJECT_SIZE - ((sizeof(uint64_t) * 7) + 5);
	ft_memcpy(*dst + sizeof(uint64_t) * 0, &elf->pt_load->p_vaddr, sizeof(uint64_t));
	ft_memcpy(*dst + sizeof(uint64_t) * 1, &elf->pt_load->p_offset, sizeof(uint64_t));
	ft_memcpy(*dst + sizeof(uint64_t) * 2, &elf->text_section->sh_offset, sizeof(uint64_t));
	ft_memcpy(*dst + sizeof(uint64_t) * 3, &elf->text_section->sh_size, sizeof(uint64_t));
	ft_memcpy(*dst + sizeof(uint64_t) * 4, &new_entry, sizeof(uint64_t));
	ft_memcpy(*dst + sizeof(uint64_t) * 5, &elf->header->e_entry, sizeof(uint64_t)); // old_entry
	ft_memcpy(*dst + sizeof(uint64_t) * 6, &key->size, sizeof(uint64_t));
	*dst += sizeof(uint64_t) * 7;
	ft_memcpy(*dst, INJECT + (INJECT_SIZE - 5), 5);
	*dst += 5;
	ft_memcpy(*dst, key->str, key->size);
	*dst += key->size;
}

/**
* @brief fill the encrypted text content section to destination
*
* @param elf struct elf
* @param src source pointer
* @param dst destination pointer
* @param key key generated
*
* @return 0 or error code on problem
*/
int			fill_encrypted_text(t_elf *elf, void *src, void **dst, t_key *key)
{
	char			*encrypt;

	encrypt = xor_encrypt(elf->addr + elf->text_section->sh_offset, elf->text_section->sh_size, key);
	if (!encrypt)
		return (MALLOC_ERROR);
	ft_memcpy(*dst, src, ((unsigned long)elf->addr + (unsigned long)elf->text_section->sh_offset) - (unsigned long)src);
	*dst += ((unsigned long)elf->addr + (unsigned long)elf->text_section->sh_offset) - (unsigned long)src;
	ft_memcpy(*dst, encrypt, elf->text_section->sh_size);
	*dst += elf->text_section->sh_size;
	free(encrypt);
	return (0);
}

/**
* @brief fill the elf header and replace the old entry with the new one (injection)
*
* @param elf struct elf
* @param src source pointer
* @param dst destination pointer
* @param new_entry new entry of the futur file (injection)
*
* @return source pointer
*/
void		*fill_header(t_elf *elf, void *src, void **dst, Elf64_Addr new_entry)
{
	ft_memcpy(*dst, src, (unsigned long)&elf->header->e_entry - (unsigned long)src);
	*dst += (unsigned long)&elf->header->e_entry - (unsigned long)src;
	ft_memcpy(*dst, &new_entry, sizeof(new_entry));
	*dst += sizeof(new_entry);
	src = (void *)&elf->header->e_entry + sizeof(elf->header->e_entry);
	return (src);
}

/**
* @brief fill the destination with header, encrypted text section, injection,
* padding if needed, and previous source content
*
* @param elf struct elf
* @param key key generated
* @param dst destination pointer
* @param type add_padding or not
*
* @return 0 or error on problem
*/
int			fill_binary(t_elf *elf, t_key *key, void *dst, int type)
{
	int				ret;
	void			*src;
	void			*end;
	Elf64_Addr		new_entry;

	src = elf->addr;
	end = elf->addr + elf->size;
	new_entry = elf->pt_load->p_vaddr + elf->pt_load->p_memsz;
	src = fill_header(elf, src, &dst, new_entry);
	if (type == ADD_PADDING)
	{
		printf("LOL\n");
		src = add_padding_segments(elf, src, &dst, key);
	}
	else
		src = update_segment_sz(src, &dst, elf->pt_load, key);
	ret = fill_encrypted_text(elf, src, &dst, key);
	if (ret)
		return (ret);
	src = elf->addr + elf->text_section->sh_offset + elf->text_section->sh_size;
	ft_memcpy(dst, src, ((unsigned long)elf->addr + (unsigned long)elf->pt_load->p_offset + elf->pt_load->p_memsz) - (unsigned long)src);
	dst += ((unsigned long)elf->addr + (unsigned long)elf->pt_load->p_offset + elf->pt_load->p_memsz) - (unsigned long)src;
	src = elf->addr + elf->pt_load->p_offset + elf->pt_load->p_memsz;
	add_injection(elf, &dst, new_entry, key);
	if (type ==  ADD_PADDING)
		src = add_padding_sections(elf, src, &dst, key);
	else
		src += INJECT_SIZE + key->size;
	ft_memcpy(dst, src, end - src);
	return (0);
}
