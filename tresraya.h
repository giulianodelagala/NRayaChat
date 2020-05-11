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
    void ReiniciarTablero(int size);
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
        cout << endl ;
    }
    cout << "Presione Enter para continuar..." << endl;
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

bool Raya::IsWin(int x,int y,char ficha){
    int d=0,a=0;
    for(int i = 0; i < size; i++){
        if(tablero[y][i].prop ==ficha)d++;
        if(tablero[i][x].prop ==ficha)a++;
    }
    if (d==size || a==size ){return true;}
    d=0;a=0;
    if (x==y || (x+y)==size-1){
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
        if (num_jugada >= max_jugadas -1)
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

void Raya::ReiniciarTablero(int size)
{
    this->size = size;
    tablero.resize(size, vector<Casilla>(size));
    this->max_jugadas = size * size;

    for (auto i = tablero.begin(); i != tablero.end(); ++i)
    {
        for( auto j = i->begin(); j != i->end(); ++j)           
            j->prop = '*';          
    }
    num_jugada = 0;
}
