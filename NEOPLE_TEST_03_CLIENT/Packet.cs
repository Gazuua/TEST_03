using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NEOPLE_TEST_03_CLIENTS
{
    class Packet
    {
        // 패킷 헤더 구조는 다음과 같다.
        // 최초 1바이트 - 데이터 영역의 총 byte 길이(최초 2바이트 제외)
        // 두번째 1바이트 - 코드값(이 값에 따라 요청/응답 구분)
        // 나머지 영역 - 데이터 영역

        // 주의점 : string 길이가 아닌 byte 길이로 데이터 길이를 환산해야 한다.
        // 또한 C++서버와 통신할 때 일어날 수 있는 인코딩 문제도 생각해야 한다.
        public enum Code
        {
            Standard = 0,
            NickName = 1,
            UserList = 2,
            Whisper = 3
        }

        private byte length;
        private byte code;
        private String data;

        // Code 0 - Standard 패킷 (채팅창에 메세지 전송)
        public Packet(String msg)
        {
            this.code = (byte) Code.Standard;
            this.data = msg;
            this.length = (byte)Encoding.Default.GetBytes(msg).Length;
        }

        // Code 지정에 따른 패킷 형성
        public Packet(String data, byte code)
        {
            this.code = code;
            this.data = data;
            this.length = (byte)Encoding.Default.GetBytes(data).Length;
        }

        // 클래스 형식의 패킷을 byte 배열로 직렬화하는 함수
        public static byte[] Encode(Packet packet)
        {
            byte[] data = new byte[packet.length + 2];
            data[0] = packet.length;
            data[1] = packet.code;

            byte[] strdata = Encoding.Default.GetBytes(packet.data);
            // data영역 copy
            for (int i=2; i<data.Length; i++)
            {
                data[i] = strdata[i - 2];
            }

            return data;
        }

        // byte배열 형식 패킷을 클래스로 역직렬화하는 함수
        public static Packet Decode(byte[] data)
        {
            byte[] strdata = new byte[data[0]];

            // data영역 copy
            for (int i = 0; i < strdata.Length; i++)
            {
                strdata[i] = data[i + 2];
            }
            String msg = Encoding.Default.GetString(strdata);

            return new Packet(msg, data[1]);
        }

        public byte getLength() { return this.length; }
        public byte getCode() { return this.code; }
        public String getData() { return this.data; }
    }
}
