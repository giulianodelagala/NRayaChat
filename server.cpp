/* Server N-Raya */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <map>


#include "tresraya.h"

using namespace std;

std::vector<pair<char, int>> LISTA_CLIENTES;
//Un mapa inverso, otra opcion es un Bimap
std::map<int,char> R_LISTA;

// Variables del Juego
int TURNO = 0;
int size_juego = 5;
int NUM_JUGADORES = 2;
Raya TresRaya(size_juego);
///////////

string PadZeros(int number, int longitud)
{
  string num_letra = to_string(number);
  for (int i = 0; i < longitud - num_letra.length()+1; ++i)
    num_letra = "0" + num_letra;
  
  return num_letra;
}

void IniciarJuego(int size_tablero)
{
  TresRaya.ReiniciarTablero(size_tablero);
  TURNO = 0;

}

void EnviarMensaje(int socket_client, string mensaje)
{
  int n = write(socket_client, mensaje.c_str(), mensaje.length());
  if (n < 0) perror("ERROR writing to socket");
  
}

int IsTurno(int socket_client)
{
  if ( LISTA_CLIENTES[TURNO].second == socket_client)  
    return true; //Le corresponde jugar
  else
    return false; //No le corresponde  
}

void BroadCast(string mensaje)
{
  for (auto &i : LISTA_CLIENTES)
  {
      EnviarMensaje(i.second, mensaje);
  }
}

void SiguienteTurno()
{
  TURNO = TresRaya.num_jugada % NUM_JUGADORES;
  int socket_client = LISTA_CLIENTES[TURNO].second;
  string ficha(1, LISTA_CLIENTES[TURNO].first);
  EnviarMensaje(socket_client, "T" + ficha);
}

void VerificarEstado(int socket_client, char ficha,  int x, int y)
{
  switch (TresRaya.VerificarEstadoJuego(x, y, ficha))
  {
  case 0: //Juego en marcha
    SiguienteTurno(); 
    break;
  
  case 1: //Hay ganador
    EnviarMensaje(socket_client, "W"); //A ganador
    BroadCast("L"); //A resto de jugadores
    IniciarJuego(size_juego);
    break;

  case -1: //Hay empate
    BroadCast("="); //A todos jugadores
    IniciarJuego(size_juego);
    break;
  }
}

void Process_Client_Thread(int socket_client)
{
  string msgToChat;
  char buffer[256];
  int n;
  char comando;
  int longitud = 0;
  //int id_jugador;
  do
    {
      if (LISTA_CLIENTES.size() < 2)
        continue;

      //Lectura comando
      n = read(socket_client,buffer,1);     

      if (n < 0)
        perror("ERROR reading from socket");

      else if (n == 1)
      { 
        comando = buffer[0];

        switch (comando)
        {
        case 'A': 
        {
          if (IsTurno(socket_client))
          {  //Es turno de jugador 
            n = read(socket_client, buffer, 2);
            char x = buffer[0]; int num_x = (int)x - 48;
            char y = buffer[1]; int num_y = (int)y - 48;
            //En este caso comando = ficha
            if (TresRaya.InsertarJugada( comando, num_x, num_y) )
            {
              //Jugada Legal
              string t(1, comando);
              string jugada = "A" + t + to_string(num_x) + to_string(num_y);
              BroadCast(jugada);
              VerificarEstado(socket_client, comando, num_x , num_y );
            }
            else
            {
              //JugadaIlegal Mensaje de Error
              EnviarMensaje(socket_client, "C08Invalida");          
            } 
          }     
          else
          { //No es turno 
            n = read(socket_client, buffer, 2); //Purga
            EnviarMensaje(socket_client, "C14No es su turno");
          }
          break;
        }
        case 'C': //Es un mensaje de chat
          //Leer tamaño mensaje
          n = read(socket_client, buffer, 2);
          if (n == 2)
          {
            string long_mens = buffer;
            int longitud = stoi(buffer);
            //Leer mensaje
            n = read(socket_client, buffer, longitud);
            if (n == longitud)
            {
              msgToChat = buffer;
              msgToChat = msgToChat.substr(0, longitud);
              msgToChat = "C" + long_mens + msgToChat;
              BroadCast(msgToChat);
            }         
          }
          break;
        case 'S': //Es Creacion de Tablero
        {
          n = read(socket_client, buffer, 2);
          if (n == 2)
          {
            int size_tab = stoi(buffer);
            IniciarJuego(size_tab);
          }
          break;
        }
        case 'U': //Elegir ficha
        {
          n = read(socket_client, buffer, 1);
          if (n == 1)
          {
            LISTA_CLIENTES.push_back(pair<char,int>(buffer[0],(int)socket_client));
          }
          break;
        }
        case 'N': //Numero jugadores
        {
          n = read(socket_client, buffer, 1);
          if (n == 1)
          {
            string num = buffer;
            num = num.substr(0,1);
            NUM_JUGADORES = stoi(num);
          }
          break;

        }
        default:
          EnviarMensaje(socket_client, "C08Invalida"); //Mensaje de Error
          break;
        }
        
      }

    } while (true);

    shutdown(socket_client, SHUT_RDWR);
    close(socket_client); 
}




int main(void)
{
  struct sockaddr_in stSockAddr;
  int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  
  
  if(-1 == SocketFD)
  {
    perror("can not create socket");
    exit(EXIT_FAILURE);
  }

  memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

  stSockAddr.sin_family = AF_INET;
  stSockAddr.sin_port = htons(1100);
  stSockAddr.sin_addr.s_addr = INADDR_ANY;

  if(-1 == bind(SocketFD,(const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
  {
    perror("error bind failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }

  if(-1 == listen(SocketFD, 10))
  {
    perror("error listen failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }

  //int last_client = -1;
  for(;;)
  {
    int ClientSD = accept(SocketFD, NULL, NULL);

    if(0 > ClientSD)
    {
      perror("error accept failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
    
    std::thread(Process_Client_Thread, ClientSD).detach();

    /*
    if (TresRaya.num_jugada == 0)
    {
      IniciarJuego();
    }
    */

  } 
  
  close(SocketFD);
  return 0;
}
