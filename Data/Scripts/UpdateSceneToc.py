import xml.etree.ElementTree as ET

to_replace = "pattern_to_replace"
replace_by = "new_pattern"

scenetoc = "E:\\Projects\\Softimage\\Procedural\\Scenes\\AudioTest\\AudioTest.scntoc"

# load scenetoc
tree = ET.parse(toc)
root = tree.getroot()

# find audio sources and update content
for src in root.findall('Sources'):
	for audio in src.findall('Audio'):
		old_value = audio.text
		new_value = old_value.replace(to_replace, replace_by)
		audio.text = new_value

# save scene toc 
tree.write(scenetoc)