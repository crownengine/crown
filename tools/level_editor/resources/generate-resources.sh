echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
echo "<gresources>"
echo "  <gresource prefix=\"/org/crown/level_editor\">"
echo "    <file compressed=\"true\">theme/Adwaita/gtk-contained-dark.css</file>"
echo "    <file compressed=\"true\">theme/Adwaita/gtk-dark.css</file>"

find * -name *.ui -exec \
echo "    <file compressed=\"true\" preprocess=\"xml-stripblanks\">{}</file>" \;

find icons theme/Adwaita/assets -name *.png -exec \
echo "    <file>{}</file>" \;

echo "  </gresource>"
echo "</gresources>"
