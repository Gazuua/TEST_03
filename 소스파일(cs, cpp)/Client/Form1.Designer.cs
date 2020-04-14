namespace NEOPLE_TEST_03_CLIENTS
{
    partial class Form1
    {
        /// <summary>
        /// 필수 디자이너 변수입니다.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 사용 중인 모든 리소스를 정리합니다.
        /// </summary>
        /// <param name="disposing">관리되는 리소스를 삭제해야 하면 true이고, 그렇지 않으면 false입니다.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form 디자이너에서 생성한 코드

        /// <summary>
        /// 디자이너 지원에 필요한 메서드입니다. 
        /// 이 메서드의 내용을 코드 편집기로 수정하지 마세요.
        /// </summary>
        private void InitializeComponent()
        {
            this.LABEL_IP = new System.Windows.Forms.Label();
            this.TEXT_IP = new System.Windows.Forms.TextBox();
            this.LABEL_PORT = new System.Windows.Forms.Label();
            this.TEXT_PORT = new System.Windows.Forms.TextBox();
            this.BTN_CONN = new System.Windows.Forms.Button();
            this.LIST_USER = new System.Windows.Forms.ListView();
            this.USER = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.LABEL_USERLIST = new System.Windows.Forms.Label();
            this.LABEL_CHAT = new System.Windows.Forms.Label();
            this.TEXT_CHAT = new System.Windows.Forms.TextBox();
            this.TEXT_INPUT = new System.Windows.Forms.TextBox();
            this.BTN_INPUT = new System.Windows.Forms.Button();
            this.TEXT_NAME = new System.Windows.Forms.TextBox();
            this.LABEL_NICKNAME = new System.Windows.Forms.Label();
            this.BTN_NAME = new System.Windows.Forms.Button();
            this.CHK_WHISPER = new System.Windows.Forms.CheckBox();
            this.SuspendLayout();
            // 
            // LABEL_IP
            // 
            this.LABEL_IP.AutoSize = true;
            this.LABEL_IP.Location = new System.Drawing.Point(12, 16);
            this.LABEL_IP.Name = "LABEL_IP";
            this.LABEL_IP.Size = new System.Drawing.Size(78, 15);
            this.LABEL_IP.TabIndex = 0;
            this.LABEL_IP.Text = "IP Address";
            // 
            // TEXT_IP
            // 
            this.TEXT_IP.Location = new System.Drawing.Point(96, 12);
            this.TEXT_IP.Name = "TEXT_IP";
            this.TEXT_IP.Size = new System.Drawing.Size(138, 25);
            this.TEXT_IP.TabIndex = 2;
            this.TEXT_IP.Text = "127.0.0.1";
            // 
            // LABEL_PORT
            // 
            this.LABEL_PORT.AutoSize = true;
            this.LABEL_PORT.Location = new System.Drawing.Point(240, 16);
            this.LABEL_PORT.Name = "LABEL_PORT";
            this.LABEL_PORT.Size = new System.Drawing.Size(47, 15);
            this.LABEL_PORT.TabIndex = 2;
            this.LABEL_PORT.Text = "PORT";
            // 
            // TEXT_PORT
            // 
            this.TEXT_PORT.Location = new System.Drawing.Point(293, 12);
            this.TEXT_PORT.MaxLength = 5;
            this.TEXT_PORT.Name = "TEXT_PORT";
            this.TEXT_PORT.Size = new System.Drawing.Size(62, 25);
            this.TEXT_PORT.TabIndex = 3;
            this.TEXT_PORT.Text = "55248";
            // 
            // BTN_CONN
            // 
            this.BTN_CONN.Location = new System.Drawing.Point(362, 8);
            this.BTN_CONN.Name = "BTN_CONN";
            this.BTN_CONN.Size = new System.Drawing.Size(60, 31);
            this.BTN_CONN.TabIndex = 4;
            this.BTN_CONN.Text = "접속";
            this.BTN_CONN.UseVisualStyleBackColor = true;
            this.BTN_CONN.Click += new System.EventHandler(this.BTN_CONN_Click);
            // 
            // LIST_USER
            // 
            this.LIST_USER.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.USER});
            this.LIST_USER.FullRowSelect = true;
            this.LIST_USER.GridLines = true;
            this.LIST_USER.HideSelection = false;
            this.LIST_USER.Location = new System.Drawing.Point(12, 72);
            this.LIST_USER.MultiSelect = false;
            this.LIST_USER.Name = "LIST_USER";
            this.LIST_USER.Size = new System.Drawing.Size(206, 134);
            this.LIST_USER.TabIndex = 5;
            this.LIST_USER.TabStop = false;
            this.LIST_USER.UseCompatibleStateImageBehavior = false;
            this.LIST_USER.View = System.Windows.Forms.View.Details;
            // 
            // USER
            // 
            this.USER.Text = "USER_ID";
            this.USER.Width = 206;
            // 
            // LABEL_USERLIST
            // 
            this.LABEL_USERLIST.AutoSize = true;
            this.LABEL_USERLIST.Location = new System.Drawing.Point(67, 50);
            this.LABEL_USERLIST.Name = "LABEL_USERLIST";
            this.LABEL_USERLIST.Size = new System.Drawing.Size(87, 15);
            this.LABEL_USERLIST.TabIndex = 6;
            this.LABEL_USERLIST.Text = "유저 리스트";
            // 
            // LABEL_CHAT
            // 
            this.LABEL_CHAT.AutoSize = true;
            this.LABEL_CHAT.Location = new System.Drawing.Point(180, 220);
            this.LABEL_CHAT.Name = "LABEL_CHAT";
            this.LABEL_CHAT.Size = new System.Drawing.Size(72, 15);
            this.LABEL_CHAT.TabIndex = 7;
            this.LABEL_CHAT.Text = "대화 내용";
            // 
            // TEXT_CHAT
            // 
            this.TEXT_CHAT.Location = new System.Drawing.Point(12, 252);
            this.TEXT_CHAT.MaxLength = 2000000000;
            this.TEXT_CHAT.Multiline = true;
            this.TEXT_CHAT.Name = "TEXT_CHAT";
            this.TEXT_CHAT.ReadOnly = true;
            this.TEXT_CHAT.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.TEXT_CHAT.Size = new System.Drawing.Size(412, 250);
            this.TEXT_CHAT.TabIndex = 8;
            this.TEXT_CHAT.TabStop = false;
            // 
            // TEXT_INPUT
            // 
            this.TEXT_INPUT.Enabled = false;
            this.TEXT_INPUT.Location = new System.Drawing.Point(12, 520);
            this.TEXT_INPUT.MaxLength = 50;
            this.TEXT_INPUT.Name = "TEXT_INPUT";
            this.TEXT_INPUT.Size = new System.Drawing.Size(343, 25);
            this.TEXT_INPUT.TabIndex = 0;
            this.TEXT_INPUT.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.TEXT_INPUT_KeyPress);
            // 
            // BTN_INPUT
            // 
            this.BTN_INPUT.Enabled = false;
            this.BTN_INPUT.Location = new System.Drawing.Point(361, 518);
            this.BTN_INPUT.Name = "BTN_INPUT";
            this.BTN_INPUT.Size = new System.Drawing.Size(60, 31);
            this.BTN_INPUT.TabIndex = 1;
            this.BTN_INPUT.Text = "전송";
            this.BTN_INPUT.UseVisualStyleBackColor = true;
            this.BTN_INPUT.Click += new System.EventHandler(this.BTN_INPUT_Click);
            // 
            // TEXT_NAME
            // 
            this.TEXT_NAME.Location = new System.Drawing.Point(273, 99);
            this.TEXT_NAME.MaxLength = 12;
            this.TEXT_NAME.Name = "TEXT_NAME";
            this.TEXT_NAME.ReadOnly = true;
            this.TEXT_NAME.Size = new System.Drawing.Size(115, 25);
            this.TEXT_NAME.TabIndex = 9;
            // 
            // LABEL_NICKNAME
            // 
            this.LABEL_NICKNAME.AutoSize = true;
            this.LABEL_NICKNAME.Location = new System.Drawing.Point(304, 72);
            this.LABEL_NICKNAME.Name = "LABEL_NICKNAME";
            this.LABEL_NICKNAME.Size = new System.Drawing.Size(52, 15);
            this.LABEL_NICKNAME.TabIndex = 10;
            this.LABEL_NICKNAME.Text = "닉네임";
            // 
            // BTN_NAME
            // 
            this.BTN_NAME.Enabled = false;
            this.BTN_NAME.Location = new System.Drawing.Point(303, 138);
            this.BTN_NAME.Name = "BTN_NAME";
            this.BTN_NAME.Size = new System.Drawing.Size(60, 31);
            this.BTN_NAME.TabIndex = 11;
            this.BTN_NAME.Text = "설정";
            this.BTN_NAME.UseVisualStyleBackColor = true;
            this.BTN_NAME.Click += new System.EventHandler(this.BTN_NICKNAME_Click);
            // 
            // CHK_WHISPER
            // 
            this.CHK_WHISPER.AutoSize = true;
            this.CHK_WHISPER.Location = new System.Drawing.Point(15, 220);
            this.CHK_WHISPER.Name = "CHK_WHISPER";
            this.CHK_WHISPER.Size = new System.Drawing.Size(121, 19);
            this.CHK_WHISPER.TabIndex = 12;
            this.CHK_WHISPER.Text = "귓속말 보내기";
            this.CHK_WHISPER.UseVisualStyleBackColor = true;
            // 
            // Form1
            // 
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.None;
            this.ClientSize = new System.Drawing.Size(434, 561);
            this.Controls.Add(this.CHK_WHISPER);
            this.Controls.Add(this.BTN_NAME);
            this.Controls.Add(this.LABEL_NICKNAME);
            this.Controls.Add(this.TEXT_NAME);
            this.Controls.Add(this.BTN_INPUT);
            this.Controls.Add(this.TEXT_INPUT);
            this.Controls.Add(this.TEXT_CHAT);
            this.Controls.Add(this.LABEL_CHAT);
            this.Controls.Add(this.LABEL_USERLIST);
            this.Controls.Add(this.LIST_USER);
            this.Controls.Add(this.BTN_CONN);
            this.Controls.Add(this.TEXT_PORT);
            this.Controls.Add(this.LABEL_PORT);
            this.Controls.Add(this.TEXT_IP);
            this.Controls.Add(this.LABEL_IP);
            this.Font = new System.Drawing.Font("굴림", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
            this.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
            this.MaximizeBox = false;
            this.Name = "Form1";
            this.Text = "Form1";
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.Form1_FormClosed);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label LABEL_IP;
        private System.Windows.Forms.TextBox TEXT_IP;
        private System.Windows.Forms.Label LABEL_PORT;
        private System.Windows.Forms.TextBox TEXT_PORT;
        private System.Windows.Forms.Button BTN_CONN;
        private System.Windows.Forms.ListView LIST_USER;
        private System.Windows.Forms.Label LABEL_USERLIST;
        private System.Windows.Forms.Label LABEL_CHAT;
        private System.Windows.Forms.TextBox TEXT_CHAT;
        private System.Windows.Forms.TextBox TEXT_INPUT;
        private System.Windows.Forms.Button BTN_INPUT;
        private System.Windows.Forms.TextBox TEXT_NAME;
        private System.Windows.Forms.Label LABEL_NICKNAME;
        private System.Windows.Forms.Button BTN_NAME;
        private System.Windows.Forms.ColumnHeader USER;
        private System.Windows.Forms.CheckBox CHK_WHISPER;
    }
}

