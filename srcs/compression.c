#include "woody_woodpacker.h"

typedef struct	s_compressed_char
{
	unsigned char	c;
	int				bits;
	char			nb_bits;
}				t_compressed_char;

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

void	print_binary_tree(t_node	*x, int n)
{
	int i;

	i = n;
	if (x->left)
		print_binary_tree(x->left, n + 1);
	while (i--)
		printf("   ");
	printf("%d\n", x->freq);
	if (x->right)
		print_binary_tree(x->right, n + 1);
}

int			print_translation(t_node *x, char *string)
{
	int		ret;

	ret = 0;
	if (x->left)
		ret += print_translation(x->left, ft_strjoin(string, "0"));
	if (x->right)
		ret += print_translation(x->right, ft_strjoin(string, "1"));
	if (!x->right && !x->left)
	{
		printf("char ");
		if (x->c > 31 && x->c < 127)
			printf("'%c'", x->c);
		else
			printf("%d", x->c);
		printf(": %d occurence - ", x->freq);
		printf("%s\n", string);
		ret = x->freq * ft_strlen(string);
	}
	free(string);
	return (ret);
}

void		make_array_compressed_char(t_compressed_char **ptr, t_node *x, int bits, int nb_bits)
{
	if (x->left)
		make_array_compressed_char(ptr, x->left, bits, nb_bits + 1);
	if (x->right)
	{
		bits = (bits & ~(1 << (nb_bits + 1))) | (1 << (nb_bits + 1));
		make_array_compressed_char(ptr, x->right, bits, nb_bits + 1);
	}
	if (!x->right && !x->left)
	{
		(*ptr)->c = x->c;
		(*ptr)->bits = bits;
		(*ptr)->nb_bits = nb_bits;
		(*ptr)++;
	}
}

void		create_huffman_tree(int count[256])
{
	int			i;
	t_nodelst	*nodes;
	t_nodelst	*tmp_nodes;
//	t_nodelst	*ptr;

	nodes = NULL;
	i = 0;
	while (i < 256)
	{
		if (count[i] != 0)
			add_nodelst(&nodes, new_node(i, count[i]));
		i++;
	}
	i = 0;
	tmp_nodes = nodes;
	while (tmp_nodes->next && ++i)
		tmp_nodes = tmp_nodes->next;
	/*
	ptr = nodes;
	while (ptr)
	{
		printf("char ");
		if (ptr->node->c > 31 && ptr->node->c < 127)
			printf("'%c'", ptr->node->c);
		else
			printf("%d", ptr->node->c);
		printf(": %d occurence\n", ptr->node->freq);
		ptr = ptr->next;
	}
	*/
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
//	print_binary_tree(nodes->node, 0);
//	int ret;
	t_compressed_char	*compressed_chars;
	t_compressed_char	*ptr;
	
	compressed_chars = malloc(sizeof(t_compressed_char) * i);
	if (!compressed_chars)
		return ; // TODO: error
	ptr = compressed_chars;
	make_array_compressed_char(&ptr, nodes->node, 0, 0);
	int n;
	n = 0;
	while (n < i)
	{
		if (compressed_chars[n].c > 31 && compressed_chars[n].c < 127)
			printf("'%c'", compressed_chars[n].c);
		else
			printf("%d", compressed_chars[n].c);
		printf(": %d - %d\n", compressed_chars[n].bits, compressed_chars[n].nb_bits);;
		n++;
	}
//	ret = print_translation(nodes->node, ft_strjoin("", ""));
//	printf("=== AFTER COMPRESSION\n");
//	printf("%d bits\n", ret);
//	printf("%d bytes\n", ret / 8);
}

void		compress(unsigned char *addr, size_t size)
{
	size_t	i;
	int		count[256];

	i = 0;
	while (i < 256)
		count[i++] = 0;
	i = 0;
	while (i < size)
		count[(int)addr[i++]]++;
	create_huffman_tree(count);
	printf("=== BASE\n");
	printf("%ld bits\n", size * 8);
	printf("%ld bytes\n", size);
}
