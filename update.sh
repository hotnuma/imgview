#!/bin/bash

domain="imgview"
potfile="po/$domain.pot"

echo "*** creating pot file"
rm -f $potfile
xgettext \
	--copyright-holder="2025 hotnuma" \
	--package-name="Hello" \
	--package-version="1.0" \
	--msgid-bugs-address="hotnuma@gmail.com" \
	--no-wrap \
	--keyword=_ \
	--from-code=UTF-8 \
	-s -d $domain -f "po/POTFILES.in" -o "$potfile"

# create languages
linguas="po/LINGUAS"
while read -r line; do
    [[ "$line" =~ ^#.*$ ]] && continue
    pofile="po/$line.po"
    if [ ! -e "$pofile" ]; then
        echo "*** creating $pofile"
        msginit --no-translator --no-wrap --locale="$lang.UTF-8" \
        --input="$potfile" --output-file="$pofile"
    fi
done < "$linguas"

# update languages
for file in po/*.po
do
    echo "*** updating $file from $potfile"
	msgmerge --sort-output --no-wrap --update --backup=off $file $potfile
done


