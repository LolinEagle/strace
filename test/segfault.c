int	main(void)
{
	volatile int	*p;

	p = (int *)0x0;
	*p = 42;
	return (0);
}
