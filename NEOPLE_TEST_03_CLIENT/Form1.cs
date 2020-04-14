using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace NEOPLE_TEST_03_CLIENTS
{
    public partial class Form1 : Form
    {
        SocketClient client;

        public Form1()
        {
            InitializeComponent();
        }

        // Logic 1. 접속 버튼을 클릭
        // 사용자가 입력한 IP와 PORT를 바탕으로 서버에 접속을 시도한다.
        // 기본값은 localhost(127.0.0.1), 55248
        private void BTN_CONN_Click(object sender, EventArgs e)
        {
            String ip = this.TEXT_IP.Text;
            int port = int.Parse(this.TEXT_PORT.Text);

            client = new SocketClient(ip, port);

            // 접속 실패 시 다시하라고 알림
            if (!client.Connect())
            {
                showMessageBox("서버 접속에 실패하였습니다. 주소 혹은 포트번호가 틀리지 않았는지 확인해 주세요.");
                return;
            }

            // 접속 성공 시 폼 및 클라이언트 셋팅
            this.TEXT_IP.Enabled = false;
            this.TEXT_PORT.Enabled = false;
            this.BTN_CONN.Enabled = false;
            this.BTN_NAME.Enabled = true;

            // SocketClient에서 폼 관련 함수를 다룰 수 있도록 콜백(델리게이트) 설정
            client.setDel_chatText(new SocketClient.Callback_string(appendText_Chat));
            client.setDel_showMsgBox(new SocketClient.Callback_string(showMessageBox));
            client.setDel_refreshUser_List(new SocketClient.Callback_list(refreshUser_List));
            client.setDel_nicknameDupCheck(new SocketClient.Callback_string(nicknameDupCheck));
            client.setDel_noticeDisconnect(new SocketClient.Callback(noticeDisconnect));

            // 채팅방 이용 전 닉네임 설정을 해야 함을 알림
            this.TEXT_NAME.ReadOnly = false;
            showMessageBox("서버 접속에 성공하였습니다.\r\n채팅방에서 사용할 닉네임 설정을 해주세요.");

            // 일단 메세지는 받을수 있도록 스레드를 시작
            client.startReadThread();
        }

        // Logic 2. 닉네임 설정 버튼을 클릭 (서버 접속 이후 활성화)
        // 사용자가 입력한 닉네임을 바탕으로 서버에 자신의 닉네임 설정을 요청한다.
        private void BTN_NICKNAME_Click(object sender, EventArgs e)
        {
            String name = this.TEXT_NAME.Text;

            if ( name == null || name == "" )
            {
                showMessageBox("올바른 닉네임을 입력해 주세요.");
                return;
            }

            // 서버에 본인 닉네임 설정 요청
            client.sendMessage(name, Packet.Code.NickName);
        }

        // Logic 3. 전송 버튼을 클릭시 채팅 메세지 전송
        private void BTN_INPUT_Click(object sender, EventArgs e)
        {
            String msg = this.TEXT_INPUT.Text;
            
            if (msg == "" || msg == null)
                return;

            // 귓속말 버튼이 클릭되어 있을 경우
            if (this.CHK_WHISPER.Checked)
            {
                // 리스트에서 선택한 유저도 있을 경우 귓속말을 그 사람에게 보낸다
                if (this.LIST_USER.SelectedItems.Count == 1)
                {
                    String whisperMsg = this.LIST_USER.SelectedItems[0].Text + "/" + msg;
                    client.sendMessage(whisperMsg, Packet.Code.Whisper);
                    this.TEXT_INPUT.Clear();
                }
                // 리스트에서 선택한 유저가 없을 경우 선택하라고 알린다
                else
                {
                    showMessageBox("귓속말을 보낼 사람을 선택해주세요.");
                }
                return;
            }

            client.sendMessage(msg);
            this.TEXT_INPUT.Clear();
        }

        // 채팅 텍스트박스에서 엔터 키를 누르면 채팅 메세지 전송
        private void TEXT_INPUT_KeyPress(object sender, KeyPressEventArgs e)
        {
            // 엔터 키일 경우 전송한다.
            if (e.KeyChar == (char)Keys.Enter)
            {
                String msg = this.TEXT_INPUT.Text;

                if (msg == "" || msg == null)
                    return;

                // 귓속말 버튼이 클릭되어 있을 경우
                if (this.CHK_WHISPER.Checked)
                {
                    // 리스트에서 선택한 유저도 있을 경우 귓속말을 그 사람에게 보낸다
                    if (this.LIST_USER.SelectedItems.Count == 1)
                    {
                        String whisperMsg = this.LIST_USER.SelectedItems[0].Text + "/" + msg;
                        client.sendMessage(whisperMsg, Packet.Code.Whisper);
                        this.TEXT_INPUT.Clear();
                    }
                    // 리스트에서 선택한 유저가 없을 경우 선택하라고 알린다
                    else
                    {
                        showMessageBox("귓속말을 보낼 사람을 선택해주세요.");
                    }
                    return;
                }

                client.sendMessage(msg);
                this.TEXT_INPUT.Clear();
            }
        }

        // 중복된 닉네임을 점검하는 함수
        private void nicknameDupCheck(string result)
        {
            if (result == "FAIL")
            {
                showMessageBox("중복된 닉네임입니다.\r\n다른 닉네임으로 다시 시도하여 주세요.");
                return;
            }

            // 닉네임 설정 되었을 시
            this.TEXT_NAME.ReadOnly = true;
            this.TEXT_INPUT.Enabled = true;
            this.BTN_NAME.Enabled = false;
            this.BTN_INPUT.Enabled = true;
            showMessageBox("[" + this.TEXT_NAME.Text + "]으로 닉네임이 설정되었습니다.");
        }

        private void showMessageBox(String msg)
        {
            MessageBox.Show(msg, "알림", MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

        private void noticeDisconnect()
        {
            showMessageBox("서버와의 연결이 끊어졌습니다.");
            Application.OpenForms["Form1"].Close();
        }

        private void appendText_Chat(String msg)
        {
            this.TEXT_CHAT.AppendText(msg + "\r\n\r\n");
        }

        private void refreshUser_List(List<String> list)
        {
            this.LIST_USER.Items.Clear();

            foreach (String name in list)
            {
                this.LIST_USER.Items.Add(name);
            }
        }

        private void Form1_FormClosed(object sender, FormClosedEventArgs e)
        {
            // 폼이 닫히면 Disconnect 하여 준다.
            if( client != null )
                client.DisConnect();
        }
    }
}
