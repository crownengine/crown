import sys
import os
import socket

from gi.repository import Gtk

# Client Console commands
CMD_CLEAR   = "clear"   # Clear console output
CMD_EXIT    = "exit"    # Close console
CMD_HELP    = "help"    # Console help

# Server Console commands
CMD_STOP    = "device stop" # Stop Engine and close console

# Help message
MSG_HELP    =   "1- clear - clear screen output\n2- exit  - terminate console\n3- device stop - terminate engine and console\n4- help - print this message\n"

class Console:
#------------------------------------------------------------------------------
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

        self.m_sock = socket.create_connection(('localhost', 10000))

        Gtk.main()

#------------------------------------------------------------------------------
    def on_destroy(self, *args):
        self.m_sock.shutdown()
        Gtk.main_quit(*args)

#------------------------------------------------------------------------------
    def on_key_pressed(self, entry, event):
        # If return is pressed, run command
        if event.keyval == 0xff0d :
            cmd = entry.get_text()
            self.parse_command(cmd)

#------------------------------------------------------------------------------
    def parse_command(self, cmd):
        if cmd == CMD_CLEAR:
            self.m_buffer.set_text("")
            self.m_entry.set_text("")

        elif cmd == CMD_EXIT:
            self.on_destroy()

        elif cmd == CMD_STOP:
            self.run_command(cmd)
            self.popup_dialog("Crown has stopped!", "Console connection will be closed")
            self.on_destroy()

        elif cmd == CMD_HELP:
            self.print_help()

        else:    
            self.run_command(cmd)        

#------------------------------------------------------------------------------
    def run_command(self, cmd):
        # Send command to Crown
        self.m_sock.send(cmd.encode())
        self.print_command(cmd)

#------------------------------------------------------------------------------
    def print_command(self, cmd):
        # Print command to console
        end_iter = self.m_buffer.get_end_iter()
        a_string = "> " + cmd + "\n"
        # Append command to the end of buffer
        self.m_buffer.insert(end_iter, a_string, len(a_string))
        # Reset entry
        self.m_entry.set_text("")


#------------------------------------------------------------------------------
    def print_help(self):
        end_iter = self.m_buffer.get_end_iter()
        a_string = MSG_HELP + "\n"
        # Append command to the end of buffer
        self.m_buffer.insert(end_iter, a_string, len(a_string))
        # Reset entry
        self.m_entry.set_text("")

#------------------------------------------------------------------------------
    def popup_dialog(self, message, expl):
        dialog = Gtk.MessageDialog(self.m_window, 0, Gtk.MessageType.INFO, Gtk.ButtonsType.OK, message)
        dialog.format_secondary_text(expl)
        dialog.run()
        dialog.destroy()

#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
def main():
   console = Console()

main()
    
