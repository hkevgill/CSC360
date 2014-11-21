.phony all:
all: diskinfo disklist

diskinfo: diskinfo.c
	gcc -Wall diskinfo.c -o diskinfo

disklist: disklist.c
	gcc -Wall disklist.c -o disklist

.PHONY clean:
clean:
	-rm -rf *.o *.exe
