#!/usr/bin/env python3
# -*- coding: UTF-8 -*-
#
# generated by wxGlade 1.1.1 on Mon Mar 31 13:29:51 2025
#

import wx
import subprocess
import os
import sys
import threading
from pathlib import Path
import ndef # Note : install ndefLIB and not ndef

# small hack to get current path to work
Git_repo_path = Path.cwd()
if not(str(Path.cwd()).endswith('NFC_message_switcher')):
    # was run in place -> go up a folder
    Git_repo_path = Git_repo_path.parents[1]

sys.path.append(Git_repo_path.as_posix() + '\\software')
from dataGenerator import message_as_text_list, generate_nfc_c_table, header_content_template

# begin wxGlade: dependencies
import wx.grid
# end wxGlade

# begin wxGlade: extracode
import wx.html2
# end wxGlade


class MyFrame(wx.Frame):
    def __init__(self, *args, **kwds):
        # begin wxGlade: MyFrame.__init__
        kwds["style"] = kwds.get("style", 0) | wx.BORDER_SIMPLE | wx.DEFAULT_FRAME_STYLE
        wx.Frame.__init__(self, *args, **kwds)
        self.SetSize((600, 800))
        self.SetMinSize((600, 800))
        self.SetTitle("NFC tag Programmer")
        _icon = wx.NullIcon
        _icon.CopyFromBitmap(wx.ArtProvider.GetBitmap(wx.ART_PLUS, wx.ART_FRAME_ICON, (32, 32)))
        self.SetIcon(_icon)

        self.window_1 = wx.SplitterWindow(self, wx.ID_ANY, style=wx.SP_LIVE_UPDATE | wx.SP_THIN_SASH)
        self.window_1.SetMinimumPaneSize(20)
        self.window_1.SetSashGravity(1.0)

        self.notebook = wx.Notebook(self.window_1, wx.ID_ANY)

        self.window_2 = wx.SplitterWindow(self.notebook, wx.ID_ANY, style=wx.SP_3D | wx.SP_LIVE_UPDATE)
        self.window_2.SetMinimumPaneSize(20)
        self.window_2.SetSashGravity(0.5)
        self.notebook.AddPage(self.window_2, "device programming")

        self.web_view_panel = wx.html2.WebView.New(self.window_2, wx.ID_ANY)
        self.web_view_panel.LoadURL(str(Git_repo_path) + "\\software\\programmer gui\\manual.htm")

        self.device_programming = wx.ScrolledWindow(self.window_2, wx.ID_ANY, style=wx.FULL_REPAINT_ON_RESIZE)
        self.device_programming.SetScrollRate(10, 10)

        programming_Main_Sizer = wx.BoxSizer(wx.VERTICAL)

        Header_programming_tab = wx.StaticText(self.device_programming, wx.ID_ANY, "NFC tag programmer", style=wx.ALIGN_CENTER_HORIZONTAL)
        Header_programming_tab.SetFont(wx.Font(20, wx.FONTFAMILY_SWISS, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_BOLD, 1, ""))
        programming_Main_Sizer.Add(Header_programming_tab, 0, wx.ALIGN_CENTER_HORIZONTAL | wx.ALL, 10)

        self.grid_data_in = wx.grid.Grid(self.device_programming, wx.ID_ANY)
        self.grid_data_in.CreateGrid(1, 1)
        self.grid_data_in.EnableDragRowSize(0)
        self.grid_data_in.EnableDragGridSize(0)
        self.grid_data_in.SetColLabelValue(0, "Text")
        self.grid_data_in.SetColMinimalWidth(0, 50)
        programming_Main_Sizer.Add(self.grid_data_in, 0, wx.ALIGN_CENTER_HORIZONTAL | wx.ALL, 10)

        grid_sizer_1 = wx.FlexGridSizer(2, 2, 0, 0)
        programming_Main_Sizer.Add(grid_sizer_1, 1, wx.ALIGN_CENTER_HORIZONTAL, 0)

        label_5 = wx.StaticText(self.device_programming, wx.ID_ANY, "iPhone compatability mode : ")
        label_5.SetToolTip("The iPhone cant handle text on its own. This option adds a url that displays text to the text")
        grid_sizer_1.Add(label_5, 0, wx.ALL, 10)

        self.checkbox_Iphone_comp_Mode = wx.CheckBox(self.device_programming, wx.ID_ANY, "")
        self.checkbox_Iphone_comp_Mode.SetToolTip("The iPhone cant handle text on its own. This option adds a url that displays text to the text")
        grid_sizer_1.Add(self.checkbox_Iphone_comp_Mode, 0, wx.ALL, 10)

        label_11 = wx.StaticText(self.device_programming, wx.ID_ANY, "Remove predefined texts: ")
        grid_sizer_1.Add(label_11, 0, wx.ALL, 10)

        self.checkbox_remove_defaults = wx.CheckBox(self.device_programming, wx.ID_ANY, "", style=wx.CHK_2STATE)
        grid_sizer_1.Add(self.checkbox_remove_defaults, 0, wx.ALL, 10)

        self.button_reset_data = wx.Button(self.device_programming, wx.ID_ANY, "Reset data")
        self.button_reset_data.SetMinSize((150, 40))
        self.button_reset_data.SetToolTip("Resets entered data")
        programming_Main_Sizer.Add(self.button_reset_data, 0, wx.ALIGN_CENTER_HORIZONTAL | wx.ALL, 10)

        static_line_1 = wx.StaticLine(self.device_programming, wx.ID_ANY)
        programming_Main_Sizer.Add(static_line_1, 0, wx.ALL | wx.EXPAND, 10)

        programming_button_sizer = wx.BoxSizer(wx.HORIZONTAL)
        programming_Main_Sizer.Add(programming_button_sizer, 1, wx.ALIGN_CENTER_HORIZONTAL | wx.BOTTOM | wx.TOP, 0)

        sub_sizer_1 = wx.BoxSizer(wx.VERTICAL)
        programming_button_sizer.Add(sub_sizer_1, 1, wx.EXPAND, 0)

        self.button_programm_device = wx.Button(self.device_programming, wx.ID_ANY, "program device", style=wx.BU_AUTODRAW)
        self.button_programm_device.SetMinSize((200, 75))
        self.button_programm_device.SetFont(wx.Font(14, wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_NORMAL, 0, "Segoe UI"))
        self.button_programm_device.SetToolTip("Press this button to program the NFC tag")
        sub_sizer_1.Add(self.button_programm_device, 0, wx.ALL, 10)

        self.Programmer_error_label = wx.StaticText(self.device_programming, wx.ID_ANY, "Programmer error", style=wx.ALIGN_CENTER_HORIZONTAL | wx.ST_NO_AUTORESIZE)
        self.Programmer_error_label.SetForegroundColour(wx.Colour(255, 0, 0))
        self.Programmer_error_label.SetFont(wx.Font(16, wx.FONTFAMILY_ROMAN, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_BOLD, 0, "Times New Roman"))
        self.Programmer_error_label.Hide()
        sub_sizer_1.Add(self.Programmer_error_label, 0, wx.ALL | wx.EXPAND | wx.RESERVE_SPACE_EVEN_IF_HIDDEN, 10)

        self.notebook_1_pane_3 = wx.Panel(self.notebook, wx.ID_ANY)
        self.notebook.AddPage(self.notebook_1_pane_3, "programming setup")

        grid_sizer_2 = wx.FlexGridSizer(8, 2, 0, 10)

        label_3 = wx.StaticText(self.notebook_1_pane_3, wx.ID_ANY, "Programer selected :", style=wx.ALIGN_LEFT)
        grid_sizer_2.Add(label_3, 0, wx.ALIGN_CENTER_VERTICAL | wx.EXPAND | wx.LEFT, 10)

        self.choice_programmer = wx.Choice(self.notebook_1_pane_3, wx.ID_ANY, choices=["Snap", "Atmel ICE", "none"])
        self.choice_programmer.SetSelection(0)
        grid_sizer_2.Add(self.choice_programmer, 0, wx.ALIGN_CENTER_VERTICAL | wx.ALL, 10)

        label_10 = wx.StaticText(self.notebook_1_pane_3, wx.ID_ANY, "Max message count :", style=wx.ALIGN_LEFT)
        grid_sizer_2.Add(label_10, 0, wx.ALIGN_CENTER_VERTICAL | wx.EXPAND | wx.LEFT, 10)

        self.spin_ctrl_max_message_count = wx.SpinCtrl(self.notebook_1_pane_3, wx.ID_ANY, "10", min=1, max=100)
        grid_sizer_2.Add(self.spin_ctrl_max_message_count, 0, wx.ALL, 10)

        label_4 = wx.StaticText(self.notebook_1_pane_3, wx.ID_ANY, "Path to source :", style=wx.ALIGN_CENTER_HORIZONTAL)
        grid_sizer_2.Add(label_4, 0, wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 10)

        self.path_to_source_text = wx.TextCtrl(self.notebook_1_pane_3, wx.ID_ANY, "<Auto filled>", style=wx.TE_NO_VSCROLL | wx.TE_PROCESS_ENTER)
        self.path_to_source_text.SetMinSize((600, 23))
        self.path_to_source_text.SetValue(Git_repo_path.as_posix().replace('C:', '/c'))
        grid_sizer_2.Add(self.path_to_source_text, 0, wx.ALL, 10)

        label_6 = wx.StaticText(self.notebook_1_pane_3, wx.ID_ANY, "Build command :", style=wx.ALIGN_CENTER_HORIZONTAL)
        grid_sizer_2.Add(label_6, 0, wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 10)

        self.build_command = wx.TextCtrl(self.notebook_1_pane_3, wx.ID_ANY, "make clean all", style=wx.TE_NO_VSCROLL | wx.TE_PROCESS_ENTER)
        self.build_command.SetMinSize((600, 23))
        grid_sizer_2.Add(self.build_command, 0, wx.ALIGN_CENTER_VERTICAL | wx.ALL, 10)

        label_7 = wx.StaticText(self.notebook_1_pane_3, wx.ID_ANY, "Upload command :", style=wx.ALIGN_CENTER_HORIZONTAL)
        grid_sizer_2.Add(label_7, 0, wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 10)

        self.upload_command = wx.TextCtrl(self.notebook_1_pane_3, wx.ID_ANY, "make upload", style=wx.TE_NO_VSCROLL | wx.TE_PROCESS_ENTER)
        self.upload_command.SetMinSize((600, 23))
        grid_sizer_2.Add(self.upload_command, 0, wx.ALIGN_CENTER_VERTICAL | wx.ALL, 10)

        label_8 = wx.StaticText(self.notebook_1_pane_3, wx.ID_ANY, "Kill uploader command :", style=wx.ALIGN_CENTER_HORIZONTAL)
        grid_sizer_2.Add(label_8, 0, wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 10)

        self.kill_command = wx.TextCtrl(self.notebook_1_pane_3, wx.ID_ANY, "make end_uploader", style=wx.TE_NO_VSCROLL | wx.TE_PROCESS_ENTER)
        self.kill_command.SetMinSize((600, 23))
        grid_sizer_2.Add(self.kill_command, 0, wx.ALIGN_CENTER_VERTICAL | wx.ALL, 10)

        label_9 = wx.StaticText(self.notebook_1_pane_3, wx.ID_ANY, "Stop Uploader :", style=wx.ALIGN_CENTER_HORIZONTAL)
        grid_sizer_2.Add(label_9, 0, wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 10)

        self.button_stop_programmer = wx.Button(self.notebook_1_pane_3, wx.ID_ANY, "stop uploader")
        self.button_stop_programmer.SetMinSize((150, 40))
        self.button_stop_programmer.SetToolTip("stops the programmer task")
        grid_sizer_2.Add(self.button_stop_programmer, 0, wx.ALL, 10)

        grid_sizer_2.Add((0, 0), 0, 0, 0)

        grid_sizer_2.Add((0, 0), 0, 0, 0)

        self.console_out = wx.TextCtrl(self.window_1, wx.ID_ANY, "", style=wx.BORDER_NONE | wx.TE_AUTO_URL | wx.TE_MULTILINE | wx.TE_READONLY | wx.TE_WORDWRAP)

        self.notebook_1_pane_3.SetSizer(grid_sizer_2)

        self.device_programming.SetSizer(programming_Main_Sizer)

        self.window_2.SplitVertically(self.web_view_panel, self.device_programming)

        self.window_1.SplitHorizontally(self.notebook, self.console_out)

        self.Layout()
        self.Centre()

        self.grid_data_in.Bind(wx.grid.EVT_GRID_CMD_CELL_CHANGED, self.grid_changed)
        self.button_reset_data.Bind(wx.EVT_BUTTON, self.Reset_Data_entered)
        self.button_programm_device.Bind(wx.EVT_BUTTON, self.program_device)
        self.button_stop_programmer.Bind(wx.EVT_BUTTON, self.stop_programmer)
        # end wxGlade
        self.Bind(wx.EVT_CLOSE, self.OnClose)
        self.programmer_task_running = False
        self.wait_dialog = WaitNotification(self)

    def grid_changed(self, event):  # wxGlade: MyFrame.<event_handler>
        empty_rows = False
        self.grid_data_in.AutoSizeColumns(True)
        if self.grid_data_in.GetNumberRows() >= self.spin_ctrl_max_message_count.Value:
            self.device_programming.Layout()
            self.device_programming.FitInside()
            return

        for row in range(self.grid_data_in.GetNumberRows()):
            if (self.grid_data_in.GetCellValue(row, 0) == ""):
                empty_rows = True
                break
        if empty_rows == False:
            self.grid_data_in.AppendRows(1)
        self.device_programming.Layout()
        self.device_programming.FitInside()


    def Reset_Data_entered(self, event):  # wxGlade: MyFrame.<event_handler>
        self.grid_data_in.ClearGrid()
        if self.grid_data_in.GetNumberRows() > 1:
            self.grid_data_in.DeleteRows(1, self.grid_data_in.GetNumberRows())
            self.grid_data_in.SetColSize(0, 100)
            self.device_programming.Layout()
        self.checkbox_Iphone_comp_Mode.SetValue(False)
        self.checkbox_remove_defaults.SetValue(False)
        self.console_out.Clear()
        self.device_programming.FitInside()


    def program_device(self, event):  # wxGlade: MyFrame.<event_handler>
        self.disable_control()
        programmer_Book = {0:"TPSNAP", 1:"TPAICE"}
        self.wait_dialog.Show()
        self.Programmer_error_label.Hide()
        self.console_out.Clear()

        def paralell_task():
            ret = self.execute_command(self.build_command.Value, '/software/src', True)
            self.generate_h_file()
            if ret != 0:
                wx.CallAfter(self.Programmer_error_label.Show)
                wx.CallAfter(self.wait_dialog.Hide)
                return
            self.check_ipe_lockfile()
            cmd = self.upload_command.Value +" PROGTOOL=" +  programmer_Book[self.choice_programmer.Selection]
            ret = self.execute_command(cmd, '/software/src', True)
            if ret != 0:
                wx.CallAfter(self.Programmer_error_label.Show)
                self.programmer_task_running = False
                self.check_ipe_lockfile()
            else:
                self.programmer_task_running = True
            wx.CallAfter(self.wait_dialog.Hide)
            wx.CallAfter(self.enable_user_input)

        threading.Thread(target=paralell_task).start()
        return

    def stop_programmer(self, event):  # wxGlade: MyFrame.<event_handler>
        self.execute_command(self.kill_command.Value, '/software/src', True)
        self.programmer_task_running = False

    def OnClose(self, event):
        if self.programmer_task_running:
            print("END THE DAM PROGRAMMER!!!!!!")
            self.execute_command('make end_uploader', '/software/src', True)

        self.Destroy()

    def execute_command(self, cmd, where = '', addSourceDirectory = False) -> int:
        cmd =  'cd ' + (self.path_to_source_text.Value if addSourceDirectory else '') + where + ' && '+  cmd
        env = os.environ.copy()
        env["PATH"] = "/mingw64/bin:/usr/bin:/bin:" + env.get("PATH", "")
        result = subprocess.run(
            r'C:\msys64\usr\bin\bash.exe --norc -c "{}"'.format(cmd),
            shell=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            universal_newlines=True,
            env=env
        )
        wx.CallAfter(self.console_out.AppendText, result.stdout)
        return result.returncode

    def generate_h_file(self):
        messages_user_in = []
        messages_lines = []
        name_of_messagearray = []

        for row in range(self.grid_data_in.GetNumberRows()):
            if self.grid_data_in.GetCellValue(row, 0) != "":
                messages_user_in.append(self.grid_data_in.GetCellValue(row, 0))

        if len(messages_user_in) == 0 and self.checkbox_remove_defaults.Value:
            messages_user_in.append("I identify as empty")

        for num, message in enumerate(messages_user_in if self.checkbox_remove_defaults.Value else messages_user_in + message_as_text_list):
            if self.checkbox_Iphone_comp_Mode.Value:
                if not(message.startswith('http') or message.startswith('spotify:track:')):
                    message = "https://large-type.com/#" + message

            messages_lines.append(f"// \"{message}\"")
            messages_lines.append(f"const uint8_t dat{num}[] = "+ generate_nfc_c_table([ndef.UriRecord(message)], read_only=False))
        
        number_of_messages = int(len(messages_lines)/2)

        messages_multilinetext = "\n".join(messages_lines)
        
        name_of_messagearray = ", ".join([f"dat{num}" for num in range(number_of_messages)])

        with open(Git_repo_path.as_posix() + "/software/src/nfc_messages.h", "w") as header_file:
            header_file.write(header_content_template.format(messages_multilinetext, 
                                                             name_of_messagearray, 
                                                             number_of_messages))
        self.console_out.AppendText("Generated Header File\n")

    def check_ipe_lockfile(self):
        lockfile_path = Path(str(Path.home()) + "\\.mchp_ipe\\2025.lock") 
        inifile_path = Path(str(Path.home()) + "\\.mchp_ipe\\2025.ini")
        if lockfile_path.is_file():
            os.remove(lockfile_path)
            self.console_out.AppendText(" --- Deleted Lock file ---\n")
        if inifile_path.is_file():
            os.remove(inifile_path)
            self.console_out.AppendText(" --- Deleted ini file ---\n")


    def disable_control(self):
        self.button_programm_device.Disable()
        self.button_reset_data.Disable()
        self.grid_data_in.Disable()
        self.checkbox_Iphone_comp_Mode.Disable()
        self.checkbox_remove_defaults.Disable()
        self.notebook.Disable()

    def enable_user_input(self):
        self.button_programm_device.Enable()
        self.button_reset_data.Enable()
        self.grid_data_in.Enable()
        self.checkbox_Iphone_comp_Mode.Enable()
        self.checkbox_remove_defaults.Enable()
        self.notebook.Enable()        

# end of class MyFrame

class WaitNotification(wx.Dialog):
    def __init__(self, *args, **kwds):
        # begin wxGlade: WaitNotification.__init__
        kwds["style"] = kwds.get("style", 0) | wx.STAY_ON_TOP
        wx.Dialog.__init__(self, *args, **kwds)
        self.SetTitle("Programmer wait")
        self.Hide()

        sizer_1 = wx.BoxSizer(wx.VERTICAL)

        label_1 = wx.StaticText(self, wx.ID_ANY, "Programming! please wait")
        label_1.SetFont(wx.Font(21, wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_BOLD, 0, ""))
        sizer_1.Add(label_1, 0, wx.ALIGN_CENTER_HORIZONTAL | wx.ALL, 10)

        label_2 = wx.StaticText(self, wx.ID_ANY, "The main window may seem unresponsive, please wait until it closes", style=wx.ALIGN_CENTER_HORIZONTAL)
        sizer_1.Add(label_2, 0, wx.ALIGN_CENTER_HORIZONTAL | wx.ALL, 50)

        sizer_1.Add((0, 0), 0, 0, 0)

        self.SetSizer(sizer_1)
        sizer_1.Fit(self)

        self.Layout()
        self.Centre()
        # end wxGlade

# end of class WaitNotification

class MyApp(wx.App):
    def OnInit(self):
        self.MainFrame = MyFrame(None, wx.ID_ANY, "")
        self.SetTopWindow(self.MainFrame)
        self.MainFrame.Show()
        return True

# end of class MyApp

if __name__ == "__main__":
    NFC_Tag_programmer = MyApp(0)
    NFC_Tag_programmer.MainLoop()
