#include "labelingtool.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	ScanningAndFusion w;
	w.show();
	return a.exec();
}
