#include "woody_woodpacker.h"

typedef struct	s_node
{
	unsigned char	c;
	unsigned int	freq;
	void			*right;
	void			*left;
}				t_node;

typedef struct	s_nodelst
{
	t_node		*node;
	void		*next;
}				t_nodelst;

t_node		*new_node(unsigned char c, unsigned int freq)
{
	t_node	*node;

	node = malloc(sizeof(t_node));
	if (!node)
		return (NULL);
	node->c = c;
	node->freq = freq;
	node->left = NULL;
	node->right = NULL;
	return (node);
}

t_nodelst	*new_nodelst(t_node *node)
{
	t_nodelst	*lst;

	lst = malloc(sizeof(t_nodelst));
	if (!lst)
		return (NULL);
	lst->node = node;
	lst->next = NULL;
	return (lst);
}

void		add_nodelst(t_nodelst **nodes, t_node *node) // add croissant
{
	t_nodelst	*prev;
	t_nodelst	*ptr;
	t_nodelst	*new;

	new = new_nodelst(node);
	ptr = *nodes;
	prev = NULL;
	while (ptr && ptr->node->freq < node->freq)
	{
		prev = ptr;
		ptr = ptr->next;
	}
	if (prev)
		prev->next = new;
	else
		*nodes = new;
	new->next = ptr;
}

void		remove_nodelst(t_nodelst **nodes, t_node *node)
{
	t_nodelst	*ptr;
	t_nodelst	*prev;

	ptr = *nodes;
	prev = NULL;
	while (ptr && ptr->node != node)
	{
		prev = ptr;
		ptr = ptr->next;
	}
	if (ptr)
	{
		if (prev)
			prev->next = ptr->next;
		else if (ptr->next)
			*nodes = ptr->next;
		else
			*nodes = NULL;
		free(ptr);
	}
}

t_node		*make_tree(t_nodelst *nodes)
{
	t_node	*left;
	t_node	*right;
	t_node	*tmp;

	while (nodes->next)
	{
		left = nodes->node;
		right = ((t_nodelst *)nodes->next)->node;
		remove_nodelst(&nodes, left);
		remove_nodelst(&nodes, right);
		tmp = new_node(0, left->freq + right->freq);
		tmp->left = left;
		tmp->right = right;
		add_nodelst(&nodes, tmp);
	}
	tmp = nodes->node;
	free(nodes);
	return (tmp);
}

t_compressed_char		*find_compressed_char(unsigned char to_find, t_compressed_char *compress, int size)
{
	int		i;

	i = 0;
	while (i < size)
	{
		if (compress[i].c == to_find)
			return (&compress[i]);
		i++;
	}
	return (NULL);
}

void		print_binary(t_compressed_char *compress)
{
	int		i;

	i = 0;
	while (i < compress->nb_bits)
	{
		if (compress->bits & (1 << (8 * 4 - i)))
			printf("1");
		else printf("0");
		i++;
	}
}

// RETURN TOTAL_SIZE
int			make_array_compressed_char(t_compressed_char **ptr, t_node *x, int bits, int nb_bits)
{
	int		ret;

	ret = 0;
	if (x->left)
		ret += make_array_compressed_char(ptr, x->left, bits, nb_bits + 1);
	if (x->right)
	{
		bits = (bits | (1 << (8 * 4 - nb_bits)));
		ret += make_array_compressed_char(ptr, x->right, bits, nb_bits + 1);
	}
	if (!x->right && !x->left)
	{
		ret = nb_bits * x->freq;
		(*ptr)->c = x->c;
		(*ptr)->bits = bits;
		(*ptr)->nb_bits = nb_bits;
		(*ptr)++;
	}
	return (ret);
}

t_node		*create_huffman_tree(int count[256])
{
	int			i;
	t_nodelst	*nodes;

	nodes = NULL;
	i = 0;
	while (i < 256)
	{
		if (count[i] != 0)
			add_nodelst(&nodes, new_node(i, count[i]));
		i++;
	}
	return (make_tree(nodes));
}

t_compression		*compress(unsigned char *addr, int size)
{
	int				i;
	int				n;
	int				count[256];
	t_node			*root;
	t_compression	*compression;

	i = 0;
	while (i < 256)
		count[i++] = 0;
	i = 0;
	while (i < size)
		count[(int)addr[i++]]++;
	n = 0;
	i = 0;
	while (i < 256)
	{
		if (count[i++])
			n++;
	}
	root = create_huffman_tree(count);
	t_compressed_char	*compressed_chars;
	t_compressed_char	*ptr;
	
	compressed_chars = malloc(sizeof(t_compressed_char) * (n + 1));
	if (!compressed_chars)
		return (NULL); // TODO: error
	ptr = compressed_chars;
	compression = malloc(sizeof(t_compression));
	if (!compression)
		return (NULL);
	compression->nb_bits = make_array_compressed_char(&ptr, root, 0, 0);
	compression->result = malloc(compression->nb_bits / 8 + (compression->nb_bits % 8 != 0));
	if (!compression->compress)
		return (NULL); //TODO: error
	compression->table = compressed_chars;
	i = 0;
	while (i < n)
	{
		if (compressed_chars[i].c > 31 && compressed_chars[i].c < 127)
			printf("'%c'", compressed_chars[i].c);
		else
			printf("%d", compressed_chars[i].c);
		printf(": ");
		print_binary(&compressed_chars[i]);
		printf("\n");
		i++;
	}
	i = 0;
	while (i < size)
	{
		ptr = find_compressed_char(addr[i++], compressed_chars, n);
	}
	compression->result
	printf("new_size: %d bits - %d bytes\n", compression->nb_bits, compression->nb_bits / 8);
	printf("=== BASE\n");
	printf("%d bits\n", size * 8);
	printf("%d bytes\n", size);
	return (compression);
}
