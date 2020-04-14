using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Windows.Forms;

namespace NEOPLE_TEST_03_CLIENTS
{
    class SocketClient
    {
        public delegate void Callback();
        public delegate void Callback_string(String msg);
        public delegate void Callback_list(List<String> list);

        private Callback_string appendText_Chat = null;
        private Callback_string showMessageBox = null;
        private Callback_list refreshUser_List = null;
        private Callback_string nicknameDupCheck = null;
        private Callback noticeDisconnect = null;

        private List<String> list_user;

        private IPEndPoint endPoint;
        private Socket socket;

        private Thread readThread;
        private byte[] readBuffer;

        public SocketClient(String ip, int port)
        {
            endPoint = new IPEndPoint(IPAddress.Parse(ip), port);
            socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

            list_user = new List<String>();
        }

        public Boolean Connect()
        {
            try
            {
                if (!endPoint.Equals(null) && !socket.Equals(null))
                    socket.Connect(endPoint);
            }
            catch
            {
                return false;
            }

            return true;
        }

        public void startReadThread()
        {
            readBuffer = new byte[256];

            readThread = new Thread(readThreadProc);
            readThread.Start();
        }

        public void sendMessage(String msg)
        {
            // 쓰기 중에 예외가 발생할 경우 처리하기 위해 try~catch 블럭으로 감싼다
            try
            {
                Packet packet = new Packet(msg);
                byte[] sendbytes = Packet.Encode(packet);
                socket.Send(sendbytes);
            }
            catch (SocketException)
            {
                // 예외 발생 시 접속 종료를 알린다
                noticeDisconnect();
            }
        }

        public void sendMessage(String msg, Packet.Code code)
        {
            // 쓰기 중에 예외가 발생할 경우 처리하기 위해 try~catch 블럭으로 감싼다
            try
            {
                Packet packet = new Packet(msg, (byte)code);
                byte[] sendbytes = Packet.Encode(packet);
                socket.Send(sendbytes);
            }
            catch (SocketException)
            {
                // 예외 발생 시 접속 종료를 알린다
                noticeDisconnect();
            }
        }

        private void readThreadProc()
        {
            while (true)
            {
                int recvBytes = 0;
                byte length = 0;
                byte code = 0;
                byte[] data = null;

                // 읽기 버퍼를 쓰기 전 제로필(clear) 해준다.
                for (int i = 0; i < 256; i++)
                    readBuffer[i] = 0;

                // 읽기 중에 예외가 발생할 경우 처리하기 위해 try~catch 블럭으로 감싼다
                try
                {
                    // 1바이트(크기)를 읽는다. 읽고 나서 다시 읽으면 덮어씌워지므로 length에 옮겨둔다
                    recvBytes += socket.Receive(readBuffer, 1, SocketFlags.None);
                    length = readBuffer[0];

                    // 데이터 영역 크기 + 1바이트(헤더) 만큼 읽어올 때까지 계속 읽는다.
                    while ((recvBytes += socket.Receive(readBuffer, length + 1, SocketFlags.None)) != length + 2) ;
                }
                catch (SocketException)
                {
                    // 예외 발생 시 접속 종료를 알린다
                    noticeDisconnect();
                }

                // Code와 Data값을 지역변수에 카피한다.
                code = readBuffer[0];
                data = new byte[length];
                for(int i=0; i<length; i++)
                {
                    data[i] = readBuffer[i + 1];
                }

                String dataString = Encoding.Default.GetString(data);

                // 받은 데이터를 분석하여 적절한 동작을 한다.
                // 올바르지 않은 프로토콜일 경우
                if ( code > 3 )
                {
                    // 잘못된 서버로 접속했을 가능성이 있으므로 접속을 종료한다.
                    noticeDisconnect();
                }

                // 코드값을 구분하기 전 Packet.Code로 캐스팅한다.
                switch((Packet.Code)code)
                {
                    // 일반 패킷 - 채팅창에 메세지를 출력하여 준다.
                    case Packet.Code.Standard:
                        {
                            appendText_Chat(dataString);
                        }
                        break;
                    // 닉네임 패킷 - SUCCESS / FAIL 중 하나로 응답이 온다.
                    // 그 값을 구분하여 동작을 진행하면 된다.
                    case Packet.Code.NickName:
                        {
                            nicknameDupCheck(dataString);
                        }
                        break;
                    // 리스트 패킷 - 유저 리스트가 "유저1/유저2/유저3/...." 형식으로 온다.
                    // 구분자 '/'을 기준으로 
                    case Packet.Code.UserList:
                        {
                            // 1. 리스트를 초기화한다.
                            list_user.Clear();

                            // 2. 리스트 문자열을 split한다.
                            String[] liststr = dataString.Split('/');

                            // 3. split된 문자열을 list에 넣어준다.
                            foreach(String user in liststr)
                                list_user.Add(user);

                            // 4. ListView를 Refresh한다.
                            refreshUser_List(list_user);
                        }
                        break;
                    // 귓속말 패킷 - 유저가 한 명을 지정하여 그 사람에게만 보이도록 말한다.
                    // 서버에 갈 때 - "귓속말 대상자/메세지"
                    // 대상자에게 갈 때 - "귓속말 보낸 사람/메세지"
                    case Packet.Code.Whisper:
                        {
                            appendText_Chat(dataString);
                        }
                        break;
                }
            }
        }

        public void DisConnect()
        {
            this.readThread.Abort();
            socket.Close();
        }

        public void setDel_chatText(Callback_string del)
        {
            this.appendText_Chat = del;
        }

        public void setDel_showMsgBox(Callback_string del)
        {
            this.showMessageBox = del;
        }

        public void setDel_refreshUser_List(Callback_list del)
        {
            this.refreshUser_List = del;
        }

        public void setDel_nicknameDupCheck(Callback_string del)
        {
            this.nicknameDupCheck = del;
        }

        public void setDel_noticeDisconnect(Callback del)
        {
            this.noticeDisconnect = del;
        } 
    }
}
