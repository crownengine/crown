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
import subprocess

from gi.repository import Gtk

from pycrown import Repository

class Toolchain:
	def __init__(self):

		self.m_current_platform = "linux"

		builder = Gtk.Builder()
		builder.add_from_file("ui/toolchain.glade")

		builder.connect_signals(self)

		self.m_run_button = builder.get_object("run_button")
		self.m_project_chooser = builder.get_object("project_chooser")

		self.m_window = builder.get_object("window1")
		self.m_window.set_title("Crown Toolchain")

		self.m_window.show_all()

		Gtk.main()

	# Callback
	def on_delete(self, *args):
		Gtk.main_quit(*args)

	def on_project_chooser_file_set(self, button):
		self.m_run_button.set_sensitive(True)
		self.m_root_path = button.get_filename()

	def on_platform_combobox_changed(self, combo):
		tree_iter = combo.get_active_iter()
		if tree_iter != None:
			model = combo.get_model()
			platform, name = model[tree_iter][:2]
			# Sets the current platform name
			self.m_current_platform = platform

	def on_target_combobox_changed(self, combo):
		print("Not implemented.")


	def on_run_button_clicked(self, button):
		root_path = str(self.m_root_path)
		dest_path = root_path + "_" + self.m_current_platform

		comp = subprocess.Popen(["python", "resource-compiler.py", str(self.m_root_path), self.m_current_platform])
		crown = subprocess.Popen(["../bin/crown-linux", "--root-path", dest_path, "--dev"])

	def on_browser_button_clicked(self, button):
		browser = subprocess.Popen(["python", "resource-browser.py", str(self.m_root_path)])

	def on_console_button_clicked(self, button):
		console = subprocess.Popen(["python", "console.py", "localhost"])


#------------------------------------------------------------------------------
def main():
	toolchain = Toolchain()

main()

