echo "Compile $@ and then load to system.img"
for file in "$@"
do
    filename="${file%.*}"
    echo "Compile $file"
    bcc -ansi -c -o "$filename".o "$file" || exit 1

    echo "Link $filename.o and lib_asm.o"
    ld86 -o "$filename" -d "$filename".o lib_asm.o

    echo "Load $filename to system.img"
    ./loadFile $filename
done