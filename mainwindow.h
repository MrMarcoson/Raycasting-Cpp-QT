//Marek Kasprowicz 303020

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMainWindow>
#include <QImage>
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>

#define BLOCK_SIZE 32

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void paintEvent(QPaintEvent*);
    void keyPressEvent(QKeyEvent *event);

private:
    Ui::MainWindow *ui;
    QImage *img;
    int startX, startY, height, width;

    //Struktura ktora przechowuje informacje o umownym "graczu", czyli miejsca z ktorego patrzymy na swiat
    struct player
    {
        double x, y;                //wektor pozycyjny gracza
        double vectX, vectY;        //kierunek w którym patrzy
        double cameraX, cameraY;    //prostopadła linia do kierunku, która "zbiera" informacje o reyach (usuwa to efekt rybiego oka)
        double movement_speed;      //odleglosc z jaka po wcisnieciu klawisza przechodzimy po kaflach

        player(double x, double y, double vectX, double dirY, double cameraX, double cameraY, double movement_speed)
        {
            this->x = x;
            this->y = y;
            this->vectX = vectX;
            this->vectY = dirY;
            this->cameraX = cameraX;
            this->cameraY = cameraY;                //tutaj zdefinujemy szerokosc pola widzenia, 0.8 = ok 80 stopni
            this->movement_speed = movement_speed;  //0.1 to jakies 10 krokow na kafelek
        }
    };

    player *PLAYER;

    //Struktura dla koloru (do latwiejszego przekazywania)
    struct Color
    {
        int r, g, b;

        Color(int r, int g, int b)
        {
            this->r = r;
            this->g = g;
            this->b = b;
        }
    };

    //Struktury do par liczb koordynatowych
    struct vectorI
    {
        int x, y;

        vectorI() {}

        vectorI(int x, int y)
        {
            this->x = x;
            this->y = y;
        }
    };

    struct vectorD
    {
        double x, y;

        vectorD() {}

        vectorD(double x, double y)
        {
            this->x = x;
            this->y = y;
        }
    };

    //Tablica ktora na ktorej umownie: 0 - pusta przestrzen, >0 sciana. Rozrozniam 2 rodzaje scian, ktore roznia sie od siebie kolorem
    int map[10][10] = {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0, 2, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 2, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 2, 2, 0, 2, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 2, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    };

    //Metoda ktora rysuje klatke gry, czyli wylicza wszystkie wysokosci scian od lewej do prawej
    void gameLoop();

    //Metoda rysujaca sciany i sufit
    void drawAmbient();

    //Funkcje rysujace linie
    void drawPixel(int x, int y, Color *color);
    void drawLine(int x1, int y1, int x2, int y2, Color *color);
};
#endif // MAINWINDOW_H
