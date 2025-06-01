SRCS = main_local.c rendering.c updater.c

main: $(SRCS)
	clang -o main $(SRCS)
