using System.Windows.Forms;

namespace OverGUI
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Alon Region
        private void FixFonts()
        {
            /*
            SuspendLayout();
            button1.Font = new Font("Microsoft Sans Serif", 40F);
            textBox1.Font = new Font("Microsoft Sans Serif", 20F);
            ResumeLayout(false);
            PerformLayout();
            */
        }
        #endregion

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            Label label2;
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form1));
            button1 = new Button();
            button3 = new Button();
            fileSystemWatcher1 = new FileSystemWatcher();
            textBox1 = new TextBox();
            label1 = new Label();
            OptionalArguments = new TextBox();
            openFileDialog2 = new OpenFileDialog();
            button4 = new Button();
            label2 = new Label();
            ((System.ComponentModel.ISupportInitialize)fileSystemWatcher1).BeginInit();
            SuspendLayout();
            // 
            // label2
            // 
            label2.Font = new Font("Microsoft Sans Serif", 20F);
            label2.ForeColor = SystemColors.ButtonHighlight;
            label2.Location = new Point(90, 729);
            label2.Name = "label2";
            label2.Size = new Size(446, 32);
            label2.TabIndex = 3;
            label2.Text = "Optional Command Line Arguments";
            // 
            // button1
            // 
            button1.BackColor = Color.Transparent;
            button1.BackgroundImageLayout = ImageLayout.Zoom;
            button1.FlatAppearance.BorderSize = 0;
            button1.FlatAppearance.MouseDownBackColor = Color.Transparent;
            button1.FlatAppearance.MouseOverBackColor = Color.Transparent;
            button1.FlatStyle = FlatStyle.Flat;
            button1.Font = new Font("Microsoft Sans Serif", 25F);
            button1.Image = Properties.Resources.buttonRegular;
            button1.ImageAlign = ContentAlignment.MiddleRight;
            button1.Location = new Point(602, 334);
            button1.Name = "button1";
            button1.Size = new Size(445, 163);
            button1.TabIndex = 0;
            button1.Text = "      Inject";
            button1.UseVisualStyleBackColor = false;
            button1.Click += button1_Click;
            button1.MouseEnter += changeToWhite;
            button1.MouseLeave += changeToYellow;
            // 
            // button3
            // 
            button3.BackColor = Color.Transparent;
            button3.BackgroundImageLayout = ImageLayout.Zoom;
            button3.FlatAppearance.BorderSize = 0;
            button3.FlatAppearance.MouseDownBackColor = Color.Transparent;
            button3.FlatAppearance.MouseOverBackColor = Color.Transparent;
            button3.FlatStyle = FlatStyle.Flat;
            button3.Font = new Font("Microsoft Sans Serif", 25F);
            button3.Image = Properties.Resources.buttonRegular;
            button3.Location = new Point(623, 574);
            button3.Name = "button3";
            button3.Size = new Size(445, 163);
            button3.TabIndex = 0;
            button3.Text = "Choose DLL";
            button3.UseVisualStyleBackColor = false;
            button3.Click += button3_Click;
            button3.MouseEnter += changeToWhite;
            button3.MouseLeave += changeToYellow;
            // 
            // fileSystemWatcher1
            // 
            fileSystemWatcher1.EnableRaisingEvents = true;
            fileSystemWatcher1.SynchronizingObject = this;
            // 
            // textBox1
            // 
            textBox1.Anchor = AnchorStyles.Bottom;
            textBox1.Font = new Font("Consolas", 12F);
            textBox1.Location = new Point(90, 856);
            textBox1.Multiline = true;
            textBox1.Name = "textBox1";
            textBox1.ReadOnly = true;
            textBox1.ScrollBars = ScrollBars.Vertical;
            textBox1.Size = new Size(900, 170);
            textBox1.TabIndex = 2;
            textBox1.TextChanged += textBox1_TextChanged;
            // 
            // label1
            // 
            label1.Font = new Font("Microsoft Sans Serif", 20F);
            label1.ForeColor = SystemColors.ButtonHighlight;
            label1.Location = new Point(90, 821);
            label1.Name = "label1";
            label1.Size = new Size(138, 32);
            label1.TabIndex = 3;
            label1.Text = "Logs";
            label1.Click += label1_Click;
            // 
            // OptionalArguments
            // 
            OptionalArguments.Anchor = AnchorStyles.Bottom;
            OptionalArguments.Font = new Font("Consolas", 20F);
            OptionalArguments.Location = new Point(90, 764);
            OptionalArguments.Name = "OptionalArguments";
            OptionalArguments.ScrollBars = ScrollBars.Vertical;
            OptionalArguments.Size = new Size(900, 39);
            OptionalArguments.TabIndex = 2;
            OptionalArguments.TextChanged += textBox1_TextChanged;
            // 
            // openFileDialog2
            // 
            openFileDialog2.FileName = "openFileDialog2";
            // 
            // button4
            // 
            button4.BackColor = Color.Transparent;
            button4.BackgroundImageLayout = ImageLayout.Zoom;
            button4.FlatAppearance.BorderSize = 0;
            button4.FlatAppearance.MouseDownBackColor = Color.Transparent;
            button4.FlatAppearance.MouseOverBackColor = Color.Transparent;
            button4.FlatStyle = FlatStyle.Flat;
            button4.Font = new Font("Microsoft Sans Serif", 25F);
            button4.Image = Properties.Resources.buttonRegular;
            button4.ImageAlign = ContentAlignment.MiddleRight;
            button4.Location = new Point(623, 127);
            button4.Name = "button4";
            button4.Size = new Size(445, 163);
            button4.TabIndex = 0;
            button4.Text = "    Deobfuscate";
            button4.UseVisualStyleBackColor = false;
            button4.Click += button4_Click;
            button4.MouseEnter += changeToWhite;
            button4.MouseLeave += changeToYellow;
            // 
            // Form1
            // 
            AutoScaleMode = AutoScaleMode.None;
            BackColor = Color.FromArgb(31, 38, 46);
            BackgroundImage = Properties.Resources.background;
            BackgroundImageLayout = ImageLayout.Zoom;
            ClientSize = new Size(1080, 941);
            Controls.Add(label2);
            Controls.Add(label1);
            Controls.Add(OptionalArguments);
            Controls.Add(textBox1);
            Controls.Add(button3);
            Controls.Add(button4);
            Controls.Add(button1);
            FormBorderStyle = FormBorderStyle.FixedSingle;
            Icon = (Icon)resources.GetObject("$this.Icon");
            Margin = new Padding(3, 13, 3, 13);
            Name = "Form1";
            Text = "OverPatcher";
            Load += Form1_Load;
            ((System.ComponentModel.ISupportInitialize)fileSystemWatcher1).EndInit();
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion

        private Button button1;
        private Button button2;
        private Button button3;
        private FileSystemWatcher fileSystemWatcher1;
        private TextBox textBox1;
        private Label label1;
        private TextBox OptionalArguments;
        private OpenFileDialog openFileDialog2;
        private Button button4;
    }
}