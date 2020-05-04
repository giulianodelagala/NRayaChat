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
int size_juego = 3;
int num_jugadores = 2;
Raya TresRaya(size_juego);
///////////

string PadZeros(int number, int longitud)
{
  string num_letra = to_string(number);
  for (int i = 0; i < longitud - num_letra.length()+1; ++i)
    num_letra = "0" + num_letra;
  
  return num_letra;
}

void IniciarJuego()
{
  TresRaya.ReiniciarTablero();
  TURNO = 0;

}

void EnviarMensaje(int socket_client, string mensaje)
{
  //mensaje = PadZeros(mensaje.length() + 1, 3) + mensaje + "\0";

  int n = write(socket_client, mensaje.c_str(), mensaje.length());
  if (n < 0) perror("ERROR writing to socket");
  
}

int VerificarJugador(char comando, int socket_client)
{
  if (comando == 'C') //Es un mensaje de Chat
    return 1;
  else
  {
    if ( LISTA_CLIENTES[TURNO].second == socket_client)  
      return 0; //Le corresponde jugar
    else
      return -1; //No le corresponde  
  }
}

void BroadCast(string mensaje, int excepcion = -1)
{
  for (auto &i : LISTA_CLIENTES)
  {
    if (i.second != excepcion)
    {
      EnviarMensaje(i.second, mensaje);
    }
  }
}

void SiguienteTurno()
{
  TURNO = TresRaya.num_jugada % num_jugadores;
  int socket_client = LISTA_CLIENTES[TURNO].second;
  EnviarMensaje(socket_client, "T"+ LISTA_CLIENTES[TURNO].first);
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
    BroadCast("L", socket_client); //A resto de jugadores
    //TresRaya.ReiniciarTablero();
    IniciarJuego();
    break;

  case 2: //Hay empate
    BroadCast("="); //A todos jugadores
    //TresRaya.ReiniciarTablero();
    IniciarJuego();
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

        switch (VerificarJugador(comando, socket_client))
        {
        case 0: //Es turno de jugador
        {
          n = read(socket_client, buffer, 2);
          char x = buffer[0]; int num_x = (int)x - 48;
          char y = buffer[1]; int num_y = (int)y - 48;
          //En este caso comando = ficha
          if (TresRaya.InsertarJugada( comando, num_x, num_y) )
          {
            //Jugada Legal
            string t(1, comando);
            string jugada = "A" + t + to_string(num_x) + to_string(num_y);
            BroadCast(jugada, socket_client);
            VerificarEstado(socket_client, comando, num_x , num_y );
          }
          else
          {
            //JugadaIlegal Mensaje de Error
            EnviarMensaje(socket_client, "C08Invalida");          
          }       
          break;
        }
        case 1: //Es un mensaje de chat
          BroadCast("Mensaje", socket_client); //TODO
          break;

        case -1:
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

  //LISTA_CLIENTES.insert(std::pair<string,int>("lista",9999));

  int last_client = -1;
  for(;;)
  {
    int ClientSD = accept(SocketFD, NULL, NULL);

    if(0 > ClientSD)
    {
      perror("error accept failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
    else
    {
      if (ClientSD != last_client && LISTA_CLIENTES.size() == 0)
      {
        LISTA_CLIENTES.push_back(std::pair<char,int>('O',(int)ClientSD));
        //EnviarMensaje(ClientSD, "Tu Fichas es O");
        last_client = ClientSD;
      }
      else if (ClientSD != last_client && LISTA_CLIENTES.size() == 1)
      {
        LISTA_CLIENTES.push_back(std::pair<char,int>('X',(int)ClientSD));
        //EnviarMensaje(ClientSD, "Tu Fichas es X");
        last_client = ClientSD;
      }     
    }
    
    

    //Verificar al menos dos jugadores

    std::thread(Process_Client_Thread, ClientSD).detach();

    if (TresRaya.num_jugada == 0)
    {
      IniciarJuego();
    }

  } 
  
  close(SocketFD);
  return 0;
}
