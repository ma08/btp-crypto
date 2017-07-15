#a.out:	attack.c
	#g++ -maes -msse4 ../aes_imp/aes_intrinsic.c attack.c

attack256:	attack256.c
	g++ -maes -msse4 -o attack256 ../aes_imp/aes_intrinsic.c aes.c attack_helper.c attack256.c

attack:	attack256.c
	g++ -maes -msse4 -o attack ../aes_imp/aes_intrinsic.c aes.c attack_helper.c attack.c

attack6:	attack256_6round.c
	g++ -maes -msse4 -o attack6 ../aes_imp/aes_intrinsic.c aes.c attack_helper.c attack256_6round.c
