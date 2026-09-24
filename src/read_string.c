#include "strace.h"

char	*read_string(
	pid_t child_pid, unsigned long addr, char *dest_buf, size_t max_len)
{
	struct iovec	local;
	struct iovec	remote;
	ssize_t			nread;

	if (addr == 0)
		return (NULL);

	local = (struct iovec){
		.iov_base = dest_buf,
		.iov_len = max_len - 1
	};
	remote = (struct iovec){
		.iov_base = (void *)addr,
		.iov_len = max_len - 1
	};

	nread = process_vm_readv(child_pid, &local, 1, &remote, 1, 0);
	if (nread <= 0)
		return (NULL);

	dest_buf[nread] = '\0';
	return (dest_buf);
}

void	print_escaped_string(FILE *stream, const char *str, size_t count)
{
	static size_t	count_max = 34;
	size_t			count_copy;
	size_t			i;
	unsigned char	c;

	if (!str)
		return ;

	fputs("\"", stream);
	if (count > count_max)
		count_copy = count_max;
	else
		count_copy = count;
	i = 0;
	while ((count == ULONG_MAX && str[i] != '\0')
		|| (count != ULONG_MAX && count_copy-- > 0))
	{
		c = (unsigned char)str[i];
		if (c == '\n')
			fputs("\\n", stream);
		else if (c == '\t')
			fputs("\\t", stream);
		else if (c == '\r')
			fputs("\\r", stream);
		else if (c == '\v')
			fputs("\\v", stream);
		else if (c == '\f')
			fputs("\\f", stream);
		else if (c == '\a')
			fputs("\\a", stream);
		else if (c == '\b')
			fputs("\\b", stream);
		else if (c == '\\')
			fputs("\\\\", stream);
		else if (c == '\"')
			fputs("\\\"", stream);
		else if (c == '\0')
			fputs("\\0", stream);
		else
		{
			if (isprint(c))
				fputc(c, stream);
			else
				fprintf(stream, "\\x%02x", c);
		}
		i++;
	}
	if (count > count_max && count != ULONG_MAX)
		fputs("\"...", stream);
	else
		fputs("\"", stream);
}

void	print_syscall_entry_string(pid_t child_pid, unsigned long args,
	size_t count)
{
	char	str_buf[256];
	char	*res;

	res = read_string(child_pid, args, str_buf, sizeof(str_buf));
	if (args == 0)
		fprintf(stderr, "NULL");
	else if (res != NULL)
		print_escaped_string(stderr, res, count);
	else
		fprintf(stderr, "0x%lx", (unsigned long)args);
}

void	print_syscall_entry_argv(t_tracer *t)
{
	int	i;

	fprintf(stderr, "[");
	i = 1;
	while (1)
	{
		fprintf(stderr, "\"%s\"", t->argv[i]);
		if (++i >= t->argc)
			break ;
		fprintf(stderr, ", ");
	}
	fprintf(stderr, "]");
}
