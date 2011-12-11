#!/bin/bash
#
#
# generate index.html.<lang> from input templates index.html.<lang>.in

# list of available languages
LANGUAGES=""
for f in index.html.*.in; do
  LANGUAGES="$LANGUAGES ${f:11:2}"
done

# build the html code to be added
CODE=""
for l in $LANGUAGES; do
  CODE=$CODE"<a href=\"index.html.$l\"><img src=\"images/$l.png\"></a> "
done

# now generate the files
for l in $LANGUAGES; do
  sed -e "s;<!--LANG-->;$CODE;g" index.html.$l.in >index.html.$l
done
