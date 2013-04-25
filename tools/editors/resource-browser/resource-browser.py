#!/usr/bin/python

# Copyright (c) 2013 Daniele Bartolini, Michele Rossi
# Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use,
# copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following
# conditions:
# 
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
# OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

import sys
import os
from gi.repository import Gtk

from crown.resources import Repository

class ResourceBrowser:
	def __init__(self, repository):
		repository.scan()

		builder = Gtk.Builder()
		builder.add_from_file("glade/resource-browser.glade")

		self.m_filter_entry = builder.get_object("entry1")
		self.m_list_store = builder.get_object("liststore1")

		self.m_list_view = builder.get_object("treeview1")

		self.m_window = builder.get_object("window1")
		self.m_window.set_title(repository.root_path())

		renderer = Gtk.CellRendererText()
		column = Gtk.TreeViewColumn("Name", renderer, text=0)
		self.m_list_view.append_column(column)

		# Populate list model
		for res in repository.all_resources():
			self.m_list_store.append([res])

		self.m_list_filter = self.m_list_store.filter_new()
		self.m_list_filter.set_visible_func(self.visible_func)

		self.m_list_view.set_model(self.m_list_filter)

		builder.connect_signals(self)

		self.m_window.show_all()

		Gtk.main()

	# Callback
	def on_delete(self, *args):
		Gtk.main_quit(*args)

	# We call refilter whenever the user types into the filter entry
	def on_filter_entry_text_changed(self, entry):
		self.m_list_filter.refilter()

	# The callback used to filter resources in the list view
	def visible_func(self, model, iter, user_data):
		name = str(model.get_value(iter, 0))

		# Strip leading and trailing spaces
		search_text = self.m_filter_entry.get_text().strip()

		if (search_text == ""):
			return True

		if (name.find(search_text) == -1):
			return False

		return True

#------------------------------------------------------------------------------
def main():
	root_path = ""

	if (len(sys.argv) != 2):
		print("Usage: resource-browser <root-path>")
		sys.exit(-1)

	root_path = sys.argv[1];

	root_path = os.path.abspath(root_path)

	if not os.path.exists(root_path):
		print("The path does not exist.")
		sys.exit(-1)

	if (os.path.islink(root_path)):
		print("The path is a symbolic link.")
		sys.exit(-1)

	if not os.path.isdir(root_path):
		print("The path has to be a directory.")
		sys.exit(-1)

	repository = Repository.Repository(root_path)

	browser = ResourceBrowser(repository)

main()

