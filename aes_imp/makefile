run:	a.out
	./a.out

a.out:	main.c aes_intrinsic.c
	g++ -maes -msse4 aes_intrinsic.c main.c
