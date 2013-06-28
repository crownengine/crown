import sys
import os
import socket

from gi.repository import Gtk

#------------------------------------------------------------------------------
class Console:

    def __init__(self):   
        builder = Gtk.Builder()
        builder.add_from_file("ui/console.glade")
        
        self.m_view = builder.get_object("textview1")
        self.m_buffer = builder.get_object("textbuffer1")
        self.m_entry = builder.get_object("entry1")
        
        self.m_window = builder.get_object('window1')
        self.m_window.set_title("Crown Console")      
        self.m_window.show_all()

        builder.connect_signals(self)

        #self.m_sock = socket.create_connection(('localhost', 10000))

        Gtk.main()
    
    def on_destroy(self, *args):
        #self.m_sock.close()
        Gtk.main_quit(*args)

    def on_key_pressed(self, entry, event):
        # If return is pressed, run command
        if event.keyval == 0xff0d:  
            self.run_command(entry.get_text())

    def run_command(self, cmd):
        # Send command to Crown
            #self.m_sock.send(cmd)
        # Print command to console
        end_iter = self.m_buffer.get_end_iter()
        a_string = "> " + cmd + "\n"
        self.m_buffer.insert(end_iter, a_string, len(a_string))
        self.m_entry.set_text("")
    

#------------------------------------------------------------------------------
def main():
   console = Console()

main()
    
