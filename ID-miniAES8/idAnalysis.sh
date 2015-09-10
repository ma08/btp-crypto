cc ./idAttack.c ./miniAES.c
powlimit=20
rm output.txt
for (( c=20; c<=27; c++ ))
do
	./a.out $c >> output.txt
done


