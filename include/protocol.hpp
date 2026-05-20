#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP
#include <string>
#include <iomanip> 
#include <vector>
#include <sys/socket.h> 
#include <sstream>
using namespace std;
namespace chat_protocol
{
    inline string encode_header(int body_length)
    {
        ostringstream oss;
        oss<<setw(4)<<setfill('0')<<body_length;
        return oss.str();
    }


inline bool read_exact(int socket_fd,int target_len,char *output_buf)

{
        int total_bytes=0;
        while(total_bytes<target_len)
        {
            int curr_bytes=recv(socket_fd,output_buf+total_bytes,target_len-total_bytes,0);
            if(curr_bytes<=0)
            return false;
            total_bytes+=curr_bytes;
        }
        return true;
}

inline bool receive_message(int socket_fd,string &out_message)
{
    char header_buff[4];
    if(!read_exact(socket_fd,4,header_buff))
    return false;
    string header_str(header_buff, 4);
    int body_length = 0;
    stringstream ss(header_str);
    if (!(ss >> body_length) || body_length <= 0) {
            return false; 
        }
    vector<char>body_buf(body_length);
    if(!read_exact(socket_fd,body_length,body_buf.data()))
    {
        return false;
    }
    out_message.assign(body_buf.begin(), body_buf.end());
    return true;
}

inline bool send_message(int socket_fd, const string &message)
{
    string header=encode_header(message.length());
    string full_packet=header+message;
    int total_bytes_sent=0;
    int total_bytes=full_packet.length();
    const char *ptr=full_packet.c_str();
    while(total_bytes_sent<total_bytes)
    {
        int data_send=send(socket_fd,ptr+total_bytes_sent,total_bytes-total_bytes_sent,0);
        if(data_send<=0)
        return false;
        total_bytes_sent+=data_send;

    }
    return true;

}}
#endif