#!/bin/bash
#
#
# generate *.html.<lang> from input templates *.html.<lang>.in

# list of available languages
LANGUAGES=""
for f in $1.html.*.in; do
  LANGUAGES="$LANGUAGES $(echo $f |cut -d . -f 3)"
done

# build the html code to be added
CODE=""
for l in $LANGUAGES; do
  CODE=$CODE"<a href=\"$1.html.${l}\"><img src=\"images/${l}.png\"></a> "
done

# now generate the files
#for l in $LANGUAGES; do
  sed -e "s;<!--LANG-->;$CODE;g" $1.html.$2.in >$1.html.$2
#done
