using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Diagnostics;

namespace OverGUI
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
            MaximizeBox = false;
            FixFonts();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            ProcessStartInfo processInfo = new ProcessStartInfo();
            processInfo.FileName = @"DllInjector.exe";
            processInfo.UseShellExecute = true;
            processInfo.RedirectStandardOutput = false;
            processInfo.RedirectStandardError = false;
            processInfo.CreateNoWindow = false;
            processInfo.Arguments = OptionalArguments.Text;
            try
            {
                using (Process process = Process.Start(processInfo))
                {
                    // Capture the output
                    //string output = process.StandardOutput.ReadToEnd();
                    //string error = process.StandardError.ReadToEnd();

                    // Display the output in a TextBox or other control
                    //textBox1.Text += output + "\n" + error;

                    process.WaitForExit();
                    textBox1.AppendText("Done! \n");
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("The operation was canceled or failed: " + ex.Message);
            }
        }
        private void button3_Click(object sender, EventArgs e)
        {
            OpenFileDialog overwatch = new OpenFileDialog();
            overwatch.InitialDirectory = System.AppDomain.CurrentDomain.BaseDirectory;
            overwatch.DefaultExt = @".dll";
            overwatch.Filter = @"DLL file|*.dll";
            overwatch.Title = "Browse for DLL";
            overwatch.ShowDialog();
            if (overwatch.FileName != string.Empty)
            {
                string arguments = "\"" + overwatch.FileName + "\" " + OptionalArguments.Text;
                OptionalArguments.Text = arguments;
            }
            overwatch = new OpenFileDialog();
            overwatch.InitialDirectory = System.AppDomain.CurrentDomain.BaseDirectory;
            overwatch.DefaultExt = @".exe";
            overwatch.Filter = @"Windows Executable|*.exe";
            overwatch.Title = "Browse for executable to inject";
            overwatch.ShowDialog();
            if (overwatch.FileName != string.Empty)
            {
                string arguments = "\"" + overwatch.FileName + "\" " + OptionalArguments.Text;
                OptionalArguments.Text = arguments;
            }
        }
        private void changeToWhite(object sender, EventArgs e)
        {
            Button button = sender as Button;
            button.Image = Properties.Resources.buttonHover;
            //button.BackColor = Color.Transparent;
        }
        private void changeToYellow(object sender, EventArgs e)
        {
            Button button = sender as Button;
            button.Image = Properties.Resources.buttonRegular;
        }

        private void Form1_Load(object sender, EventArgs e)
        {

        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {

        }

        private void label1_Click(object sender, EventArgs e)
        {

        }

        private void pictureBox1_Click(object sender, EventArgs e)
        {

        }
    }
}
