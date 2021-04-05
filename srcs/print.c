#include "woody_woodpacker.h"

/**
* @brief print hexadecimal
*
* @param dec
*/
void	dec_to_hex(char dec)
{
	int				i;
	int				remainder;
	unsigned char	result[2];
	unsigned char	copy;
	char			*hex;

	hex = "0123456789abcdef";
	copy = dec;
	if (dec < 0)
		copy += 256;
	i = 0;
	while (i < 2)
	{
		remainder = copy % 16;
		result[i] = hex[remainder];
		copy /= 16;
		i++;
	}
	i = 1;
	while (i >= 0)
		write(STDOUT_FILENO, &result[i--], 1);
}

/**
* @brief Print a line for ft_print_memory
* 16 character as hexadecimal, space every 2 character then print
* ascii for those char, if they are unprintable, print '.'
*
* @param str
* @param n
*/
void	print_line(char *str, int n)
{
	int temp;
	int i;

	if (n == 0)
		n = 16;
	i = 0;
	temp = 0;
	while (temp < n)
	{
		if (!(temp % 2) && temp != 0)
			write(STDOUT_FILENO, " ", 1);
		dec_to_hex(str[temp++]);
	}
	while (i++ < ((16 - n) * 2) + (16 - n) / 2)
		write(STDOUT_FILENO, " ", 1);
	write(STDOUT_FILENO, " ", 1);
	temp = 0;
	while (temp < n)
	{
		if (str[temp] < 32 || str[temp] > 126)
			write(STDOUT_FILENO, ".", 1);
		else
			write(STDOUT_FILENO, &str[temp], 1);
		temp++;
	}
}

/**
* @brief Print memory pointed by addr of size 'size'
*
* @param addr
* @param size
*
* @return addr
*/
void	*ft_print_memory(void *addr, unsigned int size)
{
	int					i;
	unsigned int		temp;
	char				*str;
	char				copy[16];

	str = addr;
	i = 0;
	temp = 0;
	while (temp < size && size != 0)
	{
		copy[temp % 16] = str[temp];
		if ((temp + 1) % 16 == 0 || temp == size - 1)
		{
			print_line(copy, temp % 16 + 1);
			i += 16;
			write(STDOUT_FILENO, "\n", 1);
		}
		temp++;
	}
	return (addr);
}

/**
* @brief Print the key in hexadecimal with '\x' before every char
*
* @param key generated key
* @param size size of key
*/
void	print_hexa_key(char *key, size_t size)
{
	size_t			i;

	i = 0;
	while (i < size)
	{
		write(STDOUT_FILENO, "\\x", 2);
		dec_to_hex(key[i++]);
	}
}
