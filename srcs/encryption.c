#include "woody_woodpacker.h"

/**
* @brief Generate a key of randoms 'size' chars
*
* @param size
*
* @return string generated
*/
char		*generate_key(size_t size)
{
	size_t		i;
	struct		timespec spec;
	char		*key;

	key = malloc(size);
	if (!key)
		return (NULL); //TODO: malloc error
	syscall(228, CLOCK_REALTIME, &spec); // clock_gettime syscall
	ft_srand(spec.tv_nsec);
	for (i = 0; i < size; i++)
		key[i] = ft_rand() % 256;
	return (key);
}

/**
* @brief Encrypt the input with the key generated. Every char is xor with the j
* char on key
*
* @param input input to encrypt
* @param input_len length of input
* @param key key generated
*
* @return string encrypted
*/
char		*xor_encrypt(char *input, size_t input_len, t_key *key)
{
	size_t		i;
	size_t		j;
	char		*encrypt;

	if (!(encrypt = malloc(sizeof(char) * input_len)))
		return (NULL);
	j = 0;
	for (i = 0; i < input_len; i++)
	{
		encrypt[i] = input[i] ^ key->str[j];
		j++;
		if (j == key->size)
			j = 0;
	}
	return (encrypt);
}
