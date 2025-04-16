#!/bin/bash

domain_opt="imgview"
potfile_opt="po/$domain_opt.pot"

error_exit()
{
    msg="$1"
    test "$msg" != "" || msg="an error occurred"
    printf "*** $msg\nabort...\n"
    exit 1
}

echo "*** creating pot file"
rm -f $potfile_opt
xgettext \
	--copyright-holder="2025 hotnuma" \
	--package-name="Hello" \
	--package-version="1.0" \
	--msgid-bugs-address="hotnuma@gmail.com" \
	--no-wrap \
	--from-code=UTF-8 \
	-k_ -kN_ \
	-s -d $domain_opt -f "po/POTFILES.in" -o "$potfile_opt"

test $? -eq 0 || error_exit "xgettext failed"

# create languages
linguas="po/LINGUAS"
while read -r line; do
    [[ "$line" =~ ^#.*$ ]] && continue
    pofile="po/$line.po"
    if [ ! -e "$pofile" ]; then
        echo "*** creating $pofile"
        msginit --no-translator --no-wrap --locale="$lang.UTF-8" \
        --input="$potfile_opt" --output-file="$pofile"
    fi
done < "$linguas"

# update languages
for file in po/*.po
do
    echo "*** updating $file from $potfile_opt"
	msgmerge --sort-output --no-wrap --update --backup=off $file $potfile_opt
done


