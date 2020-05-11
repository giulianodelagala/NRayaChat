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

#include "tresraya.h"

using namespace std;

int SIZE_JUEGO = 3;
Raya TresRaya(SIZE_JUEGO);
string usuario = "";

string PadZeros(int number, int longitud)
{
  string num_letra = to_string(number);
  for (int i = 0; i < longitud - num_letra.length()+1; ++i)
    num_letra = "0" + num_letra;
  
  return num_letra;
}

string ProtocoloMensaje(string mensaje)
{
  int longitud = mensaje.length();
  return "C" + PadZeros(longitud, 2) + mensaje.substr(1);
}

void RecepcionMensaje(int SocketFD)
{
  int n;
  char buffer[256];
  int longitud = 0;
  for(;;)
  {
    //Lectura Comando
    n = read(SocketFD,buffer,1);
    if (n < 0) perror("ERROR reading from socket");
    if (n == 1)
    {
      switch (buffer[0]) //LetraComando
      {
        case 'L':
          cout << "Perdiste";
          TresRaya.ReiniciarTablero(SIZE_JUEGO);
          break;
        case 'W':
          cout << "Ganaste";
          TresRaya.ReiniciarTablero(SIZE_JUEGO);
          break;
        case '=':
          cout << "Empate";
          TresRaya.ReiniciarTablero(SIZE_JUEGO);
          break;
        case 'T':
        {
          n = read(SocketFD,buffer,1);
          if (n < 0) perror("ERROR reading from socket");
          cout << "Es tu turno " << buffer[0];
          break;
        }
        case 'A': 
        { 
          n = read(SocketFD,buffer,3); //[0] es la ficha a insertar
          //[1] [2] Estas son x y y por ahora de una cifra!!!
          if (n < 0) perror("ERROR reading from socket");
          if (n == 3)
          {
            TresRaya.InsertarJugada(buffer[0], (int)(buffer[1]-48), (int)(buffer[2]-48));
            TresRaya.ImprimirTablero();
            bzero(buffer,256);
          }
          break;
        }
        case 'C': //Mensaje
        {
          n = read(SocketFD, buffer, 2);
          if (n == 2)
          {
            longitud = stoi(buffer);
            //Leer mensaje
            n = read(SocketFD, buffer, longitud);
            if (n == longitud)
            {
              cout << buffer << "\n";
            } 
          }
          break;
        }
        case 'S': // Creacion Tablero
        {
          n = read(SocketFD, buffer, 2);
          if (n == 2)
          {
            SIZE_JUEGO = stoi(buffer);
            TresRaya.ReiniciarTablero(SIZE_JUEGO);
          }
          break;
        }

      }
      
    }
  }
}

void EnvioMensaje(int SocketFD)
{
  string msgToChat = "";
  //char buffer[256];
  int n;
  TresRaya.ImprimirTablero();
  for(;;)
  { 
    //cin.clear(); 
    cout << "\nIngrese jugada: ";
    getline(cin, msgToChat);
    if(msgToChat != "" && msgToChat[0] == 'C')
    { //Es mensaje a Chat
      //Preparar mensaje
      msgToChat = ProtocoloMensaje(msgToChat);
      n = write(SocketFD, msgToChat.c_str(), msgToChat.length());
    }
    else if (msgToChat != "")
    {
      //Posible Comando o jugada
      n = write(SocketFD, msgToChat.c_str(), msgToChat.length());
    }
    else
      continue;
       
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
  stSockAddr.sin_port = htons(10017);
  Res = inet_pton(AF_INET, "51.15.220.108", &stSockAddr.sin_addr);

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
  // Creacion de usuario///
  cout <<"Ingrese su ficha\n";
  getline(cin, usuario);
  msgFromChat = "U" + usuario[0];
  n = write(SocketFD, msgFromChat.c_str(), msgFromChat.length());

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
