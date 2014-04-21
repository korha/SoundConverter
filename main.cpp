#include "soundconverter.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SoundConverter w;
    w.show();
    return a.exec();
}
