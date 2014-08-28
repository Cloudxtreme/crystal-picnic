# use inside anim set directory, recurively flips collision bounds horizontally
# if image was 64 pixels wide

for f in `find . -name "*.xml"` ; do cat $f | sed -e 's#\(.*\)<x>\(.*\)</x>\(.*\)#\1<x>$((64-\2))</x>\3#' > __foo__ ; cat __foo__ | sed -e 's/\(.*\)/echo "\1"/' > __foo2__ ; sh __foo2__ > $f ; rm __foo__ __foo2__ ; done

