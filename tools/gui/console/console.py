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
import socket
import threading

from gi.repository import Gtk

# Client Console commands
CMD_CLEAR   = "clear"   # Clear console output
CMD_EXIT    = "exit"    # Close console
CMD_HELP    = "help"    # Console help
CMD_VOID    = ""        


# class ReaderThread(threading.Thread):
# #------------------------------------------------------------------------------
#     def __init__(self, socket, error_buffer):
#         threading.Thread.__init__(self)

#         self.t_socket = socket
#         self.t_error = error_buffer
#         self.t_tmp = bytearray()

# #------------------------------------------------------------------------------
#     def run(self):
#         while True:
#             self.t_tmp = self.t_socket.recv(1024)
#             self.t_error = self.t_tmp.decode('utf-8', 'ignore')
#             print(self.t_error)

class ConsoleHistory:
#------------------------------------------------------------------------------
    def __init__(self):
        self.m_list = list()

        self.m_count = 0
        self.m_index = 0

#------------------------------------------------------------------------------
    def add(self, cmd):
        self.m_list.append(cmd)
        self.m_count += 1
        self.m_index = self.m_count

#------------------------------------------------------------------------------
    def previous(self):
        if self.m_count != 0:
            self.m_index -= 1

            if self.m_index < 0:
                self.m_index = 0

            return self.m_list[self.m_index]

        return ""

#------------------------------------------------------------------------------
    def following(self):
        if self.m_count != 0:
            self.m_index += 1

            if self.m_index > self.m_count-1:
                self.m_index = self.m_count - 1

            return self.m_list[self.m_index]

        return ""


class Console:
#------------------------------------------------------------------------------
    def __init__(self, address):   
        builder = Gtk.Builder()
        builder.add_from_file("ui/console.glade")
        
        self.m_view = builder.get_object("textview1")
        self.m_buffer = builder.get_object("textbuffer1")
        self.m_entry = builder.get_object("entry1")
        
        self.m_window = builder.get_object('window1')
        self.m_window.set_title("Crown Console")      
        self.m_window.show_all()

        builder.connect_signals(self)

        self.m_address = address
        self.m_error_buffer = ""

        self.history = ConsoleHistory()

        self.m_sock = socket.create_connection((self.m_address, 10000))

        #self.m_thread = ReaderThread(self.m_sock, self.m_error_buffer)
        #self.m_lock = threading.Lock()

        #self.m_thread.start()

        Gtk.main()

#------------------------------------------------------------------------------
    def on_destroy(self, *args):
        self.m_sock.close()
        Gtk.main_quit(*args)

#------------------------------------------------------------------------------
    def on_key_pressed(self, entry, event):
        # If return is pressed, run command
        if event.keyval == 0xff0d:
            cmd = entry.get_text()
            self.parse_command(cmd)
            return True

        if event.keyval == 0xff52:
            cmd = self.history.previous()
            self.print_to_entry(cmd)
            return True

        if event.keyval == 0xff54:
            cmd = self.history.following()
            self.print_to_entry(cmd)
            return True


#------------------------------------------------------------------------------
    def parse_command(self, cmd):
        self.history.add(cmd)

        if cmd == CMD_CLEAR:
            self.m_buffer.set_text("")
            self.m_entry.set_text("")

        elif cmd == CMD_EXIT:
            self.on_destroy()  

        elif cmd == CMD_HELP:
            self.print_help()

        elif cmd == CMD_VOID:
            self.print_to_console("");

        else:    
            self.run_command(cmd)        

#------------------------------------------------------------------------------
    def run_command(self, cmd):
        self.m_sock.send(cmd.encode('utf-8'))
        self.print_to_console(cmd)

#------------------------------------------------------------------------------
    def print_to_console(self, text):
        # Print command to console
        end_iter = self.m_buffer.get_end_iter()
        a_string = "> " + text + "\n"
        # Append command to the end of buffer
        self.m_buffer.insert(end_iter, a_string, len(a_string))
        # Reset entry
        self.print_to_entry("")

#------------------------------------------------------------------------------
    def print_to_entry(self, text):
        self.m_entry.set_text(text)


#------------------------------------------------------------------------------
    # def popup_dialog(self, message, expl):
    #     dialog = Gtk.MessageDialog(self.m_window, 0, Gtk.MessageType.INFO, Gtk.ButtonsType.OK, message)
    #     dialog.format_secondary_text(expl)
    #     dialog.run()
    #     dialog.destroy()

#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
def main():
    if len(sys.argv) != 2:
        print("Usage: console.py <ip-address>")
        exit(-1)

    address = sys.argv[1]

    console = Console(address)


main()
    
