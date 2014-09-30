#include <QDebug>
#include <kio/job.h>
#include <QApplication>
#include <kaboutdata.h>
#include <kcomponentdata.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QStringList args;
    for (int i=1; i<argc; ++i) {
        args << argv[i];
    }
    if (args.count() != 2) {
        qFatal("Required parameters: source destination");
    }
    qDebug() << QUrl::fromUserInput(args[0]) << QUrl::fromUserInput(args[1]);
    KIO::Job* job = KIO::file_copy(QUrl::fromUserInput(args[0]), QUrl::fromUserInput(args[1]), -1, KIO::Overwrite | KIO::HideProgressInfo);
    if (job->exec()) {
        qDebug() << "successfully copied";
        return 0;
    } else {
        qDebug() << "copy failed";
        return 1;
    }
    return 0;
}
