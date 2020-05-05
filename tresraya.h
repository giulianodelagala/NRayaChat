#include <iostream>
#include <vector>
#include <string>

using namespace std;

struct Casilla
{
    //bool isPlayed = false;
    char prop = '*';
};

class Raya
{
private:
    vector<vector<Casilla>> tablero;
public:
    int size;
    int num_jugada = 0; //En que jugada vamos?
    int max_jugadas; 
    void ImprimirTablero();
    bool InsertarJugada(char ficha, int x, int y);
    bool IsWin(int y, int x, char ficha);
    int VerificarEstadoJuego(int x, int y, char ficha);
    void ReiniciarTablero();
    Raya(int size)
    {
        this->size = size;
        tablero.resize(size, vector<Casilla>(size));
        this->max_jugadas = size * size;
    }
    ~Raya(){};
};

void Raya::ImprimirTablero()
{
    int x = 0;
    string col = "\n  ";
    for (int y = 0; y < size; ++y)
        col+= to_string(y);
    cout << col << "\n";
    for (auto i = tablero.begin(); i != tablero.end(); ++i)
    {
        cout << x << " ";
        x++;
        for( auto j = i->begin(); j != i->end(); ++j)           
            cout << j->prop ;          
        cout << "\n";
    }        
}

bool Raya::InsertarJugada(char ficha, int x, int y)
{
    if (tablero[x][y].prop == '*')
    {
        tablero[x][y].prop = ficha;
        return true;
    }
    else
    {
        return false;
    }
}

bool Raya::IsWin(int y,int x,char ficha){
    int d=0,a=0;
    for(int i = 0; i < size; i++){
        if(tablero[y][i].prop ==ficha)d++;
        if(tablero[i][x].prop ==ficha)a++;
    }
    if (d==size || a==size ){return true;}
    d=0;a=0;
    if (x==y || (x+y)==size){
        for(int i = 0; i < size; i++){
            if(tablero[i][i].prop==ficha)d++;
            if(tablero[i][size-1-i].prop==ficha)a++;
        }
        if (d==size || a==size ){return true;}
    }
    return false;
}


int Raya::VerificarEstadoJuego(int x, int y, char ficha)
{
    if (IsWin(y, x, ficha))
    {
        return 1; //Hay ganador
    }
    else
    {
        if (num_jugada >= max_jugadas)
        {
            //Hay empate
            return -1;
        }
        else
        {   //Juego en marcha
            num_jugada++;
            return 0;
        }       
    }
}

void Raya::ReiniciarTablero()
{
    tablero.assign(size, vector<Casilla>(size));
    num_jugada = 0;
}

/*
int main()
{
    int size = 3;
    int num_jugadores = 2;
    int turno = 0;
    string mensaje = "";
    Raya TresRaya(size);
    int jugadas = 0;

    //max numero de jugadas = size * size
    for (int i = 0; i < size * size; ++i)
    {
        turno = i % num_jugadores;
        cout << "Turno jugador " << turno << "\n";
        cout << "Ingrese jugada";
        getline( cin, mensaje);
        TresRaya.InsertarJugada(mensaje[0], (int)(mensaje[1])-48, (int)mensaje[2]-48);
        TresRaya.ImprimirTablero();
    }
    return 0;
}
*/

