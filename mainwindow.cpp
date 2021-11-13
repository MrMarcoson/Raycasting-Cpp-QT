//Marek Kasprowicz 303020

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qdebug.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    startX = ui->frame->x();
    startY = ui->frame->y();

    width = ui->frame->width();
    height = ui->frame->height();

    img = new QImage(width, height, QImage::Format_RGB32);
    PLAYER = new player(8, 8, -1, 0, 0, 0.8, 0.1);

    gameLoop();
}

void MainWindow::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.drawImage(startX, startY, *img);
}

//implementacja krokow i obracania sie
void MainWindow::keyPressEvent(QKeyEvent *event)
{   
    //dodaje do pozycji gracza jego kierunek i mnoze przez szybkosc ruchu
    //przy predkosci 0.1 gracz moze zrobic 10 krokow wzdloz i wszerz na jednym klocku
    if(char(event->key()) == 'W') {
        PLAYER->x += PLAYER->vectX * PLAYER->movement_speed;
        PLAYER->y += PLAYER->vectY * PLAYER->movement_speed;
    }

    if(char(event->key()) == 'S') {
        PLAYER->x -= PLAYER->vectX * PLAYER->movement_speed;
        PLAYER->y -= PLAYER->vectY * PLAYER->movement_speed;
    }

    /*
        Rotacja odbywa sie poprzez przemnoznie wektorow gracza przez macierz rotacji (patrz przeksztalcenia2D)

        Rownanie wyglada tak:

        Macierz rotacji          x i y gracza
        | cos(r) -sin(r) |  *   | x |   =  ...
        | sin(a)  cos(r) |      | y |
    */

    if(char(event->key()) == 'A') {
        //obracam kierunek patrzenia gracza
        double vect = PLAYER->vectX;
        PLAYER->vectX = PLAYER->vectX * cos(PLAYER->movement_speed) - PLAYER->vectY * sin(PLAYER->movement_speed);
        PLAYER->vectY = vect * sin(PLAYER->movement_speed) + PLAYER->vectY * cos(PLAYER->movement_speed);

        //oraz jego kamere
        vect = PLAYER->cameraX;
        PLAYER->cameraX = PLAYER->cameraX * cos(PLAYER->movement_speed) - PLAYER->cameraY * sin(PLAYER->movement_speed);
        PLAYER->cameraY = vect * sin(PLAYER->movement_speed) + PLAYER->cameraY * cos(PLAYER->movement_speed);
    }

    if(char(event->key()) == 'D') {
        double vect = PLAYER->vectX;
        PLAYER->vectX = PLAYER->vectX * cos(-PLAYER->movement_speed) - PLAYER->vectY * sin(-PLAYER->movement_speed);
        PLAYER->vectY = vect * sin(-PLAYER->movement_speed) + PLAYER->vectY * cos(-PLAYER->movement_speed);

        vect = PLAYER->cameraX;
        PLAYER->cameraX = PLAYER->cameraX * cos(-PLAYER->movement_speed) - PLAYER->cameraY * sin(-PLAYER->movement_speed);
        PLAYER->cameraY = vect * sin(-PLAYER->movement_speed) + PLAYER->cameraY * cos(-PLAYER->movement_speed);
    }

    gameLoop();
}

//rysowanie scian, odbywa sie to tylko gdy gracz zdecyduje sie na krok
void MainWindow::gameLoop()
{
    drawAmbient();

    //sprawdzenie dla kazdego poziomego piksela jak daleko znajduje sie od gracza
    for(int i = 0; i < width; i++)
    {
        //Kafelek w którym się znajdujemy
        vectorI *block = new vectorI(PLAYER->x, PLAYER->y);

        //Wyznaczenie lokalizacji na "kamerze" i kierunku raya (lini od gracza szukajacej sciany)
        double pov = ((double)(2 * i) / width) - 1;
        vectorD *ray = new vectorD(PLAYER->cameraX * pov + PLAYER->vectX, PLAYER->cameraY * pov + PLAYER->vectY);

        //odleglosc miedzy liniami, przez ktore przechodzi ray
        //przy pomocy wz pitagorasa
        vectorD *delta = new vectorD(sqrt(1 + (ray->y * ray->y) / (ray->x * ray->x)), sqrt(1 + (ray->x * ray->x) / (ray->y * ray->y)));

        //Kierunek wektora w ktora leci ray, w wersji absolutnej
        vectorI *dir = new vectorI();

        //Odleglosci do pierwszych kafelkow od zderzenia z pierwsza linia
        vectorD *toNext = new vectorD();

        //prawo (dodaje jeden, aby uniknąć zera)
        if(ray->x >= 0) {
            toNext->x = (block->x + 1 - PLAYER->x) * delta->x;
            dir->x = 1;
        }

        //lewo
        if(ray->x < 0) {
            toNext->x = (PLAYER->x - block->x) * delta->x;
            dir->x = -1;
        }

        //gora
        if(ray->y >= 0) {
            toNext->y = (block->y + 1 - PLAYER->y) * delta->y;
            dir->y = 1;
        }

        //dol
        if(ray->y < 0) {
            toNext->y = (PLAYER->y - block->y) * delta->y;
            dir->y = -1;
        }

        //okreslenie ktory brzeg sciany zostala trafiona(vertical or horizontal)
        char edge;

        //iteruje do czasu znalezienia sciany na mapie
        //robie kroki do kolejnych lini x lub y, w zaleznosci od tego ktora odleglosc jest blizsza
        for(;;)
        {
            //kazda iteracja to krok do kolejnej lini, dodaje wiec odleglosc delty i krok do kolejnego bloku
            //latwo tez okreslic tutaj ktora czesc sciany uderzam (lewa/prawa lub gora/dol)
            if(toNext->x < toNext->y) {
                block->x += dir->x;
                toNext->x += delta->x;
                edge = 'v';
            }

            else {
                block->y += dir->y;
                toNext->y += delta->y;
                edge = 'h';
            }

            //jesli wyliczone bloki odpowiadaja scianie na mapie to przerwij petle
            if(map[block->x][block->y] > 0) {
                break;
            }
        }

        //wyliczam odleglosc od gracza do sciany, w celu narysowania odpowiedniej wysokosci
        double toWall;

        //player znajduje sie w konkretnym kafelku, ale ma tam tez mniejsze "kroczki", wiec odejmuje od blocku ostatniego pozycje playera.
        //pierwszy nawias otrzymuje liczbe kafelek ktore przeszedl ray
        //aby odleglosc byla "prostopadla" podzielimy przez wektor raya, w ten sposob "odbijemy" na ekranie linie i unikniemy rybiego oka
        if(edge == 'v') {
            toWall = (block->x - PLAYER->x + (1 - dir->x) / 2) / ray->x;
        }

        if(edge == 'h') {
            toWall = (block->y - PLAYER->y + (1 - dir->y) / 2) / ray->y;
        }

        //obliczenie stalej do wysokosci sciany (maksymlny ekran na dystans do niej, sprawia, ze zludzenie przypomina realny efekt)
        //skaluje dystans przez wysokosc ekranu
        int altitude = width / toWall;

        //obliczam poczatek i koniec rysowania sciany (przesuwam tak, aby sciany byly na srodku ekranu)
        int start = (-altitude / 2) + width / 2;
        int end = (altitude / 2) + width / 2;

        //zabezpieczenie przed wyjsciem z ekranu
        if(start < 0) {
            start = 0;
        }

        if(end >= width) {
            end = width - 1;
        }

        //narysowanie odpowiedniej lini w zaleznosci od kafelka i sciany bloka
        if(map[block->x][block->y] == 1 && edge == 'v') drawLine(i, start, i, end, new Color(0, 0, 255));
        if(map[block->x][block->y] == 1 && edge == 'h') drawLine(i, start, i, end, new Color(0, 0, 200));
        if(map[block->x][block->y] == 2 && edge == 'v') drawLine(i, start, i, end, new Color(255, 0, 0));
        if(map[block->x][block->y] == 2 && edge == 'h') drawLine(i, start, i, end, new Color(200, 0, 0));

        delete(block);
        delete(ray);
        delete(delta);
        delete(dir);
        delete(toNext);
    }
}

//narysowanie "zludzenia" sufitu i podlogi
void MainWindow::drawAmbient()
{
    for(int i = 0; i < width; i++)
    {
        if(i < width / 2) drawLine(0, i, width, i, new Color(47, 79, 79));      //kolory z Wolfestein3D
        else drawLine(0, i, width, i, new Color(112, 128, 144));
    }
}

void MainWindow::drawPixel(int x, int y, MainWindow::Color *color)
{
    if(x > width || x < 0 || y > height || y < 0) return;
    unsigned char *wsk;
    wsk = img->scanLine(y);    
    wsk[4*x] = color->b;
    wsk[4*x+1] = color->g;
    wsk[4*x+2] = color->r;
    update();
}

void MainWindow::drawLine(int x1, int y1, int x2, int y2, MainWindow::Color *color)
{
    int x, y;
    bool ver = false;
    double m = (double)(y2 - y1) / (x2 - x1);

    //gdy nie ruszymy myszką, to nie rysuj
    if(x1 == x2 && y1 == y2)
    {
        return;
    }

    //jesli wartosc kierunkowa jest > 1 to trzeba zamienic zmienne (w celu eliminacji "kropkowania")
    if(abs(m) > 1)
    {
        int pom;
        pom = x1;
        x1 = y1;
        y1 = pom;

        pom = x2;
        x2 = y2;
        y2 = pom;

        m = (double)(y2-y1)/(x2-x1);
        ver = true;
    }

    //w celu mozliwosci rysowania w obie strony, zamieniamy miejscami punkty gdy drugie klikniecie jest bardziej w lewo od pierwszego
    if(x1 > x2)
    {
        int pom;
        pom = x1;
        x1 = x2;
        x2 = pom;

        pom = y1;
        y1 = y2;
        y2 = pom;
    }

    //petla rysujaca
    for(x = x1; x <= x2; x++)
    {
        y = m * (x - x1) + y1;

        //jesli wspolczynnik m byl wiekszy od 1 to nalezy zamienic wspolczynniki miejscami (obrocic uklad wspolrzednych)
        if(ver == 0) drawPixel(x, round(y), color);
        else drawPixel(y, x, color);
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}
