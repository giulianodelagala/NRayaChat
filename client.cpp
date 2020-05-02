/* Client N-Raya*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <thread>

using namespace std;

void RecepcionMensaje(int SocketFD)
{
  int n;
  char buffer[256];
  int longitud;
  for(;;)
  {
    //Lectura length de mensaje
    n = read(SocketFD,buffer,3);
    if (n < 0) perror("ERROR reading from socket");
    if (n == 3)
      longitud = stoi(buffer);
      n = read(SocketFD,buffer,longitud);
      cout << buffer << "\n";
      bzero(buffer,256);
  }
}

string PadZeros(int number, int longitud)
{
  string num_letra = to_string(number);
  for (int i = 0; i < longitud - num_letra.length()+1; ++i)
    num_letra = "0" + num_letra;
  
  return num_letra;
}

string ProtocoloMensaje(string mensaje)
{
  //Encontrar primer espacio en blanco
  size_t blank = mensaje.find(" ");
  string nick = "";
  if (blank != string::npos)
  {
    //Encontro blank supuesto nickname
    nick = mensaje.substr(0,blank);
    nick = PadZeros(blank +1, 2) + nick;
  }
  else
  {
    //Es supuesto comando (incluir fin de cadena)
    return PadZeros(mensaje.length() + 1,2) + mensaje + '\0';  
  }
  //Generar nick + mensaje
  string msgtoChat = mensaje.substr(blank+1);
  msgtoChat = PadZeros(msgtoChat.length() +1, 3) + msgtoChat;

  return nick+ "+" + msgtoChat + '\0';  

}

void EnvioMensaje(int SocketFD)
{
  string msgToChat;
  char buffer[256];
  int n;

  for(;;)
  { 
    cin.clear(); 
    cout << "\nIngrese jugada: ";
    getline(cin, msgToChat);
    msgToChat = ProtocoloMensaje(msgToChat);
    //cout << msgToChat;
    n = write(SocketFD, msgToChat.c_str(), msgToChat.length());
    
    bzero(buffer, 256);    
  }
}

int main(void)
{
  struct sockaddr_in stSockAddr;
  int Res;
  int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  char buffer[256];
  string msgFromChat;
  int n;

  if (-1 == SocketFD)
  {
    perror("cannot create socket");
    exit(EXIT_FAILURE);
  }

  memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

  stSockAddr.sin_family = AF_INET;
  stSockAddr.sin_port = htons(1100);
  Res = inet_pton(AF_INET, "127.0.0.1", &stSockAddr.sin_addr);

  if (0 > Res)
  {
    perror("error: first parameter is not a valid address family");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }
  else if (0 == Res)
  {
    perror("char string (second parameter does not contain valid ipaddress");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }

  if (-1 == connect(SocketFD, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
  {
    perror("connect failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }
  bzero(buffer, 256);

  //Thread de Envio y Recepcion de Mensajes
  std::thread(EnvioMensaje, SocketFD).detach();
  std::thread(RecepcionMensaje, SocketFD).detach();
  //Mantener con vida los threads
  for(;;)
  {}

  shutdown(SocketFD, SHUT_RDWR);
  close(SocketFD);
  return 0;
}